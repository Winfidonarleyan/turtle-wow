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

#include "ThreatManager.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Map.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "UnitEvents.h"
#include "TargetedMovementGenerator.h"

#include "Chat.h"
#include "GridSearchers.h"

//==============================================================
//================= ThreatCalcHelper ===========================
//==============================================================

float ThreatCalcHelper::CalcThreat(Unit* pHatedUnit, float threat, bool crit, SpellSchoolMask schoolMask, SpellEntry const *pThreatSpell)
{
    // all flat mods applied early
    if (!threat)
        return 0.0f;

    if (pThreatSpell)
    {
        if (Player* modOwner = pHatedUnit->GetSpellModOwner())
            modOwner->ApplySpellMod(pThreatSpell->Id, SPELLMOD_THREAT, threat);

        if (crit)
            threat *= pHatedUnit->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_CRITICAL_THREAT, schoolMask);
    }

    threat = pHatedUnit->ApplyTotalThreatModifier(threat, schoolMask);
    return threat;
}

//============================================================
//================= HostileReference ==========================
//============================================================

HostileReference::HostileReference(Unit* pUnit, ThreatManager *pThreatManager, float pThreat)
{
    iThreat = pThreat;
    iTempThreatModifyer = 0.0f;
    link(pUnit, pThreatManager);
    iUnitGuid = pUnit->GetObjectGuid();
    iOnline = true;
    iAccessible = true;
}

//============================================================
// Tell our refTo (target) object that we have a link
void HostileReference::targetObjectBuildLink()
{
    getTarget()->AddHatedBy(this);
}

//============================================================
// Tell our refTo (taget) object, that the link is cut
void HostileReference::targetObjectDestroyLink()
{
    getTarget()->RemoveHatedBy(this);
}

//============================================================
// Tell our refFrom (source) object, that the link is cut (Target destroyed)

void HostileReference::sourceObjectDestroyLink()
{
    setOnlineOfflineState(false);
}

//============================================================
// Inform the source, that the status of the reference changed

void HostileReference::fireStatusChanged(ThreatRefStatusChangeEvent& pThreatRefStatusChangeEvent)
{
    if (getSource())
        getSource()->processThreatEvent(&pThreatRefStatusChangeEvent);
}

//============================================================

void HostileReference::addThreat(float pMod)
{
    iThreat += pMod;
    // the threat is changed. Source and target unit have to be availabe
    // if the link was cut before relink it again
    if (!isOnline())
        updateOnlineStatus();
    if (pMod != 0.0f)
    {
        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_THREAT_CHANGE, this, pMod);
        fireStatusChanged(event);
    }

    if (isValid() && pMod >= 0)
    {
        Unit* victim_owner = getTarget()->GetOwner();
        if (victim_owner && victim_owner->IsAlive())
            getSource()->addThreat(victim_owner, 0.0f);     // create a threat to the owner of a pet, if the pet attacks
    }
}

//============================================================
// check, if source can reach target and set the status

void HostileReference::updateOnlineStatus()
{
    bool online = false;
    bool accessible = false;

    if (!isValid())
    {
        if (Unit* target = ObjectAccessor::GetUnit(*getSourceUnit(), getUnitGuid()))
            link(target, getSource());
    }
    // only check for online status if
    // ref is valid
    // target is no player or not gamemaster
    // target is not in flight
    if (isValid() &&
            ((getTarget()->GetTypeId() != TYPEID_PLAYER || !((Player*)getTarget())->IsGameMaster()) ||
             !getTarget()->IsTaxiFlying()))
        online = true;

    setAccessibleState(accessible);
    setOnlineOfflineState(online);
}

//============================================================
// set the status and fire the event on status change

void HostileReference::setOnlineOfflineState(bool pIsOnline)
{
    if (iOnline != pIsOnline)
    {
        iOnline = pIsOnline;
        if (!iOnline)
            setAccessibleState(false);                      // if not online that not accessable as well

        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_ONLINE_STATUS, this);
        fireStatusChanged(event);
    }
}

//============================================================

void HostileReference::setAccessibleState(bool pIsAccessible)
{
    if (iAccessible != pIsAccessible)
    {
        iAccessible = pIsAccessible;

        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_ASSECCIBLE_STATUS, this);
        fireStatusChanged(event);
    }
}

//============================================================
// prepare the reference for deleting
// this is called be the target

void HostileReference::removeReference()
{
    invalidate();

    ThreatRefStatusChangeEvent event(UEV_THREAT_REF_REMOVE_FROM_LIST, this);
    fireStatusChanged(event);
}

//============================================================

Unit* HostileReference::getSourceUnit()
{
    return (getSource()->getOwner());
}

//============================================================
//================ ThreatContainer ===========================
//============================================================

void ThreatContainer::clearReferences()
{
    for (ThreatList::const_iterator i = iThreatList.begin(); i != iThreatList.end(); ++i)
    {
        (*i)->unlink();
        delete(*i);
    }
    iThreatList.clear();
}

//============================================================
// Return the HostileReference of nullptr, if not found
HostileReference* ThreatContainer::getReferenceByTarget(Unit* pVictim)
{
    if (!pVictim)
        return nullptr;

    HostileReference* result = nullptr;
    ObjectGuid guid = pVictim->GetObjectGuid();
    for (const auto i : iThreatList)
    {
        if (i->getUnitGuid() == guid)
        {
            result = i;
            break;
        }
    }

    return result;
}

//============================================================
// Add the threat, if we find the reference

HostileReference* ThreatContainer::addThreat(Unit* pVictim, float pThreat)
{
    if (!pVictim)
        return nullptr;

    HostileReference* ref = getReferenceByTarget(pVictim);
    if (ref)
        ref->addThreat(pThreat);
    return ref;
}

//============================================================

void ThreatContainer::modifyThreatPercent(Unit *pVictim, int32 pPercent)
{
    if (HostileReference* ref = getReferenceByTarget(pVictim))
    {
        if (pPercent < -100)
        {
            ref->removeReference();
            delete ref;
        }
        else
            ref->addThreatPercent(pPercent);
    }
}

//============================================================

bool HostileReferenceSortPredicate(const HostileReference* lhs, const HostileReference* rhs)
{
    // std::list::sort ordering predicate must be: (Pred(x,y)&&Pred(y,x))==false
    return lhs->getThreat() > rhs->getThreat();             // reverse sorting
}

//============================================================
// Check if the list is dirty and sort if necessary

void ThreatContainer::update()
{
    if (iDirty && iThreatList.size() > 1)
        iThreatList.sort(HostileReferenceSortPredicate);
    iDirty = false;
}

//============================================================
// return the next best victim
// could be the current victim

HostileReference* ThreatContainer::selectNextVictim(Creature* pAttacker, HostileReference* pCurrentVictim)
{
    if (!pAttacker)
        return nullptr;

    HostileReference* currentRef = nullptr;
    bool found = false;
    bool allowLowPriorityTargets = false;
    bool attackerImmobilized = pAttacker->HasUnitState(UNIT_STAT_CAN_NOT_MOVE);

    for (int attempt = 0; attempt < 2 && !found; ++attempt)
    {
        if (attempt) // Second attempt
            allowLowPriorityTargets = true;

        for (ThreatList::const_iterator iter = iThreatList.begin(); iter != iThreatList.end() && !found;)
        {
            currentRef = (*iter);

            Unit* target = currentRef->getTarget();
            MANGOS_ASSERT(target);                              // if the ref has status online the target must be there !

            bool outOfThreatArea = pAttacker->IsOutOfThreatArea(target);
            bool validAttackTarget = pAttacker->IsValidAttackTarget(target);

            if (outOfThreatArea)
                return nullptr;

            if (!validAttackTarget)
            {
                if (currentRef == pCurrentVictim)
                    pCurrentVictim = nullptr;
                ++iter;
                continue;
            }

            float attackDistance = pAttacker->GetMaxChaseDistance(target);
            // Skip this unit if low priority
            if (!allowLowPriorityTargets && (target->IsImmuneToDamage(pAttacker->GetMeleeDamageSchoolMask()) ||
                                            target->IsSecondaryThreatTarget() ||
                                           (attackerImmobilized && !target->IsWithinDist(pAttacker, attackDistance))))
            {
                // current victim is a second choice target, so don't compare threat with it below
                if (currentRef == pCurrentVictim)
                    pCurrentVictim = nullptr;
                ++iter;
                continue;
            }

            if (pCurrentVictim)                             // select 1.3/1.1 better target in comparison current target
            {
                // list sorted and and we check current target, then this is best case
                if (pCurrentVictim == currentRef || currentRef->getThreat() <= 1.1f * pCurrentVictim->getThreat())
                {
                    currentRef = pCurrentVictim;            // for second case
                    found = true;
                    break;
                }

                if ((currentRef->getThreat() > 1.3f * pCurrentVictim->getThreat()) || (currentRef->getThreat() > 1.1f * pCurrentVictim->getThreat() && pAttacker->CanReachWithMeleeAutoAttack(target)))
                {
                    //implement 110% threat rule for targets in melee range
                    found = true;                           //and 130% rule for targets in ranged distances
                    break;                                  //for selecting alive targets
                }
            }
            else                                            // select any
            {
                found = true;
                break;
            }

            ++iter;
        }
    }

    if (!found)
        currentRef = nullptr;

    return currentRef;
}

//============================================================
//=================== ThreatManager ==========================
//============================================================

ThreatManager::ThreatManager(Unit* owner) : iCurrentVictim(nullptr), iOwner(owner)
{
}

//============================================================

void ThreatManager::clearReferences()
{
    iThreatContainer.clearReferences();
    iThreatOfflineContainer.clearReferences();
    iCurrentVictim = nullptr;
}

//============================================================

void ThreatManager::addThreat(Unit* pVictim, float threat, bool crit, SpellSchoolMask schoolMask, SpellEntry const *pThreatSpell, bool isAssistThreat)
{
    //function deals with adding threat and adding players and pets into ThreatList
    //mobs, NPCs, guards have ThreatList and HateOfflineList
    //players and pets have only InHateListOf
    //HateOfflineList is used co contain unattackable victims (in-flight, in-water, GM etc.)

    // not to self
    if (pVictim == getOwner())
        return;

    // not to GM
    if (!pVictim || (pVictim->GetTypeId() == TYPEID_PLAYER && ((Player*)pVictim)->IsGameMaster()))
        return;

    // not to dead and not for dead
    if (!pVictim->IsAlive() || !getOwner()->IsAlive())
        return;

    MANGOS_ASSERT(getOwner()->GetTypeId() == TYPEID_UNIT);

    // Grace of Earth trinket reduces threat but has no threat attribute.
    if (pThreatSpell && pThreatSpell->HasAttribute(SPELL_ATTR_EX_NO_THREAT) &&
       (threat >= 0.0f || !iThreatContainer.getReferenceByTarget(pVictim)))
        return;

    // don't add assist threat to targets under hard CC
    // check for fear, blind, freezing trap, reckless charge, banish, etc.
    if (isAssistThreat)
        if (getOwner()->HasUnitState(UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING | UNIT_STAT_ISOLATED) || (getOwner()->HasUnitState(UNIT_STAT_STUNNED) && getOwner()->HasBreakableByDamageAuraType(SPELL_AURA_MOD_STUN, 0)))
            threat = 0.0f;
    
    float totalThreat = ThreatCalcHelper::CalcThreat(pVictim, threat, crit, schoolMask, pThreatSpell);
    addThreatDirectly(pVictim, totalThreat);
}

void ThreatManager::UnitDetailedThreatSituation(Creature* creature, Player* requester, int limit, bool tankMode)
{

	if (!creature || !creature->IsInCombat() || !creature->IsElite() || !creature->IsAlive() || !creature->CanHaveThreatList())
        return;

    if (!requester || !requester->IsAlive() || requester->GetTypeId() != TYPEID_PLAYER || !requester->GetGroup())
        return;

    auto hostileTarget = creature->GetThreatManager().getHostileTarget();

    if (!hostileTarget || creature->GetThreatManager().isThreatListEmpty())
        return;

	std::string tankName           = hostileTarget->GetName();
	std::string creatureName       = creature->GetName();
	std::string threatSeparator    = ":";
	std::string rowSeparator       = ";";
	std::string normalModePrefix   = "TWTv4=";  // threat api version
	std::string tankModePrefix     = "TMTv1=";  // tankMode threat api version
	std::string bigPacket		   = normalModePrefix;

	bool isMelee    = true;
	bool isTanking  = false;
	bool inParty    = requester->GetGroup() && !requester->GetGroup()->isRaidGroup();

	int tankThreat  =  0;
	int threatValue =  0;
	int myPos       = -1;
	int position    = -1;                // for player iteration

	float threatPct = 0;
	

	ThreatList const& threatList = creature->GetThreatManager().getThreatList();

	tankThreat = (int)round(creature->GetThreatManager().getThreat(hostileTarget));

	if (tankThreat <= 0)
		return;

	for (ThreatList::const_iterator iter = threatList.begin(); iter != threatList.end(); ++iter)
	{
		myPos++;
		if ((*iter)->getTarget()->GetName() == requester->GetName())
			break;
	}

	// packet struct:
	// TWTv4=da:ta:1;da:ta:2;da:ta:3;...
	// or with tankmode enabled, separated by #
	// TWTv4=da:ta:1;da:ta:2;... # TMTv1=da:ta:1;da:ta2;...

	for (ThreatList::const_iterator iter = threatList.begin(); iter != threatList.end(); ++iter)
	{
		position++;

		// skips !
		if (myPos < limit && position >= limit)							 // skip sending whats over the limit
			continue;
		if (myPos >= limit && position > limit - 2 && position != myPos) // skip sending whats not me
			continue;

		threatValue = (int)round((*iter)->getThreat());

		if (threatValue <= 0)                                             // dont care for negative
			continue;

		isTanking = (*iter)->getTarget()->GetName() == tankName;
		isMelee   = (*iter)->getSourceUnit()->CanReachWithMeleeAutoAttack((*iter)->getTarget());

		threatPct = isTanking ? 100 : threatValue * 100 / (tankThreat * (isMelee ? 1.1 : 1.3));
		threatPct = (float)((int)(threatPct * 10 + .5)) / 10;
		threatPct = threatPct > 100 ? 100 : threatPct;

		bigPacket += (*iter)->getTarget()->GetName() + threatSeparator;           // player name
		bigPacket += std::to_string((int)isTanking) + threatSeparator;            // 1 if player is tanking
		bigPacket += std::to_string(threatValue) + threatSeparator;               // player's threat value, rounded
		bigPacket += std::to_string(threatPct) + threatSeparator;                 // player's threat percent, rounded
		bigPacket += std::to_string(isMelee);                                     // 1 if creature can reach player with melee

		bigPacket += rowSeparator;
	}

	if (bigPacket.length() > 3096 || bigPacket.empty())
		return;

	bigPacket.pop_back(); // remove last rowSeparator
	
	if (tankMode)
	{
		std::list<std::string> pSecondMessage;   // players that are 2nd on threat
		pSecondMessage.clear();

		std::list<Creature*> hCreatureNear;
		GetHostileCreaturesListInRange(hCreatureNear, requester, 5.0f);

		int creatureIndex = 0;

		if (hCreatureNear.size() >= 2)
			for (std::list<Creature*>::iterator iter = hCreatureNear.begin(); iter != hCreatureNear.end(); ++iter)
			{
				if (creatureIndex > 4)
					break;

				if (!(*iter)->CanHaveThreatList() || !(*iter)->IsElite() || !(*iter)->IsInCombat())
					continue;
				if (!(*iter)->GetThreatManager().getHostileTarget() || (*iter)->GetThreatManager().isThreatListEmpty())
					continue;
				if ((*iter)->GetThreatManager().getHostileTarget()->GetName() != requester->GetName())
					continue;

				ThreatList const& hThreatList = (*iter)->GetThreatManager().getThreatList();

				if (hThreatList.size() < 2)
					continue;

				ThreatList::const_iterator hatedPlayers = hThreatList.begin();

				//if im 1st on threat, send 2nd, else im not 1st on threat, send 1st, begin()
				if ((*hatedPlayers)->getTarget()->GetName() == requester->GetName())
					++hatedPlayers;

				int tTankThreat = 0;
				for (ThreatList::const_iterator tankThreatIter = hThreatList.begin(); tankThreatIter != hThreatList.end(); ++tankThreatIter)
					if ((*tankThreatIter)->getTarget()->GetName() == requester->GetName())
					{
						tTankThreat = (int)round((*tankThreatIter)->getThreat());
						break;
					}

				if (tTankThreat <= 0)
					continue;

				bool tIsMelee = (*hatedPlayers)->getSourceUnit()->CanReachWithMeleeAutoAttack((*hatedPlayers)->getTarget());

				float tThreatPct = (*hatedPlayers)->getThreat() * 100 / (tTankThreat * (tIsMelee ? 1.1 : 1.3));
				tThreatPct = (float)((int)(tThreatPct * 10 + .5)) / 10;
				tThreatPct = tThreatPct > 100 ? 100 : tThreatPct;

				std::string tMsg;
				tMsg += (*iter)->GetName() + threatSeparator;						 // creature name
				tMsg += std::to_string((*iter)->GetGUIDLow()) + threatSeparator;	 // creature guid
				tMsg += (*hatedPlayers)->getTarget()->GetName() + threatSeparator;   // player name
				tMsg += std::to_string(tThreatPct);                                  // player's threat percent

				pSecondMessage.push_back(tMsg);

				creatureIndex++;

			}

		if (pSecondMessage.size() > 1)
		{
			bigPacket += "#" + tankModePrefix;
			for (std::list<std::string>::const_iterator itr = pSecondMessage.begin(); itr != pSecondMessage.end(); ++itr)
				bigPacket += (*itr).c_str() + rowSeparator;

			bigPacket.pop_back(); // remove last rowSeparator
		}
	}

	if (bigPacket.length() > 3096 || bigPacket.empty())
		return;

	WorldPacket data;
	ChatHandler::BuildChatPacket(data, inParty ? CHAT_MSG_PARTY : CHAT_MSG_RAID,
		("TWT \t" + bigPacket).c_str(), Language(LANG_ADDON), requester->GetChatTag(),
		requester->GetObjectGuid(), requester->GetName());
	requester->GetSession()->SendPacket(&data);

}

void ThreatManager::addThreatDirectly(Unit* pVictim, float threat)
{
    if (!pVictim || pVictim == getOwner() || !pVictim->IsAlive() || !pVictim->IsInMap(getOwner()))
        return;

    HostileReference* ref = iThreatContainer.addThreat(pVictim, threat);
    // Ref is not in the online refs, search the offline refs next
    if (!ref)
        ref = iThreatOfflineContainer.addThreat(pVictim, threat);

    if (!ref)                                               // there was no ref => create a new one
    {
        // threat has to be 0 here
        HostileReference* hostileReference = new HostileReference(pVictim, this, 0);
        iThreatContainer.addReference(hostileReference);
        hostileReference->addThreat(threat);                // now we add the real threat
        if (pVictim->GetTypeId() == TYPEID_PLAYER && ((Player*)pVictim)->IsGameMaster())
            hostileReference->setOnlineOfflineState(false); // GM is always offline
    }
}

//============================================================

void ThreatManager::modifyThreatPercent(Unit *pVictim, int32 pPercent)
{
    iThreatContainer.modifyThreatPercent(pVictim, pPercent);
}

//============================================================

Unit* ThreatManager::getHostileTarget()
{
    iThreatContainer.update();
    HostileReference* nextVictim = iThreatContainer.selectNextVictim((Creature*) getOwner(), getCurrentVictim());
    setCurrentVictim(nextVictim);
    return getCurrentVictim() != nullptr ? getCurrentVictim()->getTarget() : nullptr;
}

//============================================================

float ThreatManager::getThreat(Unit *pVictim, bool pAlsoSearchOfflineList)
{
    float threat = 0.0f;
    HostileReference* ref = iThreatContainer.getReferenceByTarget(pVictim);
    if (!ref && pAlsoSearchOfflineList)
        ref = iThreatOfflineContainer.getReferenceByTarget(pVictim);
    if (ref)
        threat = ref->getThreat();
    return threat;
}

//============================================================

void ThreatManager::tauntApply(Unit* pTaunter)
{
    if (HostileReference* ref = iThreatContainer.getReferenceByTarget(pTaunter))
    {
        if (getCurrentVictim() && (ref->getThreat() < getCurrentVictim()->getThreat()))
        {
            // Ok, temp threat is unused
            if (ref->getTempThreatModifyer() == 0.0f)
                ref->setTempThreat(getCurrentVictim()->getThreat());
        }
    }
}

//============================================================

void ThreatManager::tauntFadeOut(Unit *pTaunter)
{
    if (HostileReference* ref = iThreatContainer.getReferenceByTarget(pTaunter))
        ref->resetTempThreat();
}

//============================================================

void ThreatManager::setCurrentVictim(HostileReference* pHostileReference)
{
    iCurrentVictim = pHostileReference;
}

//============================================================
// The hated unit is gone, dead or deleted
// return true, if the event is consumed

void ThreatManager::processThreatEvent(ThreatRefStatusChangeEvent* threatRefStatusChangeEvent)
{
    threatRefStatusChangeEvent->setThreatManager(this);     // now we can set the threat manager

    HostileReference* hostileReference = threatRefStatusChangeEvent->getReference();

    switch (threatRefStatusChangeEvent->getType())
    {
        case UEV_THREAT_REF_THREAT_CHANGE:
            if ((getCurrentVictim() == hostileReference && threatRefStatusChangeEvent->getFValue() < 0.0f) ||
                    (getCurrentVictim() != hostileReference && threatRefStatusChangeEvent->getFValue() > 0.0f))
                setDirty(true);                             // the order in the threat list might have changed
            break;
        case UEV_THREAT_REF_ONLINE_STATUS:
            if (!hostileReference->isOnline())
            {
                if (hostileReference == getCurrentVictim())
                {
                    setCurrentVictim(nullptr);
                    setDirty(true);
                }
                iThreatContainer.remove(hostileReference);
                iThreatOfflineContainer.addReference(hostileReference);
            }
            else
            {
                if (getCurrentVictim() && hostileReference->getThreat() > (1.1f * getCurrentVictim()->getThreat()))
                    setDirty(true);
                iThreatContainer.addReference(hostileReference);
                iThreatOfflineContainer.remove(hostileReference);
            }
            break;
        case UEV_THREAT_REF_REMOVE_FROM_LIST:
            if (hostileReference == getCurrentVictim())
            {
                setCurrentVictim(nullptr);
                setDirty(true);
            }
            if (hostileReference->isOnline())
                iThreatContainer.remove(hostileReference);
            else
                iThreatOfflineContainer.remove(hostileReference);
            break;
    }
}

void ThreatManager::setCurrentVictimIfCan(Unit * pVictim)
{
    if (HostileReference* pHRef = iThreatContainer.getReferenceByTarget(pVictim))
    {
        setCurrentVictim(pHRef);
    }
}
