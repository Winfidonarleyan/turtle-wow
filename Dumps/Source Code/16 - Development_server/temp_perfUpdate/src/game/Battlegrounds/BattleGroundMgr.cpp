/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 * Copyright (C) 2011-2016 Nostalrius <https://nostalrius.org>
 * Copyright (C) 2016-2017 Elysium Project <https://github.com/elysium-project>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "SharedDefines.h"
#include "Player.h"
#include "BattleGroundMgr.h"
#include "BattleGroundAV.h"
#include "BattleGroundAB.h"
#include "BattleGroundWS.h"
#include "BattleGroundBR.h"
#include "BattleGroundSV.h"
#include "MapManager.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "World.h"
#include "WorldPacket.h"
#include "Language.h"
#include "GameEventMgr.h"
#include "Policies/SingletonImp.h"
#include <chrono>
#include <random>

BattleGroundMgr sBattleGroundMgr;

/*********************************************************/
/***            BATTLEGROUND QUEUE SYSTEM              ***/
/*********************************************************/

BattleGroundQueue::BattleGroundQueue()
{
    for (uint32 i = 0; i < BG_TEAMS_COUNT; ++i)
    {
        for (uint32 j = 0; j < MAX_BATTLEGROUND_BRACKETS; ++j)
        {
            m_SumOfWaitTimes[i][j] = 0;
            m_WaitTimeLastPlayer[i][j] = 0;
            for (uint32 k = 0; k < COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME; ++k)
                m_WaitTimes[i][j][k] = 0;
        }
    }
}

BattleGroundQueue::~BattleGroundQueue()
{
    m_QueuedPlayers.clear();
    for (auto& group : m_QueuedGroups)
    {
        for (uint32 j = 0; j < BG_QUEUE_GROUP_TYPES_COUNT; ++j)
        {
            for (GroupsQueueType::iterator itr = group[j].begin(); itr != group[j].end(); ++itr)
                delete(*itr);

            group[j].clear();
        }
    }
}

/*********************************************************/
/***      BATTLEGROUND QUEUE SELECTION POOLS           ***/
/*********************************************************/

// selection pool initialization, used to clean up from prev selection
void BattleGroundQueue::SelectionPool::Init()
{
    SelectedGroups.clear();
    PlayerCount = 0;
}

// remove group info from selection pool
// returns true when we need to try to add new group to selection pool
// returns false when selection pool is ok or when we kicked smaller group than we need to kick
// sometimes it can be called on empty selection pool
bool BattleGroundQueue::SelectionPool::KickGroup(uint32 size)
{
    //find maxgroup or LAST group with size == size and kick it
    bool found = false;
    GroupsQueueType::iterator groupToKick = SelectedGroups.begin();
    for (GroupsQueueType::iterator itr = groupToKick; itr != SelectedGroups.end(); ++itr)
    {
        if (abs((int32)((*itr)->Players.size() - size)) <= 1)
        {
            groupToKick = itr;
            found = true;
        }
        else if (!found && (*itr)->Players.size() >= (*groupToKick)->Players.size())
            groupToKick = itr;
    }
    //if pool is empty, do nothing
    if (GetPlayerCount())
    {
        //update player count
        GroupQueueInfo* ginfo = (*groupToKick);
        SelectedGroups.erase(groupToKick);
        PlayerCount -= ginfo->Players.size();
        //return false if we kicked smaller group or there are enough players in selection pool
        if (ginfo->Players.size() <= size + 1)
            return false;
    }
    return true;
}

// add group to selection pool
// used when building selection pools
// returns true if we can invite more players, or when we added group to selection pool
// returns false when selection pool is full
bool BattleGroundQueue::SelectionPool::AddGroup(GroupQueueInfo *ginfo, uint32 desiredCount, uint32 bgInstanceId)
{
    bool skip = false;
    for (const auto& memberPair : ginfo->Players)
    {
        auto player = sObjectAccessor.FindPlayer(memberPair.first);
        if (!player)
            continue;

        // dont allow adding to BG selectionpool group to join new BGs while in old BG.
        if (player->InBattleGround() || player->IsTaxiFlying() ||
            !player->GetSession()->IsConnected() ||
            player->GetSession()->isLogingOut())
        {
            skip = true;
            break;
        }
    }

    //if group is larger than desired count - don't allow to add it to pool
    if (!ginfo->IsInvitedToBGInstanceGUID &&
       (!ginfo->DesiredInstanceId || ginfo->DesiredInstanceId == bgInstanceId) &&
       (desiredCount >= PlayerCount + ginfo->Players.size()) && !skip)
    {
        SelectedGroups.push_back(ginfo);
        // increase selected players count
        PlayerCount += ginfo->Players.size();
        return true;
    }

    return PlayerCount < desiredCount;
}

/*********************************************************/
/***               BATTLEGROUND QUEUES                 ***/
/*********************************************************/

// add group or player (grp == nullptr) to bg queue with the given leader and bg specifications
GroupQueueInfo * BattleGroundQueue::AddGroup(Player *leader, Group* grp, BattleGroundTypeId BgTypeId, BattleGroundBracketId bracketId, bool isPremade, uint32 instanceId, std::vector<uint32>* excludedMembers)
{
    // create new ginfo
    GroupQueueInfo* ginfo = new GroupQueueInfo;
    ginfo->BgTypeId                  = BgTypeId;
    ginfo->IsInvitedToBGInstanceGUID = 0;
    ginfo->JoinTime                  = WorldTimer::getMSTime();
    ginfo->RemoveInviteTime          = 0;
    ginfo->GroupTeam                 = leader->GetTeam();
    ginfo->BracketId                 = bracketId;
    ginfo->DesiredInstanceId         = instanceId;
    ginfo->Players.clear();

    //compute index (if group is premade or joined a rated match) to queues
    uint32 index = 0;
    if (!isPremade)
        index += BG_TEAMS_COUNT;                            // BG_QUEUE_PREMADE_* -> BG_QUEUE_NORMAL_*

    if (ginfo->GroupTeam == HORDE)
        index++;                                            // BG_QUEUE_*_ALLIANCE -> BG_QUEUE_*_HORDE

    DEBUG_LOG("Adding Group to BattleGroundQueue bgTypeId : %u, bracket_id : %u, index : %u", BgTypeId, bracketId, index);

    //add players from group to ginfo
    {
        //ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_Lock);
        if (grp)
        {
            const uint32 group_limit = sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_GROUP_LIMIT);
            for (GroupReference *itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player *member = itr->getSource();
                if (!member)
                    continue;   // this should never happen
                
                if (excludedMembers && (std::find(excludedMembers->begin(), excludedMembers->end(), member->GetGUIDLow()) != excludedMembers->end()))
                    continue;

                if (grp->GetMembersCount() > group_limit) // queue players solo if group size is above limit set in config
                {
                    AddGroup(member, nullptr, BgTypeId, bracketId, false, instanceId, nullptr);
                    ChatHandler(member).PSendSysMessage("Group queue limit is set to %u. You have been queued solo.", group_limit);
                }
                else
                {
                    PlayerQueueInfo& pl_info = m_QueuedPlayers[member->GetObjectGuid()];
                    pl_info.online = true;
                    pl_info.LastOnlineTime = 0;
                    pl_info.GroupInfo = ginfo;
                    // add the pinfo to ginfo's list
                    ginfo->Players[member->GetObjectGuid()] = &pl_info;
                    sLog.out(LOG_BG, "%s:%u [%u:%s] tag BG=%u (groupLeader '%s')",

                        member->GetName(),
                        member->GetGUIDLow(), member->GetSession()->GetAccountId(), member->GetSession()->GetRemoteAddress().c_str(),
                        (uint32)BgTypeId, leader->GetName());
                }
            }
        }
        else
        {
            PlayerQueueInfo& pl_info = m_QueuedPlayers[leader->GetObjectGuid()];
            pl_info.online           = true;
            pl_info.LastOnlineTime   = 0;
            pl_info.GroupInfo        = ginfo;
            ginfo->Players[leader->GetObjectGuid()]  = &pl_info;
            sLog.out(LOG_BG, "%s:%u [%u:%s] tag BG=%u",
                     leader->GetName(),
                     leader->GetGUIDLow(), leader->GetSession()->GetAccountId(), leader->GetSession()->GetRemoteAddress().c_str(),
                (uint32)BgTypeId);
        }

        //add GroupInfo to m_QueuedGroups
        if (!ginfo->Players.empty())
            m_QueuedGroups[bracketId][index].push_back(ginfo);
        else
            return ginfo; // group size was above limit

        //announce to world, this code needs mutex
        if (!isPremade && sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_QUEUE_ANNOUNCER_JOIN))
        {
            BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(ginfo->BgTypeId);
            if (bg)
            {
                char const* bgName = bg->GetName();
                uint32 MinPlayers = bg->GetMinPlayersPerTeam();
                uint32 qHorde = 0;
                uint32 qAlliance = 0;
                uint32 q_min_level = leader->GetMinLevelForBattleGroundBracketId(bracketId, BgTypeId);
                uint32 q_max_level = leader->GetMaxLevelForBattleGroundBracketId(bracketId, BgTypeId);
                GroupsQueueType::const_iterator itr;
                for (itr = m_QueuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE].begin(); itr != m_QueuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE].end(); ++itr)
                    if (!(*itr)->IsInvitedToBGInstanceGUID)
                        qAlliance += (*itr)->Players.size();
                for (itr = m_QueuedGroups[bracketId][BG_QUEUE_NORMAL_HORDE].begin(); itr != m_QueuedGroups[bracketId][BG_QUEUE_NORMAL_HORDE].end(); ++itr)
                    if (!(*itr)->IsInvitedToBGInstanceGUID)
                        qHorde += (*itr)->Players.size();

                // Show queue status to player only (when joining queue)
                if (sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_QUEUE_ANNOUNCER_JOIN) == 1)
                {
                    ChatHandler(leader).PSendSysMessage(LANG_BG_QUEUE_ANNOUNCE_SELF, bgName, q_min_level, q_max_level - 1,
                                                        qAlliance, (MinPlayers > qAlliance) ? MinPlayers - qAlliance : (uint32)0, qHorde, (MinPlayers > qHorde) ? MinPlayers - qHorde : (uint32)0);
                }
                // System message
                else
                {
                    sWorld.SendWorldText(LANG_BG_QUEUE_ANNOUNCE_WORLD, bgName, q_min_level, q_max_level - 1,
                                         qAlliance, (MinPlayers > qAlliance) ? MinPlayers - qAlliance : (uint32)0, qHorde, (MinPlayers > qHorde) ? MinPlayers - qHorde : (uint32)0);
                }
            }
        }
        //release mutex
    }

    return ginfo;
}

void BattleGroundQueue::PlayerInvitedToBGUpdateAverageWaitTime(GroupQueueInfo* ginfo, BattleGroundBracketId bracket_id)
{
    uint32 timeInQueue = WorldTimer::getMSTimeDiff(ginfo->JoinTime, WorldTimer::getMSTime());
    uint8 team_index = BG_TEAM_ALLIANCE;                    //default set to BG_TEAM_ALLIANCE - or non rated arenas!

    if (ginfo->GroupTeam == HORDE)
        team_index = BG_TEAM_HORDE;

    //store pointer to arrayindex of player that was added first
    uint32* lastPlayerAddedPointer = &(m_WaitTimeLastPlayer[team_index][bracket_id]);
    //remove his time from sum
    m_SumOfWaitTimes[team_index][bracket_id] -= m_WaitTimes[team_index][bracket_id][(*lastPlayerAddedPointer)];
    //set average time to new
    m_WaitTimes[team_index][bracket_id][(*lastPlayerAddedPointer)] = timeInQueue;
    //add new time to sum
    m_SumOfWaitTimes[team_index][bracket_id] += timeInQueue;
    //set index of last player added to next one
    (*lastPlayerAddedPointer)++;
    (*lastPlayerAddedPointer) %= COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME;
}

uint32 BattleGroundQueue::GetAverageQueueWaitTime(GroupQueueInfo* ginfo, BattleGroundBracketId bracket_id)
{
    uint8 team_index = BG_TEAM_ALLIANCE;                    //default set to BG_TEAM_ALLIANCE - or non rated arenas!
    if (ginfo->GroupTeam == HORDE)
        team_index = BG_TEAM_HORDE;
    //check if there is enought values(we always add values > 0)
    if (m_WaitTimes[team_index][bracket_id][COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME - 1])
        return (m_SumOfWaitTimes[team_index][bracket_id] / COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME);
    else
        //if there aren't enough values return 0 - not available
        return 0;
}

// Ivina <Nostalrius> : log player inscription to bg queue.
void BattleGroundQueue::LogQueueInscription(Player *plr, BattleGroundTypeId BgTypeId, uint32 uiAction)
{
    std::string sPName = plr->GetName();
    std::string last_ip;
    time_t queuing_time = time(nullptr);
    if (WorldSession* session = plr->GetSession())
        last_ip = session->GetRemoteAddress();

    CharacterDatabase.escape_string(sPName);
    CharacterDatabase.escape_string(last_ip);
    CharacterDatabase.PExecute("INSERT INTO character_bgqueue (playerGUID, playerName, playerIP, BGtype, action, time) "
                               " VALUES ('%u', '%s', '%s', '%u', '%u', '%u')",
                               plr->GetGUIDLow(), sPName.c_str(), last_ip.c_str(), BgTypeId, uiAction, queuing_time);
    // CharacterDatabase.CommitTransaction(); // Pas de commit de transaction sans un BeginTransaction avant.
}

//remove player from queue and from group info, if group info is empty then remove it too
void BattleGroundQueue::RemovePlayer(ObjectGuid guid, bool decreaseInvitedCount)
{
    //Player *plr = sObjectMgr.GetPlayer(guid);
    //ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_Lock);

    int32 bracket_id = -1;                                     // signed for proper for-loop finish
    QueuedPlayersMap::iterator itr;

    //remove player from map, if he's there
    itr = m_QueuedPlayers.find(guid);
    if (itr == m_QueuedPlayers.end())
    {
        sLog.outError("BattleGroundQueue: couldn't find for remove: %s", guid.GetString().c_str());
        return;
    }

    GroupQueueInfo* group = itr->second.GroupInfo;
    GroupsQueueType::iterator group_itr, group_itr_tmp;
    // mostly people with the highest levels are in battlegrounds, thats why
    // we count from MAX_BATTLEGROUND_QUEUES - 1 to 0
    // variable index removes useless searching in other team's queue
    uint32 index = BattleGround::GetTeamIndexByTeamId(group->GroupTeam);

    for (int32 bracket_id_tmp = MAX_BATTLEGROUND_BRACKETS - 1; bracket_id_tmp >= 0 && bracket_id == -1; --bracket_id_tmp)
    {
        //we must check premade and normal team's queue - because when players from premade are joining bg,
        //they leave groupinfo so we can't use its players size to find out index
        for (uint32 j = index; j < BG_QUEUE_GROUP_TYPES_COUNT; j += BG_QUEUE_NORMAL_ALLIANCE)
        {
            for (group_itr_tmp = m_QueuedGroups[bracket_id_tmp][j].begin(); group_itr_tmp != m_QueuedGroups[bracket_id_tmp][j].end(); ++group_itr_tmp)
            {
                if ((*group_itr_tmp) == group)
                {
                    bracket_id = bracket_id_tmp;
                    group_itr = group_itr_tmp;
                    //we must store index to be able to erase iterator
                    index = j;
                    break;
                }
            }
        }
    }
    //player can't be in queue without group, but just in case
    if (bracket_id == -1)
    {
        sLog.outError("BattleGroundQueue: ERROR Cannot find groupinfo for %s", guid.GetString().c_str());
        return;
    }
    DEBUG_LOG("BattleGroundQueue: Removing %s, from bracket_id %u", guid.GetString().c_str(), (uint32)bracket_id);

    // ALL variables are correctly set
    // We can ignore leveling up in queue - it should not cause crash
    // remove player from group
    // if only one player there, remove group

    // remove player queue info from group queue info
    GroupQueueInfoPlayers::iterator pitr = group->Players.find(guid);
    if (pitr != group->Players.end())
        group->Players.erase(pitr);

    // if invited to bg, and should decrease invited count, then do it
    if (decreaseInvitedCount && group->IsInvitedToBGInstanceGUID)
    {
        BattleGround* bg = sBattleGroundMgr.GetBattleGround(group->IsInvitedToBGInstanceGUID, group->BgTypeId);
        if (bg)
            bg->DecreaseInvitedCount(group->GroupTeam);
    }

    // remove player queue info
    m_QueuedPlayers.erase(itr);

    // remove group queue info if needed
    if (group->Players.empty())
    {
        m_QueuedGroups[bracket_id][index].erase(group_itr);
        delete group;
    }
}

//returns true when player pl_guid is in queue and is invited to bgInstanceGuid
bool BattleGroundQueue::IsPlayerInvited(ObjectGuid pl_guid, const uint32 bgInstanceGuid, const uint32 removeTime)
{
    //ACE_Guard<ACE_Recursive_Thread_Mutex> g(m_Lock);
    QueuedPlayersMap::const_iterator qItr = m_QueuedPlayers.find(pl_guid);
    return (qItr != m_QueuedPlayers.end()
            && qItr->second.GroupInfo->IsInvitedToBGInstanceGUID == bgInstanceGuid
            && qItr->second.GroupInfo->RemoveInviteTime == removeTime);
}

bool BattleGroundQueue::GetPlayerGroupInfoData(ObjectGuid guid, GroupQueueInfo* ginfo)
{
    //ACE_Guard<ACE_Recursive_Thread_Mutex> g(m_Lock);
    QueuedPlayersMap::const_iterator qItr = m_QueuedPlayers.find(guid);
    if (qItr == m_QueuedPlayers.end())
        return false;
    *ginfo = *(qItr->second.GroupInfo);
    return true;
}

bool BattleGroundQueue::InviteGroupToBG(GroupQueueInfo * ginfo, BattleGround * bg, Team side)
{
    // set side if needed
    if (side)
        ginfo->GroupTeam = side;

    if (!ginfo->IsInvitedToBGInstanceGUID)
    {
        // not yet invited
        // set invitation
        ginfo->IsInvitedToBGInstanceGUID = bg->GetInstanceID();
        BattleGroundTypeId bgTypeId = bg->GetTypeID();
        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bgTypeId);
        BattleGroundBracketId bracket_id = bg->GetBracketId();

        ginfo->RemoveInviteTime = WorldTimer::getMSTime() + INVITE_ACCEPT_WAIT_TIME;

        // loop through the players
        for (GroupQueueInfoPlayers::iterator itr = ginfo->Players.begin(); itr != ginfo->Players.end(); ++itr)
        {
            // get the player
            Player* plr = ObjectAccessor::FindPlayer(itr->first);

            // if offline, skip him
            if (!plr || !plr->GetSession()->IsConnected())
                continue;

            // if logging out, skip him
            if (plr->GetSession()->isLogingOut())
                continue;

            // if already in battleground, skip him
            if (plr->InBattleGround())
                continue;

            // set invited player counters
            bg->IncreaseInvitedCount(ginfo->GroupTeam);

            // invite the player
            PlayerInvitedToBGUpdateAverageWaitTime(ginfo, bracket_id);
            //sBattleGroundMgr.InvitePlayer(plr, bg, ginfo->Team);

            plr->SetInviteForBattleGroundQueueType(bgQueueTypeId, ginfo->IsInvitedToBGInstanceGUID);

            // create remind invite events
            BGQueueInviteEvent* inviteEvent = new BGQueueInviteEvent(plr->GetObjectGuid(), ginfo->IsInvitedToBGInstanceGUID, bgTypeId, ginfo->RemoveInviteTime);
            plr->m_Events.AddEvent(inviteEvent, plr->m_Events.CalculateTime(INVITATION_REMIND_TIME));
            // create automatic remove events
            BGQueueRemoveEvent* removeEvent = new BGQueueRemoveEvent(plr->GetObjectGuid(), ginfo->IsInvitedToBGInstanceGUID, bgTypeId, bgQueueTypeId, ginfo->RemoveInviteTime);
            plr->m_Events.AddEvent(removeEvent, plr->m_Events.CalculateTime(INVITE_ACCEPT_WAIT_TIME));

            if (bgTypeId == BATTLEGROUND_BR)
            {
                plr->m_Events.AddLambdaEventAtOffset([plr, bgTypeId]
                {
                    if (plr->IsInWorld() && !plr->InBattleGround() &&
                        plr->GetSession()->IsConnected() &&
                        !plr->GetSession()->isLogingOut())
                    {
                        WorldPacket* data = new WorldPacket(CMSG_BATTLEFIELD_PORT);
                        *data << uint32(GetBattleGrounMapIdByTypeId(bgTypeId));
                        *data << uint8(1);
                        plr->GetSession()->QueuePacket(std::move(data));
                    }
                }, 1000);
            }

            WorldPacket data;

            uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);

            DEBUG_LOG("Battleground: invited %s to BG instance %u queueindex %u bgtype %u, I can't help it if they don't press the enter battle button.",
                      plr->GetGuidStr().c_str(), bg->GetInstanceID(), queueSlot, bg->GetTypeID());

            // send status packet
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME, 0);
            plr->GetSession()->SendPacket(&data);
        }
        return true;
    }

    return false;
}

/*
This function is inviting players to already running battlegrounds
Invitation type is based on config file
large groups are disadvantageous, because they will be kicked first if invitation type = 1
*/
void BattleGroundQueue::FillPlayersToBG(BattleGround* bg, BattleGroundBracketId bracket_id)
{
    // Never allow players to join into an arena.
    if (bg->IsArena())
        return;

    int32 hordeFree = bg->GetFreeSlotsForTeam(HORDE);
    int32 aliFree   = bg->GetFreeSlotsForTeam(ALLIANCE);

    int8 bg_differential = 0; // default differential between Horde & Alliance

    if (bg->GetTypeID() == BATTLEGROUND_AV)
        bg_differential = 3;    // custom differential between Horde & Alliance for Alterac Valley


    //iterator for iterating through bg queue
    GroupsQueueType::const_iterator Ali_itr = m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE].begin();
    //count of groups in queue - used to stop cycles
    uint32 aliCount = m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE].size();
    //index to queue which group is current
    uint32 aliIndex = 0;
    for (; aliIndex < aliCount && m_SelectionPools[BG_TEAM_ALLIANCE].AddGroup((*Ali_itr), aliFree, bg->GetClientInstanceID()); aliIndex++)
        ++Ali_itr;
    //the same thing for horde
    GroupsQueueType::const_iterator Horde_itr = m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_HORDE].begin();
    uint32 hordeCount = m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_HORDE].size();
    uint32 hordeIndex = 0;
    for (; hordeIndex < hordeCount && m_SelectionPools[BG_TEAM_HORDE].AddGroup((*Horde_itr), hordeFree, bg->GetClientInstanceID()); hordeIndex++)
        ++Horde_itr;

    //if ofc like BG queue invitation is set in config, then we are happy
    if (sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_INVITATION_TYPE) == 0)
        return;

    /*
    if we reached this code, then we have to solve NP - complete problem called Subset sum problem
    So one solution is to check all possible invitation subgroups, or we can use these conditions:
    1. Last time when BattleGroundQueue::Update was executed we invited all possible players - so there is only small possibility
        that we will invite now whole queue, because only 1 change has been made to queues from the last BattleGroundQueue::Update call
    2. Other thing we should consider is group order in queue
    */

    // At first we need to compare free space in bg and our selection pool
    int32 diffAli   = aliFree   - int32(m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount());
    int32 diffHorde = hordeFree - int32(m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount());
    while (abs(diffAli - diffHorde) > bg_differential && (m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount() > 0 || m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount() > 0))
    {
        //each cycle execution we need to kick at least 1 group
        if (diffAli < diffHorde)
        {
            //kick alliance group, add to pool new group if needed
            if (m_SelectionPools[BG_TEAM_ALLIANCE].KickGroup(diffHorde - diffAli))
            {
                for (; aliIndex < aliCount && m_SelectionPools[BG_TEAM_ALLIANCE].AddGroup((*Ali_itr), (aliFree >= diffHorde) ? aliFree - diffHorde : 0, bg->GetClientInstanceID()); aliIndex++)
                    ++Ali_itr;
            }
            //if ali selection is already empty, then kick horde group, but if there are less horde than ali in bg - break;
            if (!m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount())
            {
                if (aliFree <= diffHorde + 1)
                    break;
                m_SelectionPools[BG_TEAM_HORDE].KickGroup(diffHorde - diffAli);
            }
        }
        else
        {
            //kick horde group, add to pool new group if needed
            if (m_SelectionPools[BG_TEAM_HORDE].KickGroup(diffAli - diffHorde))
            {
                for (; hordeIndex < hordeCount && m_SelectionPools[BG_TEAM_HORDE].AddGroup((*Horde_itr), (hordeFree >= diffAli) ? hordeFree - diffAli : 0, bg->GetClientInstanceID()); hordeIndex++)
                    ++Horde_itr;
            }
            if (!m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount())
            {
                if (hordeFree <= diffAli + 1)
                    break;
                m_SelectionPools[BG_TEAM_ALLIANCE].KickGroup(diffAli - diffHorde);
            }
        }
        //count diffs after small update
        diffAli   = aliFree   - int32(m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount());
        diffHorde = hordeFree - int32(m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount());
    }
}

// this method checks if premade versus premade battleground is possible
// then after 30 mins (default) in queue it moves premade group to normal queue
// it tries to invite as much players as it can - to MaxPlayersPerTeam, because premade groups have more than MinPlayersPerTeam players
bool BattleGroundQueue::CheckPremadeMatch(BattleGroundBracketId bracket_id, uint32 MinPlayersPerTeam, uint32 MaxPlayersPerTeam)
{
    GroupsQueueType::const_iterator itr_team[BG_TEAMS_COUNT];
    for (uint32 queueType = 0; queueType < 2; ++queueType)
        for (uint32 i = 0; i < BG_TEAMS_COUNT; i++)
        {
            itr_team[i] = m_QueuedGroups[bracket_id][2*queueType + i].begin();
            for (; itr_team[i] != m_QueuedGroups[bracket_id][2*queueType + i].end(); ++(itr_team[i]))
            {
                if (!(*(itr_team[i]))->IsInvitedToBGInstanceGUID && (*(itr_team[i]))->Players.size() >= sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMADE_QUEUE_GROUP_MIN_SIZE))
                {
                    m_SelectionPools[i].AddGroup(*(itr_team[i]), MaxPlayersPerTeam, 0);
                    if (m_SelectionPools[i].GetPlayerCount() >= MinPlayersPerTeam)
                        break;
                }
            }
        }

    //allow 1v0 if debug bg
    if (sBattleGroundMgr.isTesting() && (m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount() || m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount()))
        return true;

    //return true if there are enough players in selection pools - enable to work .debug bg command correctly
    if (m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount() >= MinPlayersPerTeam && m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount() >= MinPlayersPerTeam)
        return true;

    // now check if we can move group from Premade queue to normal queue (timer has expired) or group size lowered!!
    // this could be 2 cycles but i'm checking only first team in queue - it can cause problem -
    // if first is invited to BG and seconds timer expired, but we can ignore it, because players have only 80 seconds to click to enter bg
    // and when they click or after 80 seconds the queue info is removed from queue
    uint32 time_before = WorldTimer::getMSTime() - sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH);
    for (uint32 i = 0; i < BG_TEAMS_COUNT; i++)
    {
        if (!m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE_ALLIANCE + i].empty())
        {
            GroupsQueueType::iterator itr = m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE_ALLIANCE + i].begin();
            if (!(*itr)->IsInvitedToBGInstanceGUID && ((*itr)->JoinTime < time_before || (*itr)->Players.size() < MinPlayersPerTeam))
            {
                //we must insert group to normal queue and erase pointer from premade queue
                m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE + i].push_back((*itr));
                m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE_ALLIANCE + i].erase(itr);
            }
        }
    }
    //selection pools are not set
    return false;
}

// this method tries to create battleground with MinPlayersPerTeam against MinPlayersPerTeam
bool BattleGroundQueue::CheckNormalMatch(BattleGroundBracketId bracket_id, uint32 minPlayers, uint32 maxPlayers)
{
    GroupsQueueType::const_iterator itr_team[BG_TEAMS_COUNT];
    for (uint32 i = 0; i < BG_TEAMS_COUNT; i++)
    {
        itr_team[i] = m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE + i].begin();
        for (; itr_team[i] != m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE + i].end(); ++(itr_team[i]))
        {
            if (!(*(itr_team[i]))->IsInvitedToBGInstanceGUID)
            {
                m_SelectionPools[i].AddGroup(*(itr_team[i]), maxPlayers, 0);
                if (m_SelectionPools[i].GetPlayerCount() >= minPlayers)
                    break;
            }
        }
    }
    //try to invite same number of players - this cycle may cause longer wait time even if there are enough players in queue, but we want ballanced bg
    uint32 j = BG_TEAM_ALLIANCE;
    if (m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount() < m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount())
        j = BG_TEAM_HORDE;
    if ((sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_INVITATION_TYPE) != 0)
            && m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount() >= minPlayers && m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount() >= minPlayers)
    {
        //we will try to invite more groups to team with less players indexed by j
        ++(itr_team[j]);                                         //this will not cause a crash, because for cycle above reached break;
        for (; itr_team[j] != m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE + j].end(); ++(itr_team[j]))
        {
            if (!(*(itr_team[j]))->IsInvitedToBGInstanceGUID)
                if (!m_SelectionPools[j].AddGroup(*(itr_team[j]), m_SelectionPools[(j + 1) % BG_TEAMS_COUNT].GetPlayerCount(), 0))
                    break;
        }
        // do not allow to start bg with more than 1 players more on 1 faction
        if (abs((int32)(m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount() - m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount())) > 1)
            return false;
    }
    //allow 1v0 if debug bg
    if (sBattleGroundMgr.isTesting() && (m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount() || m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount()))
        return true;

    //return true if there are enough players in selection pools - enable to work .debug bg command correctly
    return m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount() >= minPlayers && m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount() >= minPlayers;
}

/*
this method is called when group is inserted, or player / group is removed from BG Queue - there is only one player's status changed, so we don't use while(true) cycles to invite whole queue
it must be called after fully adding the members of a group to ensure group joining
should be called from BattleGround::RemovePlayer function in some cases
*/
void BattleGroundQueue::Update(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id)
{
    //ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_Lock);
    // First, remove old offline players
    QueuedPlayersMap::iterator itrOffline = m_QueuedPlayers.begin();
    while (itrOffline != m_QueuedPlayers.end())
    {
        if (!itrOffline->second.online && WorldTimer::getMSTimeDiffToNow(itrOffline->second.LastOnlineTime) > OFFLINE_BG_QUEUE_TIME)
        {
            RemovePlayer(itrOffline->first, true);
            itrOffline = m_QueuedPlayers.begin();
        }
        else
            ++itrOffline;
    }

    // if no players in queue - do nothing
    if (IsAllQueuesEmpty(bracket_id))
        return;

    auto InviteAllGroupsToBg = [this](BattleGround* bg)
    {
        // invite those selection pools
        for (uint32 i = 0; i < BG_TEAMS_COUNT; i++)
        {
            for (auto const& citr : m_SelectionPools[BG_TEAM_ALLIANCE + i].SelectedGroups)
            {
                InviteGroupToBG(citr, bg, citr->GroupTeam);
            }
        }
    };

    if (sWorld.getConfig(CONFIG_BOOL_BATTLEGROUND_RANDOMIZE))
    {
        auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE].begin(),
            m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE].end(),
            std::default_random_engine(seed));
        std::shuffle(m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_HORDE].begin(),
            m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_HORDE].end(),
            std::default_random_engine(seed));
    }

    //battleground with free slot for player should be always in the beginning of the queue
    // maybe it would be better to create bgfreeslotqueue for each bracket_id
    BGFreeSlotQueueType::iterator itr, next;
    for (itr = sBattleGroundMgr.BGFreeSlotQueue[bgTypeId].begin(); itr != sBattleGroundMgr.BGFreeSlotQueue[bgTypeId].end(); itr = next)
    {
        next = itr;
        ++next;
        // battleground is running, so if:
        if ((*itr)->GetTypeID() == bgTypeId && (*itr)->GetBracketId() == bracket_id &&
                (*itr)->GetStatus() > STATUS_WAIT_QUEUE && (*itr)->GetStatus() < STATUS_WAIT_LEAVE)
        {
            BattleGround* bg = *itr; //we have to store battleground pointer here, because when battleground is full, it is removed from free queue (not yet implemented!!)
            // and iterator is invalid

            // clear selection pools
            m_SelectionPools[BG_TEAM_ALLIANCE].Init();
            m_SelectionPools[BG_TEAM_HORDE].Init();

            // call a function that does the job for us
            FillPlayersToBG(bg, bracket_id);

            // now everything is set, invite players
            InviteAllGroupsToBg(bg);

            if (!bg->HasFreeSlots())
            {
                // remove BG from BGFreeSlotQueue
                bg->RemoveFromBGFreeSlotQueue();
            }
        }
    }

    // finished iterating through the bgs with free slots, maybe we need to create a new bg

    BattleGround* bg_template = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if (!bg_template)
    {
        sLog.outError("Battleground: Update: bg template not found for %u", bgTypeId);
        return;
    }
    // get the min. players per team, properly for larger arenas as well.
    uint32 MinPlayersPerTeam = bg_template->GetMinPlayersPerTeam();
    uint32 MaxPlayersPerTeam = bg_template->GetMaxPlayersPerTeam();

    int normalMatchesCreationAttempts = 1;
    // now check if there are in queues enough players to start new game of (normal battleground)
    if (bgTypeId == BATTLEGROUND_AV && sWorld.getConfig(CONFIG_UINT32_AV_MIN_PLAYERS_IN_QUEUE) && !sBattleGroundMgr.isTesting())
    {
        int minPlayersInQueue = sWorld.getConfig(CONFIG_UINT32_AV_MIN_PLAYERS_IN_QUEUE);
        int playersInQueuePerTeam[BG_TEAMS_COUNT] = {0};
        for (uint32 i = 0; i < BG_TEAMS_COUNT; i++)
        {
            GroupsQueueType::const_iterator itr = m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE + i].begin();
            for (; itr != m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE + i].end(); ++itr)
                if (!(*itr)->IsInvitedToBGInstanceGUID)
                    playersInQueuePerTeam[i]++; // Only one player, because premades are not allowed in AV.
        }
        if (playersInQueuePerTeam[BG_TEAM_ALLIANCE] < minPlayersInQueue ||
                playersInQueuePerTeam[BG_TEAM_HORDE] < minPlayersInQueue)
            normalMatchesCreationAttempts = 0;
        else
        {
            // Now randomize
            auto seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::shuffle(m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE].begin(),
                m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE].begin() + minPlayersInQueue,
                std::default_random_engine(seed));
            std::shuffle(m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_HORDE].begin(),
                m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_HORDE].begin() + minPlayersInQueue,
                std::default_random_engine(seed));
            sLog.out(LOG_BG, "Alterac queue randomized (%u alliance vs %u horde)",
                m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_ALLIANCE].size(),
                m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL_HORDE].size());
        }
    }
    if (bgTypeId == BATTLEGROUND_AV && sWorld.getConfig(CONFIG_UINT32_AV_INITIAL_MAX_PLAYERS) && !sBattleGroundMgr.isTesting() && normalMatchesCreationAttempts)
    {
        MaxPlayersPerTeam = sWorld.getConfig(CONFIG_UINT32_AV_INITIAL_MAX_PLAYERS);
        if (MaxPlayersPerTeam <= MinPlayersPerTeam)
            MaxPlayersPerTeam = MinPlayersPerTeam;
        normalMatchesCreationAttempts = 2;
    }
    if (sBattleGroundMgr.isTesting())
        MinPlayersPerTeam = 1;

    m_SelectionPools[BG_TEAM_ALLIANCE].Init();
    m_SelectionPools[BG_TEAM_HORDE].Init();

    uint32 q_min_level = Player::GetMinLevelForBattleGroundBracketId(bracket_id, bgTypeId);
    uint32 q_max_level = Player::GetMaxLevelForBattleGroundBracketId(bracket_id, bgTypeId);

    //check if there is premade against premade match
    if (CheckPremadeMatch(bracket_id, MinPlayersPerTeam, MaxPlayersPerTeam))
    {
        //create new battleground
        BattleGround* bg2 = sBattleGroundMgr.CreateNewBattleGround(bgTypeId, bracket_id);
        if (!bg2)
        {
            sLog.outError("BattleGroundQueue::Update - Cannot create battleground: %u", bgTypeId);
            return;
        }
        //invite those selection pools
        InviteAllGroupsToBg(bg2);

        //start bg
        bg2->SetLevelRange(q_min_level, q_max_level - 1);
        bg2->StartBattleGround();

        // clear structures
        m_SelectionPools[TEAM_ALLIANCE].Init();
        m_SelectionPools[TEAM_HORDE].Init();
    }


    for (int attempt = 0; attempt < normalMatchesCreationAttempts; ++attempt)
    {
        // if there are enough players in pools, start new battleground or non rated arena
        if (CheckNormalMatch(bracket_id, MinPlayersPerTeam, MaxPlayersPerTeam))
        {
            // we successfully created a pool
            BattleGround* bg2 = sBattleGroundMgr.CreateNewBattleGround(bgTypeId, bracket_id);
            if (!bg2)
            {
                sLog.outError("BattleGroundQueue::Update - Cannot create battleground: %u", bgTypeId);
                return;
            }

            // invite those selection pools
            InviteAllGroupsToBg(bg2);

            // start bg
            bg2->SetLevelRange(q_min_level, q_max_level - 1);
            bg2->StartBattleGround();

            // clear structures
            m_SelectionPools[BG_TEAM_ALLIANCE].Init();
            m_SelectionPools[BG_TEAM_HORDE].Init();
        }
    }
}

/*********************************************************/
/***            BATTLEGROUND QUEUE EVENTS              ***/
/*********************************************************/

bool BGQueueInviteEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Player* plr = ObjectAccessor::FindPlayerNotInWorld(m_PlayerGuid);
    // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
    if (!plr)
        return true;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(m_BgInstanceGUID, m_BgTypeId);
    //if battleground ended and its instance deleted - do nothing
    if (!bg)
        return true;

    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID());
    uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);
    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)         // player is in queue or in battleground
    {
        // check if player is invited to this bg
        BattleGroundQueue &bgQueue = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId];
        if (bgQueue.IsPlayerInvited(m_PlayerGuid, m_BgInstanceGUID, m_RemoveTime))
        {
            WorldPacket data;
            //we must send remaining time in queue
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME - INVITATION_REMIND_TIME, 0);
            plr->GetSession()->SendPacket(&data);
        }
    }
    return true;                                            //event will be deleted
}

void BGQueueInviteEvent::Abort(uint64 /*e_time*/)
{
    //do nothing
}

/*
    this event has many possibilities when it is executed:
    1. player is in battleground ( he clicked enter on invitation window )
    2. player left battleground queue and he isn't there any more
    3. player left battleground queue and he joined it again and IsInvitedToBGInstanceGUID = 0
    4. player left queue and he joined again and he has been invited to same battleground again -> we should not remove him from queue yet
    5. player is invited to bg and he didn't choose what to do and timer expired - only in this condition we should call queue::RemovePlayer
    we must remove player in the 5. case even if battleground object doesn't exist!
*/
bool BGQueueRemoveEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Player* plr = ObjectAccessor::FindPlayerNotInWorld(m_PlayerGuid);
    if (!plr)
        // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
        return true;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(m_BgInstanceGUID, m_BgTypeId);
    //battleground can be deleted already when we are removing queue info
    //bg pointer can be nullptr! so use it carefully!

    uint32 queueSlot = plr->GetBattleGroundQueueIndex(m_BgQueueTypeId);
    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)         // player is in queue, or in Battleground
    {
        // check if player is in queue for this BG and if we are removing his invite event
        BattleGroundQueue &bgQueue = sBattleGroundMgr.m_BattleGroundQueues[m_BgQueueTypeId];
        if (bgQueue.IsPlayerInvited(m_PlayerGuid, m_BgInstanceGUID, m_RemoveTime))
        {
            DEBUG_LOG("Battleground: removing player %u from bg queue for instance %u because of not pressing enter battle in time.", plr->GetGUIDLow(), m_BgInstanceGUID);

            plr->RemoveBattleGroundQueueId(m_BgQueueTypeId);
            bgQueue.RemovePlayer(m_PlayerGuid, true);
            //update queues if battleground isn't ended
            if (bg && bg->GetStatus() != STATUS_WAIT_LEAVE)
                sBattleGroundMgr.ScheduleQueueUpdate(m_BgQueueTypeId, m_BgTypeId, bg->GetBracketId());

            WorldPacket data;
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, queueSlot, STATUS_NONE, 0, 0);
            plr->GetSession()->SendPacket(&data);
        }
    }

    //event will be deleted
    return true;
}

void BGQueueRemoveEvent::Abort(uint64 /*e_time*/)
{
    //do nothing
}

/*********************************************************/
/***            BATTLEGROUND MANAGER                   ***/
/*********************************************************/

BattleGroundMgr::BattleGroundMgr()
{
    for (uint32 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; i++)
        m_BattleGrounds[i].clear();
    m_Testing = false;
}

BattleGroundMgr::~BattleGroundMgr()
{
    DeleteAllBattleGrounds();
}

void BattleGroundMgr::DeleteAllBattleGrounds()
{
    // will also delete template bgs:
    for (uint32 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
    {
        for (BattleGroundSet::iterator itr = m_BattleGrounds[i].begin(); itr != m_BattleGrounds[i].end();)
        {
            BattleGround * bg = itr->second;
            ++itr;                                          // step from invalidate iterator pos in result element remove in ~BattleGround call
            delete bg;
        }
    }
}

// used to update running battlegrounds, and delete finished ones
void BattleGroundMgr::Update(uint32 diff)
{
    // update scheduled queues
    if (!m_QueueUpdateScheduler.empty())
    {
        std::vector<uint64> scheduled;
        std::swap(scheduled, m_QueueUpdateScheduler);

        for (uint64 i = 0; i < scheduled.size(); i++)
        {
            BattleGroundQueueTypeId bgQueueTypeId = BattleGroundQueueTypeId(scheduled[i] >> 16 & 255);
            BattleGroundTypeId bgTypeId = BattleGroundTypeId((scheduled[i] >> 8) & 255);
            BattleGroundBracketId bracket_id = BattleGroundBracketId(scheduled[i] & 255);
            m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId, bracket_id);
        }
    }
}

void BattleGroundMgr::BuildBattleGroundStatusPacket(WorldPacket* data, BattleGround *bg, uint8 queueSlot, uint8 statusId, uint32 time1, uint32 time2)
{
    // we can be in 3 queues in same time...
    if (statusId == STATUS_NONE || !bg)
    {
        data->Initialize(SMSG_BATTLEFIELD_STATUS, 4 * 2);
        *data << uint32(queueSlot);                         // queue id (0...2)
        *data << uint32(0);
        return;
    }

    data->Initialize(SMSG_BATTLEFIELD_STATUS, (4 + 1 + 1 + 4 + 2 + 4 + 1 + 4 + 4 + 4));
    *data << uint32(queueSlot);                             // queue id (0...2) - player can be in 3 queues in time
    *data << uint32(bg->GetMapId());
    *data << uint8(bg->GetBracketId());
    *data << uint32(bg->GetClientInstanceID());
    *data << uint32(statusId);
    switch (statusId)
    {
        case STATUS_WAIT_QUEUE:                             // status_in_queue
            *data << uint32(time1);                         // average wait time, milliseconds
            *data << uint32(time2);                         // time in queue, updated every minute!, milliseconds
            break;
        case STATUS_WAIT_JOIN:                              // status_invite
            *data << uint32(time1);                         // time to remove from queue, milliseconds
            break;
        case STATUS_IN_PROGRESS:                            // status_in_progress
            *data << uint32(time1);                         // time to bg auto leave, 0 at bg start, 120000 after bg end, milliseconds
            *data << uint32(time2);                         // time from bg start, milliseconds
            break;
        default:
            sLog.outError("Unknown BG status!");
            break;
    }
}

void BattleGroundMgr::BuildPvpLogDataPacket(WorldPacket *data, BattleGround *bg)
{
    data->Initialize(MSG_PVP_LOG_DATA, (1 + 4 + 40 * bg->GetPlayerScoresSize()));

    if (bg->GetStatus() != STATUS_WAIT_LEAVE)
    {
        *data << uint8(0);                                  // bg not ended
    }
    else
    {
        *data << uint8(1);                                  // bg ended
        *data << uint8(bg->GetWinner());                    // who win
    }

    // If the arena match has not started, write no player data and return until match begins.
    if (bg->IsArena() && bg->GetStatus() == STATUS_WAIT_JOIN)
    {
        *data << uint32(0);
        return;
    }

    uint32 count = bg->GetPlayerScoresSize();
    if (count >= 80) // Client has a hard limit to 80. If we go beyond (but it should not happen ?!), WoW Error (happening !)
        count = 80;
    *data << (uint32)(count);

    for (BattleGround::BattleGroundScoreMap::const_iterator itr = bg->GetPlayerScoresBegin(); itr != bg->GetPlayerScoresEnd(); ++itr)
    {
        if (!count)
            break;

        --count;
        BattleGroundScore const* score = itr->second;

        *data << ObjectGuid(itr->first);

        Player *plr = ObjectAccessor::FindPlayerNotInWorld(itr->first);

        *data << uint32(plr ? plr->GetHonorMgr().GetRank().rank : 4);
        *data << uint32(score->KillingBlows);
        *data << uint32(score->HonorableKills);
        *data << uint32(score->Deaths);
        *data << uint32(score->BonusHonor);

        switch (bg->GetTypeID())                             // battleground specific things
        {
            case BATTLEGROUND_AV:
                *data << (uint32)0x00000007;                // count of next fields
                *data << (uint32)((BattleGroundAVScore*)score)->GraveyardsAssaulted;  // GraveyardsAssaulted
                *data << (uint32)((BattleGroundAVScore*)score)->GraveyardsDefended;   // GraveyardsDefended
                *data << (uint32)((BattleGroundAVScore*)score)->TowersAssaulted;      // TowersAssaulted
                *data << (uint32)((BattleGroundAVScore*)score)->TowersDefended;       // TowersDefended
                *data << (uint32)((BattleGroundAVScore*)score)->SecondaryObjectives;  // Mines Taken
                *data << (uint32)((BattleGroundAVScore*)score)->LieutnantCount;       // Lieutnant kills
                *data << (uint32)((BattleGroundAVScore*)score)->SecondaryNPC;         // Secondary unit summons
                break;
            case BATTLEGROUND_WS:
                *data << (uint32)0x00000002;                // count of next fields
                *data << (uint32)((BattleGroundWGScore*)score)->FlagCaptures;         // flag captures
                *data << (uint32)((BattleGroundWGScore*)score)->FlagReturns;          // flag returns
                break;
            case BATTLEGROUND_AB:
                *data << (uint32)0x00000002;                // count of next fields
                *data << (uint32)((BattleGroundABScore*)score)->BasesAssaulted;       // bases asssulted
                *data << (uint32)((BattleGroundABScore*)score)->BasesDefended;        // bases defended
                break;
            default:
                DEBUG_LOG("Unhandled MSG_PVP_LOG_DATA for BG id %u", bg->GetTypeID());
                *data << (uint32)0;
                break;
        }
    }
}

void BattleGroundMgr::BuildGroupJoinedBattlegroundPacket(WorldPacket *data, int32 status)
{
    data->Initialize(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
    // for status, see enum BattleGroundGroupJoinStatus
    *data << int32(status);
}

void BattleGroundMgr::BuildUpdateWorldStatePacket(WorldPacket *data, uint32 field, uint32 value)
{
    data->Initialize(SMSG_UPDATE_WORLD_STATE, 4 + 4);
    *data << uint32(field);
    *data << uint32(value);
}

void BattleGroundMgr::BuildPlaySoundPacket(WorldPacket *data, uint32 soundid)
{
    data->Initialize(SMSG_PLAY_SOUND, 4);
    *data << uint32(soundid);
}

void BattleGroundMgr::BuildPlayerLeftBattleGroundPacket(WorldPacket *data, ObjectGuid guid)
{
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_LEFT, 8);
    *data << ObjectGuid(guid);
}

void BattleGroundMgr::BuildPlayerJoinedBattleGroundPacket(WorldPacket *data, Player *plr)
{
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_JOINED, 8);
    *data << plr->GetObjectGuid();
}

BattleGround * BattleGroundMgr::GetBattleGroundThroughClientInstance(uint32 instanceId, BattleGroundTypeId bgTypeId)
{
    //cause at HandleBattleGroundJoinOpcode the clients sends the instanceid he gets from
    //SMSG_BATTLEFIELD_LIST we need to find the battleground with this clientinstance-id
    BattleGround* bg = GetBattleGroundTemplate(bgTypeId);
    if (!bg)
        return nullptr;

    for (const auto& itr : m_BattleGrounds[bgTypeId])
    {
        if (itr.second->GetClientInstanceID() == instanceId)
            return itr.second;
    }

    return nullptr;
}

BattleGround * BattleGroundMgr::GetBattleGround(uint32 InstanceID, BattleGroundTypeId bgTypeId)
{
    //search if needed
    BattleGroundSet::iterator itr;
    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        for (uint32 i = BATTLEGROUND_AV; i < MAX_BATTLEGROUND_TYPE_ID; i++)
        {
            itr = m_BattleGrounds[i].find(InstanceID);
            if (itr != m_BattleGrounds[i].end())
                return itr->second;
        }
        return nullptr;
    }
    itr = m_BattleGrounds[bgTypeId].find(InstanceID);
    return ((itr != m_BattleGrounds[bgTypeId].end()) ? itr->second : nullptr);
}

BattleGround * BattleGroundMgr::GetBattleGroundTemplate(BattleGroundTypeId bgTypeId)
{
    //map is sorted and we can be sure that lowest instance id has only BG template
    return m_BattleGrounds[bgTypeId].empty() ? nullptr : m_BattleGrounds[bgTypeId].begin()->second;
}

uint32 BattleGroundMgr::CreateClientVisibleInstanceId(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracketId)
{
    // here, we create an instanceid, which is just for
    // displaying this to the client and without any other use.
    // the client-instanceIds are unique for each battleground-type
    // the instance-id just needs to be as low as possible, beginning with 1
    // the following works, because std::set is default ordered with "<"
    // the optimization would be to use as bitmask std::vector<uint32> - but that would only make code unreadable
    uint32 lastId = 1;
    ClientBattleGroundIdSet& ids = m_ClientBattleGroundIds[bgTypeId][bracketId];
    for (auto id : ids)
    {
        if (lastId == id)
            lastId++;
        else
            break;
    }
    ids.insert(lastId);
    return lastId;
}

// create a new battleground that will really be used to play
BattleGround * BattleGroundMgr::CreateNewBattleGround(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id)
{
    // get the template BG
    BattleGround *bg_template = GetBattleGroundTemplate(bgTypeId);
    if (!bg_template)
    {
        sLog.outError("BattleGround: CreateNewBattleGround - bg template not found for %u", bgTypeId);
        return nullptr;
    }

    BattleGround *bg = nullptr;
    // create a copy of the BG template
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV:
            bg = new BattleGroundAV(*(BattleGroundAV*)bg_template);
            break;
        case BATTLEGROUND_WS:
            bg = new BattleGroundWS(*(BattleGroundWS*)bg_template);
            break;
        case BATTLEGROUND_AB:
            bg = new BattleGroundAB(*(BattleGroundAB*)bg_template);
            break;
        case BATTLEGROUND_BR:
            bg = new BattleGroundBR(*(BattleGroundBR*)bg_template);
            break;
        case BATTLEGROUND_SV:
            bg = new BattleGroundSV(*(BattleGroundSV*)bg_template);
            break;
        default:
            //error, but it is handled few lines above
            return 0;
    }

    bg->Reset();
    bg->SetClientInstanceID(CreateClientVisibleInstanceId(bgTypeId, bracket_id));

    // start the joining of the bg
    bg->SetStatus(STATUS_WAIT_JOIN);
    bg->SetBracketId(bracket_id);

    // will also set m_bgMap, instanceid
    sMapMgr.CreateBgMap(bg->GetMapId(), bg);
    return bg;
}

// used to create the BG templates
uint32 BattleGroundMgr::CreateBattleGround(BattleGroundTypeId bgTypeId, uint32 minPlayersPerTeam, uint32 maxPlayersPerTeam, uint32 levelMin, uint32 levelMax, uint32 allianceWinSpell, uint32 allianceLoseSpell, uint32 hordeWinSpell, uint32 hordeLoseSpell, char const* battleGroundName, uint32 mapID, float team1StartLocX, float team1StartLocY, float team1StartLocZ, float team1StartLocO, float team2StartLocX, float team2StartLocY, float team2StartLocZ, float team2StartLocO, uint32 playerSkinReflootId)
{
    // Create the BG
    BattleGround *bg = nullptr;
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV:
            bg = new BattleGroundAV;
            break;
        case BATTLEGROUND_WS:
            bg = new BattleGroundWS;
            break;
        case BATTLEGROUND_AB:
            bg = new BattleGroundAB;
            break;
        case BATTLEGROUND_BR:
            bg = new BattleGroundBR;
            break;
        case BATTLEGROUND_SV:
            bg = new BattleGroundSV;
            break;
        default:
            sLog.outError("Could not find BG Template for bgTypeID %u.", static_cast<uint32>(bgTypeId));
            break;                           // placeholder for non implemented BG
    }


    if (!bg)
        return 0;

    bg->SetMapId(mapID);
    bg->SetTypeID(bgTypeId);
    bg->SetMinPlayersPerTeam(minPlayersPerTeam);
    bg->SetMaxPlayersPerTeam(maxPlayersPerTeam);
    bg->SetMinPlayers(minPlayersPerTeam * 2);
    bg->SetMaxPlayers(maxPlayersPerTeam * 2);
    bg->SetAllianceWinSpell(allianceWinSpell);
    bg->SetAllianceLoseSpell(allianceLoseSpell);
    bg->SetHordeWinSpell(hordeWinSpell);
    bg->SetHordeLoseSpell(hordeLoseSpell);
    bg->SetName(battleGroundName);
    bg->SetTeamStartLoc(ALLIANCE, team1StartLocX, team1StartLocY, team1StartLocZ, team1StartLocO);
    bg->SetTeamStartLoc(HORDE,    team2StartLocX, team2StartLocY, team2StartLocZ, team2StartLocO);
    bg->SetLevelRange(levelMin, levelMax);
    bg->SetPlayerSkinRefLootId(playerSkinReflootId);

    // add bg to update list
    AddBattleGround(bg->GetInstanceID(), bg->GetTypeID(), bg);

    // return some not-nullptr value, bgTypeId is good enough for me
    return bgTypeId;
}

void BattleGroundMgr::ReloadBGPlayerCounts()
{
    std::unique_ptr<QueryResult> res{ WorldDatabase.Query("SELECT `id`, `min_players_per_team`, `max_players_per_team` FROM `battleground_template`") };

    if (!res)
        return;

    std::unordered_map<uint32, std::pair<uint32, uint32>> newPlayerCounts;


    do {
        auto fields = res->Fetch();
        newPlayerCounts[fields[0].GetUInt32()] = std::make_pair(fields[1].GetUInt32(), fields[2].GetUInt32());
        
    } while (res->NextRow());

    for (auto& bgSet : m_BattleGrounds)
    {
        for (auto& bg : bgSet)
        {
            if (newPlayerCounts.find(bg.second->GetTypeID()) == newPlayerCounts.end())
                continue;

            bg.second->SetMinPlayersPerTeam(newPlayerCounts[bg.second->GetTypeID()].first);
            bg.second->SetMaxPlayersPerTeam(newPlayerCounts[bg.second->GetTypeID()].second);
        }
    }
}

void BattleGroundMgr::CreateInitialBattleGrounds()
{
    //                                                                0     1                       2                       3            4            5                     6                      7                  8                   9                          10
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `id`, `min_players_per_team`, `max_players_per_team`, `min_level`, `max_level`, `alliance_win_spell`, `alliance_lose_spell`, `horde_win_spell`, `horde_lose_spell`, `alliance_start_location`, `horde_start_location`, `player_loot_id` FROM `battleground_template`"));

    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 bgTypeID_ = fields[0].GetUInt32();

        BattleGroundTypeId bgTypeID = BattleGroundTypeId(bgTypeID_);

        uint32 MinPlayersPerTeam = fields[1].GetUInt32();
        uint32 MaxPlayersPerTeam = fields[2].GetUInt32();
        uint32 MinLvl = fields[3].GetUInt32();
        uint32 MaxLvl = fields[4].GetUInt32();
        uint32 AllianceWinSpell  = fields[5].GetUInt32();
        uint32 AllianceLoseSpell = fields[6].GetUInt32();
        uint32 HordeWinSpell  = fields[7].GetUInt32();
        uint32 HordeLoseSpell = fields[8].GetUInt32();

        float AStartLoc[4];
        float HStartLoc[4];

        uint32 start1 = fields[9].GetUInt32();

        WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry(start1);
        if (start)
        {
            AStartLoc[0] = start->x;
            AStartLoc[1] = start->y;
            AStartLoc[2] = start->z;
            AStartLoc[3] = sObjectMgr.GetWorldSafeLocFacing(start->ID);
        }
        // TEMP Overrdie for Sunnyglade Valley
        else if (start1 == 700)
        {
            AStartLoc[0] = 1677.1f;
            AStartLoc[1] = 444.5f;
            AStartLoc[2] = 115.0f;
            AStartLoc[3] = 4.4F;
        }
        // Override for arena
        else if (start1 == 9000)
        {
            AStartLoc[0] = 15682.37f;
            AStartLoc[1] = 16796.68f;
            AStartLoc[2] = 3.317f;
            AStartLoc[3] = 6.23f;
        }
        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u have nonexistent WorldSafeLocs.dbc id %u in field `AllianceStartLoc`. BG not created.", bgTypeID, start1);
            continue;
        }

        uint32 start2 = fields[10].GetUInt32();

        start = sWorldSafeLocsStore.LookupEntry(start2);
        if (start)
        {
            HStartLoc[0] = start->x;
            HStartLoc[1] = start->y;
            HStartLoc[2] = start->z;
            HStartLoc[3] = sObjectMgr.GetWorldSafeLocFacing(start->ID);
        }
        // TEMP Overrdie for Sunnyglade Valley
        else if (start2 == 701)
        {
            HStartLoc[0] = 956.5f;
            HStartLoc[1] = 157.1f;
            HStartLoc[2] = 100.2f;
            HStartLoc[3] = 0.5f;
        }
        else if (start2 == 9001)
        {
            HStartLoc[0] = 15792.27f;
            HStartLoc[1] = 16747.01f;
            HStartLoc[2] = 3.284f;
            HStartLoc[3] = 3.17f;
        }
        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `HordeStartLoc`. BG not created.", bgTypeID, start2);
            continue;
        }

        uint32 playerSkinReflootId = fields[11].GetUInt32();
        if (playerSkinReflootId && !ExistsRefLootTemplate(playerSkinReflootId))
        {
            playerSkinReflootId = 0;
            sLog.outErrorDb("Table `battleground_template` for id %u associated with nonexistent refloot id %u. Setting to 0.", bgTypeID, playerSkinReflootId);
        }

        if (playerSkinReflootId)
            m_usedRefloot.insert(playerSkinReflootId);

        uint32 mapId = GetBattleGrounMapIdByTypeId(bgTypeID);
        char const* name;

        if (MapEntry const* mapEntry = sMapStorage.LookupEntry<MapEntry>(mapId))
        {
            // Override for arena map. Give it a prettier name.
            if (mapId == 26)
                name = "Blood Ring";
            else
                name = mapEntry->name;
        }

        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u associated with nonexistent map id %u.", bgTypeID, mapId);
            continue;
        }

        //DETAIL_LOG("Creating battleground %s, %u-%u", bl->name[sWorld.GetDBClang()], MinLvl, MaxLvl);
        if (!CreateBattleGround(bgTypeID, MinPlayersPerTeam, MaxPlayersPerTeam, MinLvl, MaxLvl, AllianceWinSpell, AllianceLoseSpell, HordeWinSpell, HordeLoseSpell, name, mapId, AStartLoc[0], AStartLoc[1], AStartLoc[2], AStartLoc[3], HStartLoc[0], HStartLoc[1], HStartLoc[2], HStartLoc[3], playerSkinReflootId))
            continue;
    }
    while (result->NextRow());

}

void BattleGroundMgr::BuildBattleGroundListPacket(WorldPacket *data, ObjectGuid guid, Player* plr, BattleGroundTypeId bgTypeId)
{
    if (!plr)
        return;

    uint32 mapId = GetBattleGrounMapIdByTypeId(bgTypeId);

    data->Initialize(SMSG_BATTLEFIELD_LIST);
    *data << guid;                                          // battlemaster guid
    *data << uint32(mapId);
    *data << uint8(plr->GetBattleGroundBracketIdFromLevel(bgTypeId));

    size_t countPos = data->wpos();
    uint32 count = 0;
    *data << uint32(0); // number of bg instances

    uint32 bracketId = plr->GetBattleGroundBracketIdFromLevel(bgTypeId);
    ClientBattleGroundIdSet const& ids = m_ClientBattleGroundIds[bgTypeId][bracketId];
    for (const auto id : ids)
    {
        *data << uint32(id);
        ++count;
    }
    data->put<uint32>(countPos, count);
}

void BattleGroundMgr::SendToBattleGround(Player *pl, uint32 instanceId, BattleGroundTypeId bgTypeId)
{
    BattleGround *bg = GetBattleGround(instanceId, bgTypeId);
    if (bg)
    {
        uint32 mapid = bg->GetMapId();
        float x, y, z, o;
        Team team = pl->GetBGTeam();
        if (team == 0)
            team = pl->GetTeam();
        bg->GetTeamStartLoc(team, x, y, z, o);

        DETAIL_LOG("BATTLEGROUND: Sending %s to map %u, X %f, Y %f, Z %f, O %f", pl->GetName(), mapid, x, y, z, o);
        pl->TeleportTo(mapid, x, y, z, o);
    }
    else
        sLog.outError("player %u trying to port to nonexistent bg instance %u", pl->GetGUIDLow(), instanceId);
}

BattleGroundQueueTypeId BattleGroundMgr::BGQueueTypeId(BattleGroundTypeId bgTypeId)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_WS:
            return BATTLEGROUND_QUEUE_WS;
        case BATTLEGROUND_AB:
            return BATTLEGROUND_QUEUE_AB;
        case BATTLEGROUND_AV:
            return BATTLEGROUND_QUEUE_AV;
        case BATTLEGROUND_BR:
            return ARENA_QUEUE_BR;
        case BATTLEGROUND_SV:
            return BATTLEGROUND_QUEUE_SV;
        default:
            return BATTLEGROUND_QUEUE_NONE;
    }
}

BattleGroundTypeId BattleGroundMgr::BGTemplateId(BattleGroundQueueTypeId bgQueueTypeId)
{
    switch (bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_WS:
            return BATTLEGROUND_WS;
        case BATTLEGROUND_QUEUE_AB:
            return BATTLEGROUND_AB;
        case BATTLEGROUND_QUEUE_AV:
            return BATTLEGROUND_AV;
        case ARENA_QUEUE_BR:
            return BATTLEGROUND_BR;
        case BATTLEGROUND_QUEUE_SV:
            return BATTLEGROUND_SV;
        default:
            return BattleGroundTypeId(0);                   // used for unknown template (it exist and do nothing)
    }
}

void BattleGroundMgr::ToggleTesting()
{
    m_Testing = !m_Testing;
    if (m_Testing)
        sWorld.SendWorldText(LANG_DEBUG_BG_ON);
    else
        sWorld.SendWorldText(LANG_DEBUG_BG_OFF);
}

void BattleGroundMgr::ScheduleQueueUpdate(BattleGroundQueueTypeId bgQueueTypeId, BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id)
{
    uint64 const schedule_id = ((uint64)bgQueueTypeId << 16) | ((uint64)bgTypeId << 8) | (uint64)bracket_id;

    if (std::find(m_QueueUpdateScheduler.begin(), m_QueueUpdateScheduler.end(), schedule_id) == m_QueueUpdateScheduler.end())
        m_QueueUpdateScheduler.emplace_back(schedule_id);
}

uint32 BattleGroundMgr::GetPrematureFinishTime() const
{
    return sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMATURE_FINISH_TIMER);
}

void BattleGroundMgr::LoadBattleMastersEntry()
{
    mBattleMastersMap.clear();                              // need for reload case

    QueryResult *result = WorldDatabase.Query("SELECT entry,bg_template FROM battlemaster_entry");

    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId  = fields[1].GetUInt32();
        if (bgTypeId >= MAX_BATTLEGROUND_TYPE_ID)
        {
            sLog.outErrorDb("Table `battlemaster_entry` contain entry %u for nonexistent battleground type %u, ignored.", entry, bgTypeId);
            continue;
        }

        mBattleMastersMap[entry] = BattleGroundTypeId(bgTypeId);

    }
    while (result->NextRow());

    delete result;    
}

HolidayIds BattleGroundMgr::BGTypeToWeekendHolidayId(BattleGroundTypeId bgTypeId)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV:
            return HOLIDAY_CALL_TO_ARMS_AV;
        case BATTLEGROUND_WS:
            return HOLIDAY_CALL_TO_ARMS_WS;
        case BATTLEGROUND_AB:
            return HOLIDAY_CALL_TO_ARMS_AB;
        case BATTLEGROUND_BR:
            return HOLIDAY_CALL_TO_ARMS_BR;
        case BATTLEGROUND_SV:
            return HOLIDAY_CALL_TO_ARMS_SV;
        default:
            return HOLIDAY_NONE;
    }
}

BattleGroundTypeId BattleGroundMgr::WeekendHolidayIdToBGType(HolidayIds holiday)
{
    switch (holiday)
    {
        case HOLIDAY_CALL_TO_ARMS_AV:
            return BATTLEGROUND_AV;
        case HOLIDAY_CALL_TO_ARMS_WS:
            return BATTLEGROUND_WS;
        case HOLIDAY_CALL_TO_ARMS_AB:
            return BATTLEGROUND_AB;
        case HOLIDAY_CALL_TO_ARMS_BR:
            return BATTLEGROUND_BR;
        case HOLIDAY_CALL_TO_ARMS_SV:
            return BATTLEGROUND_SV;
        default:
            return BATTLEGROUND_TYPE_NONE;
    }
}

bool BattleGroundMgr::IsBGWeekend(BattleGroundTypeId bgTypeId)
{
    return sGameEventMgr.IsActiveHoliday(BGTypeToWeekendHolidayId(bgTypeId));
}

void BattleGroundMgr::LoadBattleEventIndexes()
{
    BattleGroundEventIdx events;
    events.event1 = BG_EVENT_NONE;
    events.event2 = BG_EVENT_NONE;
    m_GameObjectBattleEventIndexMap.clear();             // need for reload case
    m_GameObjectBattleEventIndexMap[-1].push_back(events);
    m_CreatureBattleEventIndexMap.clear();               // need for reload case
    m_CreatureBattleEventIndexMap[-1].push_back(events);

    QueryResult *result =
        //                           0         1           2                3                4              5           6
        WorldDatabase.Query("SELECT data.typ, data.guid1, data.ev1 AS ev1, data.ev2 AS ev2, data.map AS m, data.guid2, description.map, "
                            //                              7                  8                   9
                            "description.event1, description.event2, description.description "
                            "FROM "
                            "(SELECT '1' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM gameobject_battleground AS a "
                            "LEFT OUTER JOIN gameobject AS b ON a.guid = b.guid "
                            "UNION "
                            "SELECT '2' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM creature_battleground AS a "
                            "LEFT OUTER JOIN creature AS b ON a.guid = b.guid "
                            ") data "
                            "RIGHT OUTER JOIN battleground_events AS description ON data.map = description.map "
                            "AND data.ev1 = description.event1 AND data.ev2 = description.event2 "
                            // full outer join doesn't work in mysql :-/ so just UNION-select the same again and add a left outer join
                            "UNION "
                            "SELECT data.typ, data.guid1, data.ev1, data.ev2, data.map, data.guid2, description.map, "
                            "description.event1, description.event2, description.description "
                            "FROM "
                            "(SELECT '1' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM gameobject_battleground AS a "
                            "LEFT OUTER JOIN gameobject AS b ON a.guid = b.guid "
                            "UNION "
                            "SELECT '2' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM creature_battleground AS a "
                            "LEFT OUTER JOIN creature AS b ON a.guid = b.guid "
                            ") data "
                            "LEFT OUTER JOIN battleground_events AS description ON data.map = description.map "
                            "AND data.ev1 = description.event1 AND data.ev2 = description.event2 "
                            "ORDER BY m, ev1, ev2");
    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();
        if (fields[2].GetUInt8() == BG_EVENT_NONE || fields[3].GetUInt8() == BG_EVENT_NONE)
            continue;                                       // we don't need to add those to the eventmap

        bool gameobject         = (fields[0].GetUInt8() == 1);
        uint32 dbTableGuidLow   = fields[1].GetUInt32();
        events.event1           = fields[2].GetUInt8();
        events.event2           = fields[3].GetUInt8();
        uint32 map              = fields[4].GetUInt32();

        uint32 desc_map = fields[6].GetUInt32();
        uint8 desc_event1 = fields[7].GetUInt8();
        uint8 desc_event2 = fields[8].GetUInt8();
        const char *description = fields[9].GetString();

        // checking for nullptr - through right outer join this will mean following:
        if (fields[5].GetUInt32() != dbTableGuidLow)
        {
            sLog.outErrorDb("BattleGroundEvent: %s with nonexistent guid %u for event: map:%u, event1:%u, event2:%u (\"%s\")",
                            (gameobject) ? "gameobject" : "creature", dbTableGuidLow, map, events.event1, events.event2, description);
            continue;
        }

        // checking for nullptr - through full outer join this can mean 2 things:
        if (desc_map != map)
        {
            // there is an event missing
            if (dbTableGuidLow == 0)
            {
                sLog.outErrorDb("BattleGroundEvent: missing db-data for map:%u, event1:%u, event2:%u (\"%s\")", desc_map, desc_event1, desc_event2, description);
                continue;
            }
            // we have an event which shouldn't exist
            else
            {
                sLog.outErrorDb("BattleGroundEvent: %s with guid %u is registered, for a nonexistent event: map:%u, event1:%u, event2:%u",
                                (gameobject) ? "gameobject" : "creature", dbTableGuidLow, map, events.event1, events.event2);
                continue;
            }
        }

        if (gameobject)
            m_GameObjectBattleEventIndexMap[dbTableGuidLow].push_back(events);
        else
            m_CreatureBattleEventIndexMap[dbTableGuidLow].push_back(events);

    }
    while (result->NextRow());

    delete result;
}

// Offline BG queue system
void BattleGroundMgr::PlayerLoggedIn(Player* player)
{
    for (int i = 1; i <= PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        if (m_BattleGroundQueues[i].PlayerLoggedIn(player))
        {
            GroupQueueInfo groupInfo;
            m_BattleGroundQueues[i].GetPlayerGroupInfoData(player->GetObjectGuid(), &groupInfo);
            uint32 avgTime   = m_BattleGroundQueues[i].GetAverageQueueWaitTime(&groupInfo, player->GetBattleGroundBracketIdFromLevel(groupInfo.BgTypeId));
            // Send "in queue" packet.
            uint32 queueSlot = player->AddBattleGroundQueueId(BattleGroundQueueTypeId(i));

            WorldPacket data;
            BattleGround* bg = GetBattleGroundTemplate(groupInfo.BgTypeId);
            BuildBattleGroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, WorldTimer::getMSTimeDiff(groupInfo.JoinTime, WorldTimer::getMSTime()));
            player->GetSession()->SendPacket(&data);

            if (groupInfo.IsInvitedToBGInstanceGUID)
                player->SetInviteForBattleGroundQueueType(BattleGroundQueueTypeId(i), groupInfo.IsInvitedToBGInstanceGUID);
        }
}

void BattleGroundMgr::PlayerLoggedOut(Player* player)
{
    for (int queueSlot = 0; queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES; ++queueSlot)
    {
        BattleGroundQueueTypeId bgQueueTypeId = player->GetBattleGroundQueueTypeId(queueSlot);
        if (bgQueueTypeId != BATTLEGROUND_QUEUE_NONE)
        {
            player->RemoveBattleGroundQueueId(bgQueueTypeId);  // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
            m_BattleGroundQueues[bgQueueTypeId].RemovePlayer(player->GetObjectGuid(), true);

            // player left queue, we should update it
            BattleGroundTypeId bgTypeId = BattleGroundMgr::BGTemplateId(bgQueueTypeId);
            sBattleGroundMgr.ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, player->GetBattleGroundBracketIdFromLevel(bgTypeId, player->GetLevel()));
        }
    }
}

void BattleGroundQueue::PlayerLoggedOut(ObjectGuid guid)
{
    QueuedPlayersMap::iterator itr;

    //remove player from map, if he's there
    itr = m_QueuedPlayers.find(guid);
    if (itr == m_QueuedPlayers.end())
    {
        sLog.outError("BattleGroundQueue: couldn't find for remove: %s", guid.GetString().c_str());
        return;
    }
    itr->second.LastOnlineTime  = WorldTimer::getMSTime();
    itr->second.online          = false;
}

bool BattleGroundQueue::PlayerLoggedIn(Player* player)
{
    QueuedPlayersMap::iterator itr;

    //remove player from map, if he's there
    itr = m_QueuedPlayers.find(player->GetObjectGuid());
    if (itr == m_QueuedPlayers.end())
        return false;

    itr->second.online          = true;
    return true;
}

bool BattleGroundQueue::IsAllQueuesEmpty(BattleGroundBracketId bracket_id)
{
    uint8 queueEmptyCount = 0;

    for (uint8 i = 0; i < BG_QUEUE_MAX; i++)
        if (m_QueuedGroups[bracket_id][i].empty())
            queueEmptyCount++;

    return queueEmptyCount == BG_QUEUE_MAX;
}

void BattleGroundMgr::AddBattleGround(uint32 InstanceID, BattleGroundTypeId bgTypeId, BattleGround* BG)
{
    std::lock_guard<std::mutex> guard(m_BattleGroundsMutex);
    m_BattleGrounds[bgTypeId][InstanceID] = BG;
};

void BattleGroundMgr::RemoveBattleGround(uint32 instanceID, BattleGroundTypeId bgTypeId)
{
    std::lock_guard<std::mutex> guard(m_BattleGroundsMutex);
    m_BattleGrounds[bgTypeId].erase(instanceID);
}
