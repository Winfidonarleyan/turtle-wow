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

#include "SpellMgr.h"
#include "ObjectMgr.h"
#include "SpellAuraDefines.h"
#include "DBCStores.h"
#include "World.h"
#include "Chat.h"
#include "Spell.h"
#include "BattleGroundMgr.h"
#include "MapManager.h"
#include "Unit.h"

#include <optional>
#include <variant>

using namespace Spells;

SpellMgr sSpellMgr;

SpellMgr::SpellMgr()
{
}

void SpellMgr::LoadSpellTargetPositions()
{
    mSpellTargetPositions.clear();                                // need for reload case

    //                                                               0   1           2                  3                  4                  5
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT id, target_map, target_position_x, target_position_y, target_position_z, target_orientation FROM spell_target_position"));
    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTargetPosition st;

        st.mapId       = fields[1].GetUInt32();
        st.x           = fields[2].GetFloat();
        st.y           = fields[3].GetFloat();
        st.z           = fields[4].GetFloat();
        st.o           = fields[5].GetFloat();

        MapEntry const* mapEntry = sMapStorage.LookupEntry<MapEntry>(st.mapId);
        if (!mapEntry)
        {
            sLog.outErrorDb("Spell (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.", Spell_ID, st.mapId);
            continue;
        }

        if (st.x == 0 && st.y == 0 && st.z == 0)
        {
            sLog.outErrorDb("Spell (ID:%u) target coordinates not provided.", Spell_ID);
            continue;
        }

        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(Spell_ID);
        if (!spellInfo)
        {
            sLog.outErrorDb("Spell (ID:%u) listed in `spell_target_position` does not exist.", Spell_ID);
            continue;
        }

        bool found = false;
        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spellInfo->EffectImplicitTargetA[i] == TARGET_LOCATION_DATABASE || spellInfo->EffectImplicitTargetB[i] == TARGET_LOCATION_DATABASE)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            sLog.outErrorDb("Spell (Id: %u) listed in `spell_target_position` does not have target TARGET_LOCATION_DATABASE (17).", Spell_ID);
            continue;
        }

        mSpellTargetPositions[Spell_ID] = st;
    } while (result->NextRow());
}

template <typename EntryType, typename WorkerType, typename StorageType>
struct SpellRankHelper
{
    SpellRankHelper(SpellMgr &_mgr, StorageType &_storage): mgr(_mgr), worker(_storage), customRank(0) {}
    void RecordRank(EntryType &entry, uint32 spell_id)
    {
        const SpellEntry *spell = sSpellMgr.GetSpellEntry(spell_id);
        if (!spell)
        {
            sLog.outErrorDb("Spell %u listed in `%s` does not exist", spell_id, worker.TableName());
            return;
        }

        uint32 first_id = mgr.GetFirstSpellInChain(spell_id);

        // most spell ranks expected same data
        if (first_id)
        {
            firstRankSpells.insert(first_id);

            if (first_id != spell_id)
            {
                if (!worker.IsValidCustomRank(entry, spell_id, first_id))
                    return;
                // for later check that first rank also added
                else
                {
                    firstRankSpellsWithCustomRanks.insert(first_id);
                    ++customRank;
                }
            }
        }

        worker.AddEntry(entry, spell);
    }
    void FillHigherRanks()
    {
        // check that first rank added for custom ranks
        for (std::set<uint32>::const_iterator itr = firstRankSpellsWithCustomRanks.begin(); itr != firstRankSpellsWithCustomRanks.end(); ++itr)
            if (!worker.HasEntry(*itr))
                sLog.outErrorDb("Spell %u must be listed in `%s` as first rank for listed custom ranks of spell but not found!", *itr, worker.TableName());

        // fill absent non first ranks data base at first rank data
        for (std::set<uint32>::const_iterator itr = firstRankSpells.begin(); itr != firstRankSpells.end(); ++itr)
        {
            if (worker.SetStateToEntry(*itr))
                mgr.doForHighRanks(*itr, worker);
        }
    }
    std::set<uint32> firstRankSpells;
    std::set<uint32> firstRankSpellsWithCustomRanks;

    SpellMgr &mgr;
    WorkerType worker;
    uint32 customRank;
};

struct DoSpellProcEvent
{
    DoSpellProcEvent(SpellProcEventMap& _spe_map) : spe_map(_spe_map), customProc(0), count(0) {}
    void operator()(uint32 spell_id)
    {
        SpellProcEventEntry const& spe = state->second;
        // add ranks only for not filled data (some ranks have ppm data different for ranks for example)
        SpellProcEventMap::const_iterator spellItr = spe_map.find(spell_id);
        if (spellItr == spe_map.end())
            spe_map[spell_id] = spe;
        // if custom rank data added then it must be same except ppm
        else
        {
            SpellProcEventEntry const& r_spe = spellItr->second;
            if (spe.schoolMask != r_spe.schoolMask)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different schoolMask from first rank in chain", spell_id);

            if (spe.spellFamilyName != r_spe.spellFamilyName)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different spellFamilyName from first rank in chain", spell_id);

            for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                if (spe.spellFamilyMask[i] != r_spe.spellFamilyMask[i])
                {
                    sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different spellFamilyMask from first rank in chain", spell_id);
                    break;
                }
            }

            if (spe.procFlags != r_spe.procFlags)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different procFlags from first rank in chain", spell_id);

            if (spe.procEx != r_spe.procEx)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different procEx from first rank in chain", spell_id);

            // only ppm allowed has been different from first rank

            if (spe.customChance != r_spe.customChance)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different customChance from first rank in chain", spell_id);

            if (spe.cooldown != r_spe.cooldown)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different cooldown from first rank in chain", spell_id);
        }
    }

    const char* TableName()
    {
        return "spell_proc_event";
    }
    bool IsValidCustomRank(SpellProcEventEntry const &spe, uint32 entry, uint32 first_id)
    {
        // let have independent data in table for spells with ppm rates (exist rank dependent ppm rate spells)
        if (!spe.ppmRate)
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_event` is not first rank (%u) in chain", entry, first_id);
            // prevent loading since it won't have an effect anyway
            return false;
        }
        return true;
    }
    void AddEntry(SpellProcEventEntry const &spe, SpellEntry const *spell)
    {
        spe_map[spell->Id] = spe;

        bool isCustom = false;

        if (spe.procFlags == 0)
        {
            if (spell->procFlags == 0)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` probally not triggered spell (no proc flags)", spell->Id);
        }
        else
        {
            if (spell->procFlags == spe.procFlags)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` has exactly same proc flags as in spell.dbc, field value redundant", spell->Id);
            else
                isCustom = true;
        }

        if (spe.customChance == 0)
        {
            /* enable for re-check cases, 0 chance ok for some cases because in some cases it set by another spell/talent spellmod)
            if (spell->procChance==0 && !spe.ppmRate)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` probally not triggered spell (no chance or ppm)", spell->Id);
            */
        }
        else
        {
            if (spell->procChance == spe.customChance)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` has exactly same custom chance as in spell.dbc, field value redundant", spell->Id);
            else
                isCustom = true;
        }

        // totally redundant record
        if (!spe.schoolMask && !spe.procFlags &&
                !spe.procEx && !spe.ppmRate && !spe.customChance && !spe.cooldown)
        {
            bool empty = spe.spellFamilyName == 0;
            for (uint64 i : spe.spellFamilyMask)
            {
                if (i)
                {
                    empty = false;
                    break;
                }
            }
            if (empty)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` not have any useful data", spell->Id);
        }

        if (isCustom)
            ++customProc;
        else
            ++count;
    }

    bool HasEntry(uint32 spellId)
    {
        return spe_map.count(spellId) > 0;
    }
    bool SetStateToEntry(uint32 spellId)
    {
        return (state = spe_map.find(spellId)) != spe_map.end();
    }
    SpellProcEventMap& spe_map;
    SpellProcEventMap::const_iterator state;

    uint32 customProc;
    uint32 count;
};

void SpellMgr::LoadSpellProcEvents()
{
    mSpellProcEventMap.clear();                             // need for reload case

    //                                                               0      1           2                3                 4                 5                 6          7       8        9             10
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT entry, SchoolMask, SpellFamilyName, SpellFamilyMask0, SpellFamilyMask1, SpellFamilyMask2, procFlags, procEx, ppmRate, CustomChance, Cooldown FROM spell_proc_event"));
    if (!result)
    {
        return;
    }

    SpellRankHelper<SpellProcEventEntry, DoSpellProcEvent, SpellProcEventMap> rankHelper(*this, mSpellProcEventMap);

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetUInt32();
        spe.spellFamilyName = fields[2].GetUInt32();

        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
            spe.spellFamilyMask[i] = fields[3 + i].GetUInt64();

        spe.procFlags       = fields[6].GetUInt32();
        spe.procEx          = fields[7].GetUInt32();
        spe.ppmRate         = fields[8].GetFloat();
        spe.customChance    = fields[9].GetFloat();
        spe.cooldown        = fields[10].GetUInt32();

        rankHelper.RecordRank(spe, entry);

    } while (result->NextRow());

    rankHelper.FillHigherRanks();

    OverrideProcEvents();
}




void SpellMgr::OverrideProcEvents()
{
    using FuncType = std::function<void(SpellProcEventEntry*)>;

    static std::unordered_map<uint32, std::variant<uint32, FuncType>> procEventModifiers
    {
        {15268u, [this](SpellProcEventEntry* entry) { // blackout rank 1

            entry->procEx = PROC_EX_NORMAL_HIT; // dont proc on heal.
        }},
        {15323u, 15268u}, // blackout rank 2
        {15324u, 15268u}, // blackout rank 3
        {15325u, 15268u}, // blackout rank 4
        {15326u, 15268u}  // blackout rank 5
    };

    for (auto itr = procEventModifiers.begin(); itr != procEventModifiers.end(); ++itr)
    {
        auto procEventItr = mSpellProcEventMap.find(itr->first);

        if (procEventItr == mSpellProcEventMap.end())
            continue;

        SpellProcEventEntry* entry = &procEventItr->second;

        decltype(itr) ruleset = itr;

        while (auto linkedId = std::get_if<uint32>(&ruleset->second))
            ruleset = procEventModifiers.find(*linkedId);

        if (ruleset == procEventModifiers.end())
            continue;

        if (auto func = std::get_if<FuncType>(&ruleset->second))
            (*func)(entry);
    }
}

struct DoSpellProcItemEnchant
{
    DoSpellProcItemEnchant(SpellProcItemEnchantMap& _procMap, float _ppm) : procMap(_procMap), ppm(_ppm) {}
    void operator()(uint32 spell_id)
    {
        procMap[spell_id] = ppm;
    }

    SpellProcItemEnchantMap& procMap;
    float ppm;
};

void SpellMgr::LoadSpellProcItemEnchant()
{
    mSpellProcItemEnchantMap.clear();                       // need for reload case

    //                                                              0      1
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT entry, ppmRate FROM spell_proc_item_enchant"));
    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        float ppmRate = fields[1].GetFloat();

        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(entry);

        if (!spellInfo)
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_item_enchant` does not exist", entry);
            continue;
        }

        uint32 first_id = GetFirstSpellInChain(entry);

        if (first_id != entry)
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_item_enchant` is not first rank (%u) in chain", entry, first_id);
            // prevent loading since it won't have an effect anyway
            continue;
        }

        mSpellProcItemEnchantMap[entry] = ppmRate;

        // also add to high ranks
        DoSpellProcItemEnchant worker(mSpellProcItemEnchantMap, ppmRate);
        doForHighRanks(entry, worker);
    } while (result->NextRow());
}

bool SpellMgr::IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const * spellProcEvent, uint32 EventProcFlag, SpellEntry const * procSpell, uint32 procFlags, uint32 procExtra)
{
    // Store extra req
    uint32 procEvent_procEx = spellProcEvent ? spellProcEvent->procEx : PROC_EX_NONE;

    // check prockFlags for condition
    if ((procFlags & EventProcFlag) == 0)
        return false;

    // Either procs only on cast end, or only on hit.
    if ((procExtra & PROC_EX_CAST_END) != (procEvent_procEx & PROC_EX_CAST_END))
        return false;

    // Always trigger for this
    if (EventProcFlag & (PROC_FLAG_HEARTBEAT | PROC_FLAG_KILL | PROC_FLAG_ON_TRAP_ACTIVATION))
        return true;

    if (spellProcEvent)     // Exist event data
    {
        // For melee triggers
        if (procSpell == nullptr)
        {
            // Check (if set) for school (melee attack have Normal school)
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & SPELL_SCHOOL_MASK_NORMAL) == 0)
                return false;
        }
        else // For spells need check school/spell family/family mask
        {
            // Check (if set) for school
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & GetSchoolMask(procSpell->School)) == 0)
                return false;

            // Check (if set) for spellFamilyName
            if (spellProcEvent->spellFamilyName && (spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
                return false;
        }
    }

    // Check for extra req (if none) and hit/crit
    if (procEvent_procEx == PROC_EX_NONE)
    {
        // Don't allow proc from periodic heal if no extra requirement is defined
        if (EventProcFlag & (PROC_FLAG_DEAL_HARMFUL_PERIODIC | PROC_FLAG_TAKE_HARMFUL_PERIODIC) && (procExtra & PROC_EX_PERIODIC_POSITIVE))
            return false;

        // No extra req, so can trigger for (damage/healing present) and hit/crit
        if (procExtra & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT))
            return true;
    }
    else // all spells hits here only if resist/reflect/immune/evade/periodic
    {
        // Exist req for PROC_EX_EX_TRIGGER_ALWAYS
        if (procEvent_procEx & PROC_EX_EX_TRIGGER_ALWAYS)
            return true;
        // Exist req for PROC_EX_NO_PERIODIC
        if ((procEvent_procEx & PROC_EX_NO_PERIODIC) &&
            ((procFlags & (PROC_FLAG_DEAL_HARMFUL_PERIODIC | PROC_FLAG_TAKE_HARMFUL_PERIODIC))
            || 
            (procSpell && procSpell->IsSpellAppliesPeriodicAura())))
            return false;
        // Check Extra Requirement like (hit/crit/miss/resist/parry/dodge/block/immune/reflect/absorb and other)
        if (procEvent_procEx & procExtra)
            return true;
    }
    return false;
}

void SpellMgr::LoadSpellGroups()
{
    mSpellSpellGroup.clear();                                  // need for reload case
    mSpellGroupSpell.clear();

    //                                                               0         1
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT group_id, spell_id FROM spell_group ORDER BY group_id, group_spell_id, spell_id"));
    if (!result)
    {
        return;
    }

    std::set<uint32> groups;

    do
    {
        Field *fields = result->Fetch();
        uint32 group_id = fields[0].GetUInt32();
        int32 spell_id = fields[1].GetInt32();

        groups.insert(std::set<uint32>::value_type(group_id));
        mSpellGroupSpell.insert(SpellGroupSpellMap::value_type((SpellGroup)group_id, spell_id));

    }
    while (result->NextRow());

    for (SpellGroupSpellMap::iterator itr = mSpellGroupSpell.begin(); itr != mSpellGroupSpell.end();)
    {
        if (itr->second < 0)
        {
            if (groups.find(abs(itr->second)) == groups.end())
            {
                sLog.outErrorDb("SpellGroup id %u listed in `spell_groups` does not exist", abs(itr->second));
                mSpellGroupSpell.erase(itr++);
            }
            else
                ++itr;
        }
        else
        {
            SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(itr->second);

            if (!spellInfo)
            {
                sLog.outErrorDb("Spell %u listed in `spell_group` does not exist", itr->second);
                mSpellGroupSpell.erase(itr++);
            }
            else
                ++itr;
        }
    }

    for (const auto group : groups)
    {
        std::set<uint32> spells;
        GetSetOfSpellsInSpellGroup(SpellGroup(group), spells);

        for (const auto spell : spells)
        {
            mSpellSpellGroup.insert(SpellSpellGroupMap::value_type(spell, SpellGroup(group)));
        }
    }
}

void SpellMgr::LoadSpellGroupStackRules()
{
    mSpellGroupStack.clear(); // need for reload case

    //                                                               0         1
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT group_id, stack_rule FROM spell_group_stack_rules"));
    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 group_id = fields[0].GetUInt32();
        uint8 stack_rule = fields[1].GetUInt32();
        if (stack_rule >= SPELL_GROUP_STACK_RULE_MAX)
        {
            sLog.outErrorDb("SpellGroupStackRule %u listed in `spell_group_stack_rules` does not exist", stack_rule);
            continue;
        }

        SpellGroupSpellMapBounds spellGroup = GetSpellGroupSpellMapBounds((SpellGroup)group_id);

        if (spellGroup.first == spellGroup.second)
        {
            sLog.outErrorDb("SpellGroup id %u listed in `spell_group_stack_rules` does not exist", group_id);
            continue;
        }

        mSpellGroupStack[(SpellGroup)group_id] = (SpellGroupStackRule)stack_rule;
    } while (result->NextRow());
}

bool SpellMgr::ListMorePowerfulSpells(uint32 spellId, std::vector<uint32>& list) const
{
    std::vector<uint32> spellGroupIds;
    std::vector<uint32>::iterator spellGroupIdsIt;
    // first = groupid, second = spellId
    for (const auto& itr : mSpellGroupSpell)
    {
        if (itr.second == spellId)
        {
            // Un sort peut etre dans plusieurs groupes. On s'interesse au groupe 'SPELL_GROUP_STACK_RULE_POWERFULL_CHAIN'
            SpellGroupStackMap::const_iterator found = mSpellGroupStack.find(itr.first);
            // Ce groupe n'a pas de regle ... Pas d'entree dans 'spell_group_stack_rule' ?
            if (found == mSpellGroupStack.end())
                continue;
            SpellGroupStackRule stackRule = found->second;
            if (stackRule == SPELL_GROUP_STACK_RULE_POWERFULL_CHAIN)
            {
                spellGroupIds.push_back(itr.first);
            }
        }
    }
    if (spellGroupIds.empty())
        return false;
    for (spellGroupIdsIt = spellGroupIds.begin(); spellGroupIdsIt != spellGroupIds.end(); ++spellGroupIdsIt)
    {
        bool spellPassed = false;
        for (const auto& itr : mSpellGroupSpell)
        {
            if (itr.first != *(spellGroupIdsIt))
                continue;
            if (!spellPassed)
            {
                if (itr.second == spellId)
                    spellPassed = true;
                continue;
            }
            list.push_back(itr.second);
        }
        MANGOS_ASSERT(spellPassed == true);
    }
    return !list.empty();
}

bool SpellMgr::ListLessPowerfulSpells(uint32 spellId, std::vector<uint32>& list) const
{
    std::vector<uint32> spellGroupIds;
    std::vector<uint32>::iterator spellGroupIdsIt;
    // first = groupid, second = spellId
    for (const auto& itr : mSpellGroupSpell)
    {
        if (itr.second == spellId)
        {
            // Un sort peut etre dans plusieurs groupes. On s'interesse au groupe 'SPELL_GROUP_STACK_RULE_POWERFULL_CHAIN'
            SpellGroupStackMap::const_iterator found = mSpellGroupStack.find(itr.first);
            // Ce groupe n'a pas de regle ... Pas d'entree dans 'spell_group_stack_rule' ?
            if (found == mSpellGroupStack.end())
                continue;
            SpellGroupStackRule stackRule = found->second;
            if (stackRule == SPELL_GROUP_STACK_RULE_POWERFULL_CHAIN)
            {
                spellGroupIds.push_back(itr.first);
            }
        }
    }
    if (spellGroupIds.empty())
        return false;
    for (spellGroupIdsIt = spellGroupIds.begin(); spellGroupIdsIt != spellGroupIds.end(); ++spellGroupIdsIt)
    {
        for (const auto& itr : mSpellGroupSpell)
        {
            if (itr.first != *(spellGroupIdsIt))
                continue;
            if (itr.second == spellId)
                break;
            list.push_back(itr.second);
        }
    }
    return !list.empty();
}

void SpellMgr::LoadSpellElixirs()
{
    mSpellElixirs.clear(); // need for reload case

    //                                                               0      1
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT entry, mask FROM spell_elixir"));
    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint8 mask = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(entry);

        if (!spellInfo)
        {
            sLog.outErrorDb("Spell %u listed in `spell_elixir` does not exist", entry);
            continue;
        }

        mSpellElixirs[entry] = mask;
    } while (result->NextRow());
}

struct DoSpellThreat
{
    DoSpellThreat(SpellThreatMap& _threatMap) : threatMap(_threatMap), count(0) {}
    void operator()(uint32 spell_id)
    {
        SpellThreatEntry const &ste = state->second;
        // add ranks only for not filled data (spells adding flat threat are usually different for ranks)
        SpellThreatMap::const_iterator spellItr = threatMap.find(spell_id);
        if (spellItr == threatMap.end())
            threatMap[spell_id] = ste;

        // just assert that entry is not redundant
        else
        {
            SpellThreatEntry const& r_ste = spellItr->second;
            if (ste.threat == r_ste.threat && ste.multiplier == r_ste.multiplier && ste.ap_bonus == r_ste.ap_bonus)
                sLog.outErrorDb("Spell %u listed in `spell_threat` as custom rank has same data as Rank 1, so redundant", spell_id);
        }
    }
    const char* TableName()
    {
        return "spell_threat";
    }
    bool IsValidCustomRank(SpellThreatEntry const &ste, uint32 entry, uint32 first_id)
    {
        if (!ste.threat)
        {
            sLog.outErrorDb("Spell %u listed in `spell_threat` is not first rank (%u) in chain and has no threat", entry, first_id);
            // prevent loading unexpected data
            return false;
        }
        return true;
    }
    void AddEntry(SpellThreatEntry const &ste, SpellEntry const *spell)
    {
        threatMap[spell->Id] = ste;

        // flat threat bonus and attack power bonus currently only work properly when all
        // effects have same targets, otherwise, we'd need to seperate it by effect index
        if (ste.threat || ste.ap_bonus != 0.f)
        {
            const uint32 *targetA = spell->EffectImplicitTargetA;
            if ((targetA[EFFECT_INDEX_1] && targetA[EFFECT_INDEX_1] != targetA[EFFECT_INDEX_0]) ||
                    (targetA[EFFECT_INDEX_2] && targetA[EFFECT_INDEX_2] != targetA[EFFECT_INDEX_0]))
                sLog.outErrorDb("Spell %u listed in `spell_threat` has effects with different targets, threat may be assigned incorrectly", spell->Id);
        }
        ++count;
    }
    bool HasEntry(uint32 spellId)
    {
        return threatMap.count(spellId) > 0;
    }
    bool SetStateToEntry(uint32 spellId)
    {
        return (state = threatMap.find(spellId)) != threatMap.end();
    }

    SpellThreatMap& threatMap;
    SpellThreatMap::const_iterator state;
    uint32 count;
};

void SpellMgr::LoadSpellThreats()
{
    mSpellThreatMap.clear(); // need for reload case

    //                                                               0      1       2           3
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT entry, Threat, multiplier, ap_bonus FROM spell_threat"));
    if (!result)
    {
        return;
    }

    SpellRankHelper<SpellThreatEntry, DoSpellThreat, SpellThreatMap> rankHelper(*this, mSpellThreatMap);

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        SpellThreatEntry ste;
        ste.threat = fields[1].GetUInt16();
        ste.multiplier = fields[2].GetFloat();
        ste.ap_bonus = fields[3].GetFloat();

        rankHelper.RecordRank(ste, entry);

    } while (result->NextRow());

    rankHelper.FillHigherRanks();
}

bool SpellMgr::IsRankSpellDueToSpell(SpellEntry const *spellInfo_1, uint32 spellId_2) const
{
    SpellEntry const *spellInfo_2 = sSpellMgr.GetSpellEntry(spellId_2);
    if (!spellInfo_1 || !spellInfo_2) return false;
    if (spellInfo_1->Id == spellId_2) return false;
    // Nostalrius : Check generique.
    if (spellInfo_1->SpellFamilyName == spellInfo_2->SpellFamilyName &&
        spellInfo_1->SpellFamilyFlags == spellInfo_2->SpellFamilyFlags &&
        spellInfo_1->SpellIconID == spellInfo_2->SpellIconID &&
        spellInfo_1->SpellVisual == spellInfo_2->SpellVisual &&
        spellInfo_1->SpellFamilyName != SPELLFAMILY_GENERIC &&
        spellInfo_1->Effect[0] == spellInfo_2->Effect[0] &&
        spellInfo_1->EffectApplyAuraName[0] == spellInfo_2->EffectApplyAuraName[0] &&
        spellInfo_1->SpellIconID > 1 &&
        (spellInfo_1->EffectApplyAuraName[0] != SPELL_AURA_ADD_FLAT_MODIFIER ||
         spellInfo_1->EffectMiscValue[0] == spellInfo_2->EffectMiscValue[0]))
    {
        // Same modifier but it affects different spells
        if (spellInfo_1->EffectApplyAuraName[0] == SPELL_AURA_ADD_FLAT_MODIFIER &&
            spellInfo_1->EffectItemType[0] != 0 && spellInfo_2->EffectItemType[0] != 0 &&
            !(spellInfo_1->EffectItemType[0] & spellInfo_2->EffectItemType[0]))
        {
            return GetFirstSpellInChain(spellInfo_1->Id) == GetFirstSpellInChain(spellId_2);
        }

        return true;
    }

    return GetFirstSpellInChain(spellInfo_1->Id) == GetFirstSpellInChain(spellId_2);
}

bool SpellMgr::IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2) const
{
    SpellGroup unused;
    if (CheckSpellGroupStackRules(spellId_1, spellId_2, unused) != SPELL_GROUP_STACK_RULE_DEFAULT)
    {
        DETAIL_LOG("[STACK][%u/%u] Pas de stack (DB/spell_group)", spellId_1, spellId_2);
        return false;
    }

    SpellEntry const *spellInfo_1 = sSpellMgr.GetSpellEntry(spellId_1);
    SpellEntry const *spellInfo_2 = sSpellMgr.GetSpellEntry(spellId_2);

    if (!spellInfo_1 || !spellInfo_2)
        return false;

    if (spellId_1 == spellId_2)
        return false;

    // Moonkin Aura and Power of the Guardian (Atiesh):
    if (spellInfo_1->Id == 24907 && spellInfo_2->Id == 28142) return false;
    if (spellInfo_1->Id == 24907 && spellInfo_2->Id == 28143) return false;
    if (spellInfo_1->Id == 24907 && spellInfo_2->Id == 28144) return false;
    if (spellInfo_1->Id == 24907 && spellInfo_2->Id == 28145) return false;

    // Resurrection sickness
    if ((spellInfo_1->Id == SPELL_ID_PASSIVE_RESURRECTION_SICKNESS) != (spellInfo_2->Id == SPELL_ID_PASSIVE_RESURRECTION_SICKNESS))
        return false;

    // Allow stack passive and not passive spells
    if ((spellInfo_1->Attributes & SPELL_ATTR_PASSIVE) != (spellInfo_2->Attributes & SPELL_ATTR_PASSIVE))
        return false;

    // Specific spell family spells
    switch (spellInfo_1->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            switch (spellInfo_2->SpellFamilyName)
            {
                case SPELLFAMILY_GENERIC:                   // same family case
                {
                    // Thunderfury
                    if ((spellInfo_1->Id == 21992 && spellInfo_2->Id == 27648) ||
                            (spellInfo_2->Id == 21992 && spellInfo_1->Id == 27648))
                        return false;

                    // Atiesh aura stacking with Moonkin Aura
                    if (spellInfo_1->SpellIconID == 46 && spellInfo_2->SpellIconID == 46)
                        return false;

                    // Soulstone Resurrection and Twisting Nether (resurrector)
                    if (spellInfo_1->SpellIconID == 92 && spellInfo_2->SpellIconID == 92 && (
                       (spellInfo_1->SpellVisual == 99 && spellInfo_2->SpellVisual == 0) ||
                       (spellInfo_2->SpellVisual == 99 && spellInfo_1->SpellVisual == 0)))
                        return false;

                    // Heart of the Wild and (Primal Instinct (Idol of Terror) triggering spell or Agility)
                    if (spellInfo_1->SpellIconID == 240 && spellInfo_2->SpellIconID == 240 && (
                       (spellInfo_1->SpellVisual == 0 && spellInfo_2->SpellVisual == 78) ||
                       (spellInfo_2->SpellVisual == 0 && spellInfo_1->SpellVisual == 78)))
                        return false;

                    // Personalized Weather (thunder effect should overwrite rainy aura)
                    if (spellInfo_1->SpellIconID == 2606 && spellInfo_2->SpellIconID == 2606)
                        return false;

                    // Brood Affliction: Bronze
                    if ((spellInfo_1->Id == 23170 && spellInfo_2->Id == 23171) ||
                        (spellInfo_2->Id == 23170 && spellInfo_1->Id == 23171))
                        return false;

                    // Regular and Night Elf Ghost
                    if ((spellInfo_1->Id == 8326 && spellInfo_2->Id == 20584) ||
                        (spellInfo_2->Id == 8326 && spellInfo_1->Id == 20584))
                        return false;

                    // Allow Lightning Speed to stack with Haste.
                    if (spellInfo_1->SpellIconID == 30 && spellInfo_2->SpellIconID == 30 && 
                        spellInfo_1->SpellVisual == 1508 && spellInfo_2->SpellVisual == 1508)
                        return false;

                    break;
                }
                case SPELLFAMILY_MAGE:
                    // Arcane Intellect and Insight
                    if (spellInfo_2->SpellIconID == 125 && spellInfo_1->Id == 18820)
                        return false;
                    break;
                case SPELLFAMILY_WARRIOR:
                {
                    // Scroll of Protection and Defensive Stance (multi-family check)
                    if (spellInfo_1->SpellIconID == 276 && spellInfo_1->SpellVisual == 196 && spellInfo_2->Id == 71)
                        return false;

                    // Improved Hamstring -> Hamstring (multi-family check)
                    if ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x2)) && spellInfo_1->Id == 23694)
                        return false;
                    break;
                }
                case SPELLFAMILY_DRUID:
                {
                    // Scroll of Stamina and Leader of the Pack (multi-family check)
                    if (spellInfo_1->SpellIconID == 312 && spellInfo_1->SpellVisual == 216 && spellInfo_2->Id == 24932)
                        return false;
                    break;
                }
                case SPELLFAMILY_ROGUE:
                {
                    // Garrote-Silence -> Garrote (multi-family check)
                    if (spellInfo_1->SpellIconID == 498 && spellInfo_1->SpellVisual == 0 && spellInfo_2->SpellIconID == 498)
                        return false;

                    break;
                }
                case SPELLFAMILY_HUNTER:
                {
                    // Concussive Shot and Imp. Concussive Shot (multi-family check)
                    if (spellInfo_1->Id == 19410 && spellInfo_2->Id == 5116)
                        return false;

                    // Improved Wing Clip -> Wing Clip (multi-family check)
                    if ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x40)) && spellInfo_1->Id == 19229)
                        return false;
                    break;
                }
                case SPELLFAMILY_PALADIN:
                {
                    // Unstable Currents and other -> *Sanctity Aura (multi-family check)
                    if (spellInfo_2->SpellIconID == 502 && spellInfo_1->SpellIconID == 502 && spellInfo_1->SpellVisual == 969)
                        return false;
                    break;
                }
            }
            break;
        case SPELLFAMILY_MAGE:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_MAGE)
            {
                // Explosion pyrotechnique / boule de feu
                if ((spellInfo_1->SpellIconID == 184 && spellInfo_2->SpellIconID == 185) ||
                        (spellInfo_2->SpellIconID == 184 && spellInfo_1->SpellIconID == 185))
                    return false;

                // Blizzard & Chilled (and some other stacked with blizzard spells
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x80)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x100000))) ||
                        ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x80)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x100000))))
                    return false;

                // Blink & Improved Blink
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x0000000000010000)) && (spellInfo_2->SpellVisual == 72 && spellInfo_2->SpellIconID == 1499)) ||
                        ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x0000000000010000)) && (spellInfo_1->SpellVisual == 72 && spellInfo_1->SpellIconID == 1499)))
                    return false;

                // Fireball & Pyroblast (Dots)
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x1)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x400000))) ||
                        ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x1)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x400000))))
                    return false;

                // Arcane Missiles
                if (spellInfo_1->IsFitToFamilyMask(1 << CF_MAGE_ARCANE_MISSILES_CHANNEL) && spellInfo_2->IsFitToFamilyMask(1 << CF_MAGE_ARCANE_MISSILES_CHANNEL))
                    return false;
            }
            // Detect Invisibility and Mana Shield (multi-family check)
            if (spellInfo_2->Id == 132 && spellInfo_1->SpellIconID == 209 && spellInfo_1->SpellVisual == 968)
                return false;

            // Combustion and Fire Protection Aura (multi-family check)
            if (spellInfo_1->Id == 11129 && spellInfo_2->SpellIconID == 33 && spellInfo_2->SpellVisual == 321)
                return false;

            // Arcane Intellect and Insight
            if (spellInfo_1->SpellIconID == 125 && spellInfo_2->Id == 18820)
                return false;

            break;
        case SPELLFAMILY_WARLOCK:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_WARLOCK)
            {
                // Siphon Life and Drain Life
                if ((spellInfo_1->SpellIconID == 152 && spellInfo_2->SpellIconID == 546) ||
                        (spellInfo_2->SpellIconID == 152 && spellInfo_1->SpellIconID == 546))
                    return false;

                // Corruption and (Curse of Agony or Curse of Doom)
                if ((spellInfo_1->SpellIconID == 313 && (spellInfo_2->SpellIconID == 544  || spellInfo_2->SpellIconID == 91)) ||
                        (spellInfo_2->SpellIconID == 313 && (spellInfo_1->SpellIconID == 544  || spellInfo_1->SpellIconID == 91)))
                    return false;
            }
            // Detect Invisibility and Mana Shield (multi-family check)
            if (spellInfo_1->Id == 132 && spellInfo_2->SpellIconID == 209 && spellInfo_2->SpellVisual == 968)
                return false;
            break;
        case SPELLFAMILY_WARRIOR:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_WARRIOR)
            {
                // Rend and Deep Wound
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x20)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x1000000000))) ||
                        ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x20)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x1000000000))))
                    return false;

                // Battle Shout and Rampage
                if ((spellInfo_1->SpellIconID == 456 && spellInfo_2->SpellIconID == 2006) ||
                        (spellInfo_2->SpellIconID == 456 && spellInfo_1->SpellIconID == 2006))
                    return false;

                // Cri demoralisant et cri de guerre
                if ((spellInfo_2->SpellVisual == 246 && spellInfo_1->SpellVisual == 210) || (spellInfo_1->SpellVisual == 246 && spellInfo_2->SpellVisual == 210))
                    return false;
            }

            // Hamstring -> Improved Hamstring (multi-family check)
            if ((spellInfo_1->SpellFamilyFlags & UI64LIT(0x2)) && spellInfo_2->Id == 23694)
                return false;

            // Defensive Stance and Scroll of Protection (multi-family check)
            if (spellInfo_1->Id == 71 && spellInfo_2->SpellIconID == 276 && spellInfo_2->SpellVisual == 196)
                return false;

            // Bloodlust and Bloodthirst (multi-family check)
            if (spellInfo_2->Id == 2825 && spellInfo_1->SpellIconID == 38 && spellInfo_1->SpellVisual == 0)
                return false;

            break;
        case SPELLFAMILY_PRIEST:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_PRIEST)
            {
                //Greater Heal Renew (8/8 transcendence) (and Renew.)
                if (spellId_1==22009 || spellId_2==22009)
                    return false;

                // Shadow Vulnerability / Devouring Plague
                if (spellInfo_1->SpellIconID == spellInfo_2->SpellIconID /* == 9 */ &&
                        ((spellInfo_1->Id == 15258 &&  spellInfo_2->SpellFamilyFlags == 0x2000000) ||
                         (spellInfo_2->Id == 15258 &&  spellInfo_1->SpellFamilyFlags == 0x2000000)))
                    return false;

                //Devouring Plague and Shadow Vulnerability
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x2000000)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x800000000))) ||
                        ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x2000000)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x800000000))))
                    return false;

                //StarShards and Shadow Word: Pain
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x200000)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x8000))) ||
                        ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x200000)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x8000))))
                    return false;
            }
            break;
        case SPELLFAMILY_DRUID:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_DRUID)
            {
                // Cat Form and Feline Swiftness Passive* (1.x specific conflict)
                if (spellInfo_1->SpellIconID == 493 && spellInfo_2->SpellIconID == 493)
                    return false;

                //Omen of Clarity and Blood Frenzy
                if (((spellInfo_1->SpellFamilyFlags == UI64LIT(0x0) && spellInfo_1->SpellIconID == 108) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x20000000000000))) ||
                        ((spellInfo_2->SpellFamilyFlags == UI64LIT(0x0) && spellInfo_2->SpellIconID == 108) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x20000000000000))))
                    return false;
            }

            // Leader of the Pack and Scroll of Stamina (multi-family check)
            if (spellInfo_1->Id == 24932 && spellInfo_2->SpellIconID == 312 && spellInfo_2->SpellVisual == 216)
                return false;

            break;
        case SPELLFAMILY_ROGUE:
            // Garrote -> Garrote-Silence (multi-family check)
            if (spellInfo_1->SpellIconID == 498 && spellInfo_2->SpellIconID == 498 && spellInfo_2->SpellVisual == 0)
                return false;
            break;
        case SPELLFAMILY_HUNTER:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_HUNTER)
            {
                // Rapid Fire & Quick Shots
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x20)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x20000000000))) ||
                        ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x20)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x20000000000))))
                    return false;

                // Serpent Sting & (Immolation/Explosive Trap Effect)
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x4)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x00000004000))) ||
                        ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x4)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x00000004000))))
                    return false;

                // Bestial Wrath
                if (spellInfo_1->SpellIconID == 1680 && spellInfo_2->SpellIconID == 1680)
                    return false;
            }

            // Wing Clip -> Improved Wing Clip (multi-family check)
            if ((spellInfo_1->SpellFamilyFlags & UI64LIT(0x40)) && spellInfo_2->Id == 19229)
                return false;

            // Concussive Shot and Imp. Concussive Shot (multi-family check)
            if (spellInfo_2->Id == 19410 && spellInfo_1->Id == 5116)
                return false;
            break;
        case SPELLFAMILY_PALADIN:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_PALADIN)
            {
                // Paladin Seals
                if (spellInfo_1->IsSealSpell() && spellInfo_2->IsSealSpell())
                    return true;
                // Concentration Aura and Improved Concentration Aura and Aura Mastery
                if ((spellInfo_1->SpellIconID == 1487) && (spellInfo_2->SpellIconID == 1487))
                    return false;
            }

            // Combustion and Fire Protection Aura (multi-family check)
            if (spellInfo_2->Id == 11129 && spellInfo_1->SpellIconID == 33 && spellInfo_1->SpellVisual == 321)
                return false;

            // *Sanctity Aura -> Unstable Currents and other (multi-family check)
            if (spellInfo_1->SpellIconID == 502 && spellInfo_2->SpellFamilyName == SPELLFAMILY_GENERIC && spellInfo_2->SpellIconID == 502 && spellInfo_2->SpellVisual == 969)
                return false;

            // *Seal of Command and Band of Eternal Champion (multi-family check)
            if (spellInfo_1->SpellIconID == 561 && spellInfo_1->SpellVisual == 7992 && spellId_2 == 35081)
                return false;
            break;
        case SPELLFAMILY_SHAMAN:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_SHAMAN)
            {
                // Windfury weapon
                if (spellInfo_1->SpellIconID == 220 && spellInfo_2->SpellIconID == 220 &&
                        !spellInfo_1->IsFitToFamilyMask(spellInfo_2->SpellFamilyFlags))
                    return false;
            }
            // Bloodlust and Bloodthirst (multi-family check)
            if (spellInfo_1->Id == 2825 && spellInfo_2->SpellIconID == 38 && spellInfo_2->SpellVisual == 0)
                return false;
            break;
        case SPELLFAMILY_POTION:

            // Dreamshard Elixir and Greater Arcane Elixir
            if (spellInfo_1->Id == 17539 && spellInfo_2->Id == 45427)
                return false;
            if (spellInfo_2->Id == 17539 && spellInfo_1->Id == 45427)
                return false;

            // Greater Nature Protection Potion and Elixir of Greater Nature Power
            if (spellInfo_1->Id == 17546 && spellInfo_2->Id == 45988)
                return false;
            if (spellInfo_2->Id == 17546 && spellInfo_1->Id == 45988)
                return false;

            // Nature Protection Potion and Elixir of Greater Nature Power
            if (spellInfo_1->Id == 7254 && spellInfo_2->Id == 45988)
                return false;
            if (spellInfo_2->Id == 7254 && spellInfo_1->Id == 45988)
                return false;

            break;
        default:
            break;
    }

    // more generic checks
    if (spellInfo_1->SpellIconID == spellInfo_2->SpellIconID &&
            spellInfo_1->SpellIconID != 0 && spellInfo_1->SpellIconID != 1 && // SpellIconID 1 se stack avec tout ...
            spellInfo_1->SpellFamilyName == spellInfo_2->SpellFamilyName &&
            spellInfo_1->SpellVisual == spellInfo_2->SpellVisual &&
            // Generic peut etre, mais un peu debile .. :O
            // Exception :
            spellInfo_1->SpellIconID != 516 && // Sprint ameliore
            spellInfo_1->IsPositiveSpell() == spellInfo_2->IsPositiveSpell() // Jugement n'ecrase pas le sceau en dudu pala-pala
       )
    {
        bool isModifier = false;
        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                    spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER  ||
                    spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                    spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER)
                isModifier = true;
        }

        if (!isModifier)
        {
            DETAIL_LOG("[STACK][%u/%u] Meme icone et non modifier.", spellId_1, spellId_2);
            return true;
        }
    }

    if (IsRankSpellDueToSpell(spellInfo_1, spellId_2))
    {
        DETAIL_LOG("[STACK][%u/%u] IsRankSpellDueToSpell", spellId_1, spellId_2);
        return true;
    }

    if (spellInfo_1->SpellFamilyName == 0 || spellInfo_2->SpellFamilyName == 0)
        return false;

    if (spellInfo_1->SpellFamilyName != spellInfo_2->SpellFamilyName)
        return false;

    // Why would a buff and a debuff ever be exclusive with each other?
    // Fixes Corrupted Mind getting removed by Lightning Shield.
    if (spellInfo_1->IsPositiveSpell() != spellInfo_2->IsPositiveSpell())
        return false;

    // Nostalrius: potions fonctionnent autrement.
    if (spellInfo_1->SpellFamilyName == SPELLFAMILY_POTION)
        return false;

    bool dummy_only = true;
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (spellInfo_1->Effect[i] != spellInfo_2->Effect[i] ||
                spellInfo_1->EffectItemType[i] != spellInfo_2->EffectItemType[i] ||
                spellInfo_1->EffectMiscValue[i] != spellInfo_2->EffectMiscValue[i] ||
                spellInfo_1->EffectApplyAuraName[i] != spellInfo_2->EffectApplyAuraName[i])
            return false;

        // ignore dummy only spells
        if (spellInfo_1->Effect[i] && spellInfo_1->Effect[i] != SPELL_EFFECT_DUMMY && spellInfo_1->EffectApplyAuraName[i] != SPELL_AURA_DUMMY)
            dummy_only = false;
    }
    if (dummy_only)
        return false;

    DETAIL_LOG("[STACK][%u/%u] not dummy only.", spellId_1, spellId_2);
    return true;
}

bool SpellMgr::IsProfessionOrRidingSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellMgr.GetSpellEntry(spellId);
    if (!spellInfo)
        return false;

    if (spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsProfessionOrRidingSkill(skill);
}

bool SpellMgr::IsProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellMgr.GetSpellEntry(spellId);
    if (!spellInfo)
        return false;

    if (spellInfo->Effect[EFFECT_INDEX_1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[EFFECT_INDEX_1];

    return IsProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellMgr.GetSpellEntry(spellId);
    if (!spellInfo)
        return false;

    if (spellInfo->Effect[EFFECT_INDEX_1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[EFFECT_INDEX_1];

    return IsPrimaryProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionFirstRankSpell(uint32 spellId) const
{
    return IsPrimaryProfessionSpell(spellId) && GetSpellRank(spellId) == 1;
}

bool SpellMgr::IsSkillBonusSpell(uint32 spellId) const
{
    SkillLineAbilityMapBounds bounds = GetSkillLineAbilityMapBoundsBySpellId(spellId);

    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        SkillLineAbilityEntry const *pAbility = _spell_idx->second;
        if (!pAbility || pAbility->learnOnGetSkill != ABILITY_LEARNED_ON_GET_PROFESSION_SKILL)
            continue;

        if (pAbility->req_skill_value > 0)
            return true;
    }

    return false;
}

SpellEntry const* SpellMgr::SelectAuraRankForLevel(SpellEntry const* spellInfo, uint32 level) const
{
    // fast case
    if (level + 10 >= spellInfo->spellLevel)
        return spellInfo;

    // ignore selection for passive spells
    if (spellInfo->IsPassiveSpell())
        return spellInfo;

    bool needRankSelection = false;
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        // for simple aura in check apply to any non caster based targets, in rank search mode to any explicit targets
        if (((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA && (IsExplicitPositiveTarget(spellInfo->EffectImplicitTargetA[i]) || IsAreaEffectPossitiveTarget(SpellTarget(spellInfo->EffectImplicitTargetA[i])))) ||
            spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY) && spellInfo->IsPositiveEffect(SpellEffectIndex(i)))
        {
            needRankSelection = true;
            break;
        }
    }

    // not required (rank check more slow so check it here)
    if (!needRankSelection || GetSpellRank(spellInfo->Id) == 0)
        return spellInfo;

    for (uint32 nextSpellId = spellInfo->Id; nextSpellId != 0; nextSpellId = GetPrevSpellInChain(nextSpellId))
    {
        SpellEntry const *nextSpellInfo = sSpellMgr.GetSpellEntry(nextSpellId);
        if (!nextSpellInfo)
            break;

        // if found appropriate level
        if (level + 10 >= nextSpellInfo->spellLevel)
            return nextSpellInfo;

        // one rank less then
    }

    // not found
    return nullptr;
}

typedef std::unordered_map<uint32, uint32> AbilitySpellPrevMap;

static void LoadSpellChains_AbilityHelper(SpellChainMap& chainMap, AbilitySpellPrevMap const& prevRanks, uint32 spell_id, uint32 prev_id, uint32 deep = 30)
{
    // spell already listed in chains store
    SpellChainMap::const_iterator chain_itr = chainMap.find(spell_id);
    if (chain_itr != chainMap.end())
    {
        MANGOS_ASSERT(chain_itr->second.prev == prev_id && "LoadSpellChains_AbilityHelper: Conflicting data in talents or spell abilities dbc");
        return;
    }

    // prev rank listed in main chain table (can fill correct data directly)
    SpellChainMap::const_iterator prev_chain_itr = chainMap.find(prev_id);
    if (prev_chain_itr != chainMap.end())
    {
        SpellChainNode node;
        node.prev  = prev_id;
        node.first = prev_chain_itr->second.first;
        node.rank  = prev_chain_itr->second.rank + 1;
        node.req   = 0;
        chainMap[spell_id] = node;
        return;
    }

    // prev spell not listed in prev ranks store, so it first rank
    AbilitySpellPrevMap::const_iterator prev_itr = prevRanks.find(prev_id);
    if (prev_itr == prevRanks.end())
    {
        SpellChainNode prev_node;
        prev_node.prev  = 0;
        prev_node.first = prev_id;
        prev_node.rank  = 1;
        prev_node.req   = 0;
        chainMap[prev_id] = prev_node;

        SpellChainNode node;
        node.prev  = prev_id;
        node.first = prev_id;
        node.rank  = 2;
        node.req   = 0;
        chainMap[spell_id] = node;
        return;
    }

    if (deep == 0)
    {
        MANGOS_ASSERT(false && "LoadSpellChains_AbilityHelper: Infinity cycle in spell ability data");
        return;
    }

    // prev rank listed, so process it first
    LoadSpellChains_AbilityHelper(chainMap, prevRanks, prev_id, prev_itr->second, deep - 1);

    // prev rank must be listed now
    prev_chain_itr = chainMap.find(prev_id);
    if (prev_chain_itr == chainMap.end())
        return;

    SpellChainNode node;
    node.prev  = prev_id;
    node.first = prev_chain_itr->second.first;
    node.rank  = prev_chain_itr->second.rank + 1;
    node.req   = 0;
    chainMap[spell_id] = node;
}

void SpellMgr::LoadSpellChains()
{
    mSpellChains.clear();                                   // need for reload case
    mSpellChainsNext.clear();                               // need for reload case

    // load known data for talents
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        // not add ranks for 1 ranks talents (if exist non ranks spells then it will included in table data)
        if (!talentInfo->RankID[1])
            continue;

        for (int j = 0; j < MAX_TALENT_RANK; j++)
        {
            uint32 spell_id = talentInfo->RankID[j];
            if (!spell_id)
                continue;

            if (!sSpellMgr.GetSpellEntry(spell_id))
            {
                //sLog.outErrorDb("Talent %u not exist as spell",spell_id);
                continue;
            }

            SpellChainNode node;
            node.prev  = (j > 0) ? talentInfo->RankID[j - 1] : 0;
            node.first = talentInfo->RankID[0];
            node.rank  = j + 1;
            node.req   = 0;

            mSpellChains[spell_id] = node;
        }
    }

    // load known data from spell abilities
    {
        // we can calculate ranks only after full data generation
        AbilitySpellPrevMap prevRanks;
        for (const auto& ab_itr : mSkillLineAbilityMapBySpellId)
        {
            uint32 spell_id = ab_itr.first;

            // some forward spells not exist and can be ignored (some outdated data)
            SpellEntry const* spell_entry = sSpellMgr.GetSpellEntry(spell_id);
            if (!spell_entry)                                   // no cases
                continue;

            // ignore spell without forwards (non ranked or missing info in skill abilities)
            uint32 forward_id = ab_itr.second->forward_spellid;

            // by some strange reason < 3.x clients not have forward spell for 2366
            if (spell_id == 2366)                           // Herb Gathering, Apprentice
                forward_id = 2368;

            // Seal of Righteousness (20154) make double in spellbook
            if (spell_id == 20154)
                continue;

            if (!forward_id)
                continue;

            // some forward spells not exist and can be ignored (some outdated data)
            SpellEntry const* forward_entry = sSpellMgr.GetSpellEntry(forward_id);
            if (!forward_entry)
                continue;

            // some forward spells still exist but excluded from real use as ranks and not listed in skill abilities now
            SkillLineAbilityMapBounds bounds = mSkillLineAbilityMapBySpellId.equal_range(forward_id);
            if (bounds.first == bounds.second)
                continue;

            // spell already listed in chains store
            SpellChainMap::const_iterator chain_itr = mSpellChains.find(forward_id);
            if (chain_itr != mSpellChains.end())
            {
                MANGOS_ASSERT(chain_itr->second.prev == spell_id && "Conflicting data in talents or spell abilities dbc");
                continue;
            }

            // spell already listed in prev ranks store
            AbilitySpellPrevMap::const_iterator prev_itr = prevRanks.find(forward_id);
            if (prev_itr != prevRanks.end())
            {
                MANGOS_ASSERT(prev_itr->second == spell_id && "Conflicting data in talents or spell abilities dbc");
                continue;
            }

            // prev rank listed in main chain table (can fill correct data directly)
            SpellChainMap::const_iterator prev_chain_itr = mSpellChains.find(spell_id);
            if (prev_chain_itr != mSpellChains.end())
            {
                SpellChainNode node;
                node.prev  = spell_id;
                node.first = prev_chain_itr->second.first;
                node.rank  = prev_chain_itr->second.rank + 1;
                node.req   = 0;

                mSpellChains[forward_id] = node;
                continue;
            }

            // need temporary store for later rank calculation
            prevRanks[forward_id] = spell_id;
        }

        while (!prevRanks.empty())
        {
            uint32 spell_id = prevRanks.begin()->first;
            uint32 prev_id  = prevRanks.begin()->second;
            prevRanks.erase(prevRanks.begin());

            LoadSpellChains_AbilityHelper(mSpellChains, prevRanks, spell_id, prev_id);
        }
    }

    // load custom case
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT spell_id, prev_spell, first_spell, `rank`, req_spell FROM spell_chain"));
    if (!result)
    {
        sLog.outErrorDb("`spell_chains` table is empty!");
        return;
    }

    uint32 dbc_count = mSpellChains.size();
    uint32 req_count = 0;

    do
    {
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();

        SpellChainNode node;
        node.prev  = fields[1].GetUInt32();
        node.first = fields[2].GetUInt32();
        node.rank  = fields[3].GetUInt8();
        node.req   = fields[4].GetUInt32();

        if (!sSpellMgr.GetSpellEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` does not exist", spell_id);
            continue;
        }

        SpellChainMap::iterator chain_itr = mSpellChains.find(spell_id);
        if (chain_itr != mSpellChains.end())
        {
            if (chain_itr->second.rank != node.rank)
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` expected rank %u by DBC data.",
                                spell_id, node.prev, node.first, node.rank, node.req, chain_itr->second.rank);
                continue;
            }

            if (chain_itr->second.prev != node.prev)
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` expected prev %u by DBC data.",
                                spell_id, node.prev, node.first, node.rank, node.req, chain_itr->second.prev);
                continue;
            }

            if (chain_itr->second.first != node.first)
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` expected first %u by DBC data.",
                                spell_id, node.prev, node.first, node.rank, node.req, chain_itr->second.first);
                continue;
            }

            // update req field by table data
            if (node.req)
            {
                chain_itr->second.req = node.req;
                ++req_count;
                continue;
            }

            // in other case redundant
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) already added (talent or spell ability with forward) and non need in `spell_chain`",
                            spell_id, node.prev, node.first, node.rank, node.req);
            continue;
        }

        if (node.prev != 0 && !sSpellMgr.GetSpellEntry(node.prev))
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has nonexistent previous rank spell.",
                            spell_id, node.prev, node.first, node.rank, node.req);
            continue;
        }

        if (!sSpellMgr.GetSpellEntry(node.first))
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not existing first rank spell.",
                            spell_id, node.prev, node.first, node.rank, node.req);
            continue;
        }

        // check basic spell chain data integrity (note: rank can be equal 0 or 1 for first/single spell)
        if ((spell_id == node.first) != (node.rank <= 1) ||
                (spell_id == node.first) != (node.prev == 0) ||
                (node.rank <= 1) != (node.prev == 0))
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not compatible chain data.",
                            spell_id, node.prev, node.first, node.rank, node.req);
            continue;
        }

        if (node.req != 0 && !sSpellMgr.GetSpellEntry(node.req))
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not existing required spell.",
                            spell_id, node.prev, node.first, node.rank, node.req);
            continue;
        }

        // talents not required data in spell chain for work, but must be checked if present for integrity
        if (TalentSpellPos const* pos = GetTalentSpellPos(spell_id))
        {
            if (node.rank != pos->rank + 1)
            {
                sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong rank.",
                                spell_id, node.prev, node.first, node.rank, node.req);
                continue;
            }

            if (TalentEntry const* talentEntry = sTalentStore.LookupEntry(pos->talent_id))
            {
                if (node.first != talentEntry->RankID[0])
                {
                    sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong first rank spell.",
                                    spell_id, node.prev, node.first, node.rank, node.req);
                    continue;
                }

                if (node.rank > 1 && node.prev != talentEntry->RankID[node.rank - 1 - 1])
                {
                    sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong prev rank spell.",
                                    spell_id, node.prev, node.first, node.rank, node.req);
                    continue;
                }

                if (node.req != talentEntry->DependsOnSpell)
                {
                    sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong required spell.",
                                    spell_id, node.prev, node.first, node.rank, node.req);
                    continue;
                }
            }
        }

        mSpellChains[spell_id] = node;
    } while (result->NextRow());

    // additional integrity checks
    for (const auto& itr : mSpellChains)
    {
        if (itr.second.prev)
        {
            SpellChainMap::const_iterator i_prev = mSpellChains.find(itr.second.prev);
            if (i_prev == mSpellChains.end())
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not found previous rank spell in table.",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req);
            }
            else if (i_prev->second.first != itr.second.first)
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has different first spell in chain compared to previous rank spell (prev: %u, first: %u, rank: %d, req: %u).",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req,
                                i_prev->second.prev, i_prev->second.first, i_prev->second.rank, i_prev->second.req);
            }
            else if (i_prev->second.rank + 1 != itr.second.rank)
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has different rank compared to previous rank spell (prev: %u, first: %u, rank: %d, req: %u).",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req,
                                i_prev->second.prev, i_prev->second.first, i_prev->second.rank, i_prev->second.req);
            }
        }

        if (itr.second.req)
        {
            SpellChainMap::const_iterator i_req = mSpellChains.find(itr.second.req);
            if (i_req == mSpellChains.end())
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not found required rank spell in table.",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req);
            }
            else if (i_req->second.first == itr.second.first)
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has required rank spell from same spell chain (prev: %u, first: %u, rank: %d, req: %u).",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req,
                                i_req->second.prev, i_req->second.first, i_req->second.rank, i_req->second.req);
            }
            else if (i_req->second.req)
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has required rank spell with required spell (prev: %u, first: %u, rank: %d, req: %u).",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req,
                                i_req->second.prev, i_req->second.first, i_req->second.rank, i_req->second.req);
            }
        }
    }

    // fill next rank cache
    for (const auto& itr : mSpellChains)
    {
        uint32 spell_id = itr.first;
        SpellChainNode const& node = itr.second;

        if (node.prev)
            mSpellChainsNext.insert(SpellChainMapNext::value_type(node.prev, spell_id));

        if (node.req)
            mSpellChainsNext.insert(SpellChainMapNext::value_type(node.req, spell_id));
    }

    // check single rank redundant cases (single rank talents not added by default so this can be only custom cases)
    for (const auto& itr : mSpellChains)
    {
        // skip non-first ranks, and spells with additional reqs
        if (itr.second.rank > 1 || itr.second.req)
            continue;

        if (mSpellChainsNext.find(itr.first) == mSpellChainsNext.end())
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has single rank data, so redundant.",
                            itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req);
        }
    }
}

void SpellMgr::LoadSpellLearnSkills()
{
    mSpellLearnSkills.clear();                              // need for reload case

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    for (uint32 spell = 0; spell < sSpellMgr.GetMaxSpellId(); ++spell)
    {
        SpellEntry const* entry = sSpellMgr.GetSpellEntry(spell);

        if (!entry)
            continue;

        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (entry->Effect[i] == SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill    = entry->EffectMiscValue[i];
                dbc_node.step     = entry->CalculateSimpleValue(SpellEffectIndex(i));
                if (dbc_node.skill != SKILL_RIDING)
                    dbc_node.value = 1;
                else
                    dbc_node.value = dbc_node.step * 75;
                dbc_node.maxvalue = dbc_node.step * 75;

                mSpellLearnSkills[spell] = dbc_node;
                break;
            }
        }
    }
}

void SpellMgr::LoadSpellEnchantCharges()
{
    mSpellEnchantChargesMap.clear(); // need for reload case

    //                                                              0      1
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT entry, charges FROM spell_enchant_charges"));
    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 charges = fields[1].GetUInt32();

        SpellEntry const* pSpellInfo = GetSpellEntry(entry);

        if (!pSpellInfo)
        {
            if (!IsExistingSpellId(entry))
                sLog.outErrorDb("Spell %u listed in `spell_enchant_charges` does not exist", entry);
            continue;
        }

        mSpellEnchantChargesMap[entry] = charges;

    } while (result->NextRow());
}

void SpellMgr::LoadSpellLearnSpells()
{
    mSpellLearnSpells.clear(); // need for reload case

    //                                                               0      1        2
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT entry, SpellID, Active FROM spell_learn_spell"));
    if (!result)
    {
        sLog.outErrorDb("`spell_learn_spell` table is empty!");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 spell_id    = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell      = fields[1].GetUInt32();
        node.active     = fields[2].GetBool();
        node.autoLearned = false;

        if (!sSpellMgr.GetSpellEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` does not exist", spell_id);
            continue;
        }

        if (!sSpellMgr.GetSpellEntry(node.spell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` learning nonexistent spell %u", spell_id, node.spell);
            continue;
        }

        if (GetTalentSpellCost(node.spell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` attempt learning talent spell %u, skipped", spell_id, node.spell);
            continue;
        }

        mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id, node));
    } while (result->NextRow());

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    for (uint32 spell = 0; spell < sSpellMgr.GetMaxSpellId(); ++spell)
    {
        SpellEntry const* entry = sSpellMgr.GetSpellEntry(spell);

        if (!entry)
            continue;

        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (entry->Effect[i] == SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = entry->EffectTriggerSpell[i];
                dbc_node.active      = true;                // all dbc based learned spells is active (show in spell book or hide by client itself)

                // ignore learning nonexistent spells (broken/outdated/or generic learning spell 483
                if (!sSpellMgr.GetSpellEntry(dbc_node.spell))
                    continue;

                // talent or passive spells or skill-step spells auto-casted and not need dependent learning,
                // pet teaching spells don't must be dependent learning (casted)
                // other required explicit dependent learning
                dbc_node.autoLearned = entry->EffectImplicitTargetA[i] == TARGET_UNIT_CASTER_PET || GetTalentSpellCost(spell) > 0 || entry->IsPassiveSpell() || entry->HasEffect(SPELL_EFFECT_SKILL_STEP);

                SpellLearnSpellMapBounds db_node_bounds = GetSpellLearnSpellMapBounds(spell);

                bool found = false;
                for (SpellLearnSpellMap::const_iterator itr = db_node_bounds.first; itr != db_node_bounds.second; ++itr)
                {
                    if (itr->second.spell == dbc_node.spell)
                    {
                        sLog.outErrorDb("Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.",
                                        spell, dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if (!found)                                 // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell, dbc_node));
                }
            }
        }
    }
}

void SpellMgr::LoadSpellScriptTarget()
{
    std::set<uint32> conditions;

    // Load existing condition Ids so we can check for wrong condition Id later.
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `condition_entry` FROM `conditions`"));

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 conditionId = fields[0].GetUInt32();
            conditions.insert(conditionId);
        }
        while (result->NextRow());
    }

    mSpellScriptTarget.clear(); // need for reload case

    result.reset(WorldDatabase.Query("SELECT `entry`, `type`, `targetEntry`, `conditionId`, `inverseEffectMask` FROM `spell_script_target`"));

    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 spellId     = fields[0].GetUInt32();
        uint32 type        = fields[1].GetUInt32();
        uint32 targetEntry = fields[2].GetUInt32();
        uint32 conditionId = fields[3].GetUInt32();
        uint32 effectMask  = fields[4].GetUInt32();

        SpellEntry const* spellProto = sSpellMgr.GetSpellEntry(spellId);

        if (!spellProto)
        {
            if (!sSpellMgr.IsExistingSpellId(spellId))
                sLog.outErrorDb("Table `spell_script_target`: spellId %u listed for TargetEntry %u does not exist.", spellId, targetEntry);
            continue;
        }

        if (conditionId)
        {
            if (conditions.find(conditionId) == conditions.end())
            {
                sLog.outErrorDb("Table `spell_script_target`: conditionId %u listed for spell %u does not exist.", conditionId, spellId);
                continue;
            }
        }

        bool targetfound = false;
        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spellProto->EffectImplicitTargetA[i] == TARGET_UNIT_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetB[i] == TARGET_UNIT_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetA[i] == TARGET_LOCATION_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetB[i] == TARGET_LOCATION_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetA[i] == TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetB[i] == TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetA[i] == TARGET_ENUM_UNITS_SCRIPT_AOE_AT_SRC_LOC ||
                spellProto->EffectImplicitTargetB[i] == TARGET_ENUM_UNITS_SCRIPT_AOE_AT_SRC_LOC ||
                spellProto->EffectImplicitTargetA[i] == TARGET_ENUM_UNITS_SCRIPT_AOE_AT_DEST_LOC ||
                spellProto->EffectImplicitTargetB[i] == TARGET_ENUM_UNITS_SCRIPT_AOE_AT_DEST_LOC ||
                spellProto->EffectImplicitTargetA[i] == TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_SRC_LOC ||
                spellProto->EffectImplicitTargetB[i] == TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_SRC_LOC ||
                spellProto->EffectImplicitTargetA[i] == TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_DEST_LOC ||
                spellProto->EffectImplicitTargetB[i] == TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_DEST_LOC ||
                spellProto->EffectImplicitTargetA[i] == TARGET_ENUM_UNITS_SCRIPT_IN_CONE_60 ||
                spellProto->EffectImplicitTargetB[i] == TARGET_ENUM_UNITS_SCRIPT_IN_CONE_60)
            {
                targetfound = true;
                break;
            }
        }
        if (!targetfound)
        {
            sLog.outErrorDb("Table `spell_script_target`: spellId %u listed for TargetEntry %u does not have any implicit target TARGET_UNIT_SCRIPT_NEAR_CASTER(38) or TARGET_LOCATION_SCRIPT_NEAR_CASTER (46) or TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER (40).", spellId, targetEntry);
            continue;
        }

        if (type >= MAX_SPELL_TARGET_TYPE)
        {
            sLog.outErrorDb("Table `spell_script_target`: target type %u for TargetEntry %u is incorrect.", type, targetEntry);
            continue;
        }

        // Checks by target type
        switch (type)
        {
            case SPELL_TARGET_TYPE_GAMEOBJECT:
            {
                if (!targetEntry)
                    break;

                if (!sObjectMgr.GetGameObjectInfo(targetEntry))
                {
                    if (!sObjectMgr.IsExistingGameObjectId(targetEntry))
                        sLog.outErrorDb("Table `spell_script_target`: gameobject template entry %u does not exist.", targetEntry);
                    continue;
                }
                break;
            }
            default:
                if (!targetEntry)
                {
                    sLog.outErrorDb("Table `spell_script_target`: target entry == 0 for not GO target type (%u).", type);
                    continue;
                }
                if (const CreatureInfo* cInfo = sObjectMgr.GetCreatureTemplate(targetEntry))
                {
                    if (spellId == 30427 && !cInfo->skinning_loot_id)
                    {
                        sLog.outErrorDb("Table `spell_script_target` has creature %u as a target of spellid 30427, but this creature has no skinlootid. Gas extraction will not work!", cInfo->entry);
                        continue;
                    }
                }
                else
                {
                    if (!sObjectMgr.IsExistingCreatureId(targetEntry))
                        sLog.outErrorDb("Table `spell_script_target`: creature template entry %u does not exist.", targetEntry);
                    continue;
                }
                break;
        }

        mSpellScriptTarget.insert(SpellScriptTarget::value_type(spellId, SpellTargetEntry(SpellTargetType(type), targetEntry, conditionId, effectMask)));
    } while (result->NextRow());
}

void SpellMgr::LoadSpellPetAuras()
{
    mSpellPetAuraMap.clear(); // need for reload case

    //                                                              0      1    2
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT spell, pet, aura FROM spell_pet_auras"));
    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 spell = fields[0].GetUInt32();
        uint32 pet = fields[1].GetUInt32();
        uint32 aura = fields[2].GetUInt32();

        SpellPetAuraMap::iterator itr = mSpellPetAuraMap.find(spell);
        if (itr != mSpellPetAuraMap.end())
            itr->second.AddAura(pet, aura);
        else
        {
            SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(spell);
            if (!spellInfo)
            {
                sLog.outErrorDb("Spell %u listed in `spell_pet_auras` does not exist", spell);
                continue;
            }
            int i = 0;
            for (; i < MAX_EFFECT_INDEX; ++i)
                if ((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA &&
                        spellInfo->EffectApplyAuraName[i] == SPELL_AURA_DUMMY) ||
                        spellInfo->Effect[i] == SPELL_EFFECT_DUMMY)
                    break;

            if (i == MAX_EFFECT_INDEX)
            {
                sLog.outError("Spell %u listed in `spell_pet_auras` does not have dummy aura or dummy effect", spell);
                continue;
            }

            SpellEntry const* spellInfo2 = sSpellMgr.GetSpellEntry(aura);
            if (!spellInfo2)
            {
                sLog.outErrorDb("Aura %u listed in `spell_pet_auras` does not exist", aura);
                continue;
            }

            PetAura pa(pet, aura, spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_CASTER_PET, spellInfo->CalculateSimpleValue(SpellEffectIndex(i)));
            mSpellPetAuraMap[spell] = pa;
        }
    } while (result->NextRow());
}

/// Some checks for spells, to prevent adding deprecated/broken spells for trainers, spell book, etc
bool SpellMgr::IsSpellValid(SpellEntry const* spellInfo, Player* pl, bool msg)
{
    // not exist
    if (!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        switch (spellInfo->Effect[i])
        {
            case 0:
                continue;

            // craft spell for crafting nonexistent item (break client recipes list show)
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if (!sObjectMgr.GetItemPrototype(spellInfo->EffectItemType[i]))
                {
                    if (msg)
                    {
                        if (pl)
                            ChatHandler(pl).PSendSysMessage("Craft spell %u create item (Entry: %u) but item does not exist in item_template.", spellInfo->Id, spellInfo->EffectItemType[i]);
                        else
                            sLog.outErrorDb("Craft spell %u create item (Entry: %u) but item does not exist in item_template.", spellInfo->Id, spellInfo->EffectItemType[i]);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellEntry const* spellInfo2 = sSpellMgr.GetSpellEntry(spellInfo->EffectTriggerSpell[i]);
                if (!IsSpellValid(spellInfo2, pl, msg))
                {
                    if (msg)
                    {
                        if (pl)
                            ChatHandler(pl).PSendSysMessage("Spell %u learn to broken spell %u, and then...", spellInfo->Id, spellInfo->EffectTriggerSpell[i]);
                        else
                            sLog.outErrorDb("Spell %u learn to invalid spell %u, and then...", spellInfo->Id, spellInfo->EffectTriggerSpell[i]);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if (need_check_reagents)
    {
        for (int j : spellInfo->Reagent)
        {
            if (j > 0 && !sObjectMgr.GetItemPrototype(j))
            {
                if (msg)
                {
                    if (pl)
                        ChatHandler(pl).PSendSysMessage("Craft spell %u requires reagent item (Entry: %u) but item does not exist in item_template.", spellInfo->Id, j);
                    else
                        sLog.outErrorDb("Craft spell %u requires reagent item (Entry: %u) but item does not exist in item_template.", spellInfo->Id, j);
                }
                return false;
            }
        }
    }

    return true;
}

void SpellMgr::LoadSpellAreas()
{
    mSpellAreaMap.clear(); // need for reload case
    mSpellAreaForQuestMap.clear();
    mSpellAreaForActiveQuestMap.clear();
    mSpellAreaForQuestEndMap.clear();
    mSpellAreaForAuraMap.clear();

    //                                                              0      1     2            3                   4          5           6         7       8
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT spell, area, quest_start, quest_start_active, quest_end, aura_spell, racemask, gender, autocast FROM spell_area"));

    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 spell = fields[0].GetUInt32();
        SpellArea spellArea;
        spellArea.spellId             = spell;
        spellArea.areaId              = fields[1].GetUInt32();
        spellArea.questStart          = fields[2].GetUInt32();
        spellArea.questStartCanActive = fields[3].GetBool();
        spellArea.questEnd            = fields[4].GetUInt32();
        spellArea.auraSpell           = fields[5].GetInt32();
        spellArea.raceMask            = fields[6].GetUInt32();
        spellArea.gender              = Gender(fields[7].GetUInt8());
        spellArea.autocast            = fields[8].GetBool();

        if (!sSpellMgr.GetSpellEntry(spell))
        {
            if (!sSpellMgr.IsExistingSpellId(spell))
                sLog.outErrorDb("Spell %u listed in `spell_area` does not exist", spell);
            continue;
        }

        {
            bool ok = true;
            SpellAreaMapBounds sa_bounds = GetSpellAreaMapBounds(spellArea.spellId);
            for (SpellAreaMap::const_iterator itr = sa_bounds.first; itr != sa_bounds.second; ++itr)
            {
                if (spellArea.spellId != itr->second.spellId)
                    continue;
                if (spellArea.areaId != itr->second.areaId)
                    continue;
                if (spellArea.questStart != itr->second.questStart)
                    continue;
                if (spellArea.auraSpell != itr->second.auraSpell)
                    continue;
                if ((spellArea.raceMask & itr->second.raceMask) == 0)
                    continue;
                if (spellArea.gender != itr->second.gender)
                    continue;

                // duplicate by requirements
                ok = false;
                break;
            }

            if (!ok)
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` already listed with similar requirements.", spell);
                continue;
            }

        }

        if (spellArea.areaId && !AreaEntry::GetById(spellArea.areaId))
        {
            sLog.outErrorDb("Spell %u listed in `spell_area` have wrong area (%u) requirement", spell, spellArea.areaId);
            continue;
        }

        if (spellArea.questStart && !sObjectMgr.GetQuestTemplate(spellArea.questStart))
        {
            sLog.outErrorDb("Spell %u listed in `spell_area` have wrong start quest (%u) requirement", spell, spellArea.questStart);
            continue;
        }

        if (spellArea.questEnd)
        {
            if (!sObjectMgr.GetQuestTemplate(spellArea.questEnd))
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` have wrong end quest (%u) requirement", spell, spellArea.questEnd);
                continue;
            }

            if (spellArea.questEnd == spellArea.questStart && !spellArea.questStartCanActive)
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` have quest (%u) requirement for start and end in same time", spell, spellArea.questEnd);
                continue;
            }
        }

        if (spellArea.auraSpell)
        {
            SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(abs(spellArea.auraSpell));
            if (!spellInfo)
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` have wrong aura spell (%u) requirement", spell, abs(spellArea.auraSpell));
                continue;
            }

            bool validSpellEffect = false;
            for (uint32 i = EFFECT_INDEX_0; i < MAX_EFFECT_INDEX; ++i)
            {
                switch (spellInfo->EffectApplyAuraName[i])
                {
                    case SPELL_AURA_DUMMY:
                    case SPELL_AURA_GHOST:
                        validSpellEffect = true;
                        break;
                }
            }

            if (!validSpellEffect)
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` have aura spell requirement (%u) without dummy/ghost aura in spell effects", spell, abs(spellArea.auraSpell));
                continue;
            }

            if (uint32(abs(spellArea.auraSpell)) == spellArea.spellId)
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` have aura spell (%u) requirement for itself", spell, abs(spellArea.auraSpell));
                continue;
            }

            // not allow autocast chains by auraSpell field (but allow use as alternative if not present)
            if (spellArea.autocast && spellArea.auraSpell > 0)
            {
                bool chain = false;
                SpellAreaForAuraMapBounds saBound = GetSpellAreaForAuraMapBounds(spellArea.spellId);
                for (SpellAreaForAuraMap::const_iterator itr = saBound.first; itr != saBound.second; ++itr)
                {
                    if (itr->second->autocast && itr->second->auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    sLog.outErrorDb("Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura", spell, spellArea.auraSpell);
                    continue;
                }

                SpellAreaMapBounds saBound2 = GetSpellAreaMapBounds(spellArea.auraSpell);
                for (SpellAreaMap::const_iterator itr2 = saBound2.first; itr2 != saBound2.second; ++itr2)
                {
                    if (itr2->second.autocast && itr2->second.auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    sLog.outErrorDb("Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura", spell, spellArea.auraSpell);
                    continue;
                }
            }
        }

        if (spellArea.raceMask && (spellArea.raceMask & RACEMASK_ALL_PLAYABLE) == 0)
        {
            sLog.outErrorDb("Spell %u listed in `spell_area` have wrong race mask (%u) requirement", spell, spellArea.raceMask);
            continue;
        }

        if (spellArea.gender != GENDER_NONE && spellArea.gender != GENDER_FEMALE && spellArea.gender != GENDER_MALE)
        {
            sLog.outErrorDb("Spell %u listed in `spell_area` have wrong gender (%u) requirement", spell, spellArea.gender);
            continue;
        }

        SpellArea const* sa = &mSpellAreaMap.insert(SpellAreaMap::value_type(spell, spellArea))->second;

        // for search by current zone/subzone at zone/subzone change
        if (spellArea.areaId)
            mSpellAreaForAreaMap.insert(SpellAreaForAreaMap::value_type(spellArea.areaId, sa));

        // for search at quest start/reward
        if (spellArea.questStart)
        {
            if (spellArea.questStartCanActive)
                mSpellAreaForActiveQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart, sa));
            else
                mSpellAreaForQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart, sa));
        }

        // for search at quest start/reward
        if (spellArea.questEnd)
            mSpellAreaForQuestEndMap.insert(SpellAreaForQuestMap::value_type(spellArea.questEnd, sa));

        // for search at aura apply
        if (spellArea.auraSpell)
            mSpellAreaForAuraMap.insert(SpellAreaForAuraMap::value_type(abs(spellArea.auraSpell), sa));
    } while (result->NextRow());
}

SpellCastResult SpellMgr::GetSpellAllowedInLocationError(SpellEntry const* spellInfo, Unit const* caster, Player const* player)
{
    // Spell can be casted only in battleground
    if (spellInfo->HasAttribute(SPELL_ATTR_EX3_ONLY_BATTLEGROUNDS) &&
       (!player || !player->InBattleGround()))
        return SPELL_FAILED_ONLY_BATTLEGROUNDS;

    uint32 mapId = caster ? caster->GetMapId() : (player ? player->GetMapId() : 0);

	if (spellInfo->Id == 22563 && player && player->InBattleGround() && player->GetTeamId() != TEAM_HORDE)
		return SPELL_FAILED_SPELL_UNAVAILABLE;

	if (spellInfo->Id == 22564 && player && player->InBattleGround() && player->GetTeamId() != TEAM_ALLIANCE)
		return SPELL_FAILED_SPELL_UNAVAILABLE;

    // Custom arena spell blacklist.
    if (player && player->GetMapId() == 26)
    {
        if (spellInfo->HasEffect(SPELL_EFFECT_RESURRECT_NEW))
            return SPELL_FAILED_ONLY_BATTLEGROUNDS;

        switch (spellInfo->Id)
        {
            // long cd spells
            case 633:   // Lay on Hands (rank 1)
            case 2800:  // Lay on Hands (rank 2)
            case 10310: // Lay on Hands (rank 3)
            case 1719:  // Recklessness
            case 13180: // Gnomish Mind Control Cap
            case 22641: // Reckless Charge (Goblin Rocket Helmet)
            // guardian spells
            case 5666: // Summon Timberling
            case 6084: // Summon Ghost Saber
            case 7278: // Summon Harvester Swarm
            case 9515: // Summon Tracking Hound
            case 17490: // Summon Skeleton
            case 18307: // Death by Peasant (Horde)
            case 18308: // Death by Peasant (Alliance)
            case 19363: // Summon Mechanical Yeti
            case 23074: // Arcanite Dragonling
            case 23075: // Mithril Mechanical Dragonling
            case 23076: // Mechanical Dragonling
            case 26067: // Summon Mechanical Greench
            case 26391: // Tentacle Call
            case 29305: // Summon Cinder Elemental
            case 56523: // Decoy Dragonling
                return SPELL_FAILED_ONLY_BATTLEGROUNDS;
        }
    }

    switch (spellInfo->Id)
    {
        // Alterac Valley
        case 22564:                                         // Recall (Alliance)
        case 22563:                                         // Recall (Horde)
        case 23538:                                         // Battle Standard (Horde)
        case 23539:                                         // Battle Standard (Alliance)
        {
            if (!player)
                return SPELL_FAILED_REQUIRES_AREA;

            BattleGround* bg = player->GetBattleGround();

            return player->GetMapId() == 30 && bg
                   && bg->GetStatus() != STATUS_WAIT_JOIN ? SPELL_CAST_OK : SPELL_FAILED_REQUIRES_AREA;
        }
        // Warsong Gulch
        case 23333:                                         // Warsong Flag
        case 23335:                                         // Silverwing Flag
            return player && player->GetMapId() == 489 && player->InBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_REQUIRES_AREA;
        case 2584:                                          // Waiting to Resurrect
        {
        //  return player && player->InBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_ONLY_BATTLEGROUNDS;
            return SPELL_CAST_OK;
        }
        case 22011:                                         // Spirit Heal Channel
        case 22012:                                         // Spirit Heal
        case 24171:                                         // Resurrection Impact Visual
        {   /* Remove SPELL_CAST_OK if you return this check. Required for Turtle WoW Arena Tournament.
            MapEntry const* mapEntry = sMapStorage.LookupEntry<MapEntry>(mapId);
            if (!mapEntry)
                return SPELL_FAILED_REQUIRES_AREA;
            return mapEntry->IsBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_ONLY_BATTLEGROUNDS; 
            */
            return SPELL_CAST_OK;
        }
        // Disable big mounts in cities.
        case 46211: // Immortal Champion's Drake
        case 46212: // Riding Bronze Drake
        case 46504: // Red Goblin Shredder
        case 46505: // Green Goblin Shredder
        case 46506: // Red Spidertank
        case 46507: // Black Spidertank
        case 46508: // Blue Spidertank
        case 46509: // Green Spidertank
        case 46510: // Black Pounder
        case 46511: // Blue Pounder
        case 46512: // Green Pounder
        case 46513: // Red Pounder
        {
            switch (player->GetCachedAreaId())
            {
                case 1497: // Undercity
                case 1519: // Stormwind City
                case 1537: // Ironforge
                case 1637: // Orgrimmar
                case 1638: // Thunder Bluff
                case 1657: // Darnassus
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;
            }
            return SPELL_CAST_OK;
        }
        case 46038: // Little Winter Veil Tree
        {
            // Don't allow summoning xmas trees in instances.
            // Can be exploited in Naxxramas to avoid Sapphiron's frost breath. Probably other abuses too.
            if (caster && caster->GetMapId() > 1)
                return SPELL_FAILED_FIZZLE;
        }
    }

    if (caster)
    {
        SpellAreaMapBounds saBounds = GetSpellAreaMapBounds(spellInfo->Id);
        if (saBounds.first != saBounds.second)
        {
            // Performance optimization:
            // Compute zone/area only if necessay. Not at every spell cast.
            uint32 zone, area;
            caster->GetZoneAndAreaId(zone, area);
            for (SpellAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
                if (itr->second.IsFitToRequirements(player, zone, area))
                    return SPELL_CAST_OK;
            return SPELL_FAILED_REQUIRES_AREA;
        }
    }
    return SPELL_CAST_OK;
}

uint32 SpellMgr::GetRequiredAreaForSpell(uint32 spellId)
{
    SpellAreaMapBounds saBounds = GetSpellAreaMapBounds(spellId);
    if (saBounds.first != saBounds.second)
    {
        for (SpellAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            if (itr->second.areaId)
                return itr->second.areaId;
        }
    }

    // Not defined in database.
    switch (spellId)
    {
        // Alterac Valley
        case 22564: // Recall (Alliance)
        case 22563: // Recall (Horde)
        case 23538: // Battle Standard (Horde)
        case 23539: // Battle Standard (Alliance)
            return 2597;
        // Warsong Gulch
        case 23333: // Warsong Flag
        case 23335: // Silverwing Flag
            return 3277;
    }

    return 0;
}

SpellCastResult SpellMgr::GetSpellAllowedInLocationError(SpellEntry const *spellInfo, uint32 zone_id, uint32 area_id, Player const* player)
{
    // DB base check (if non empty then must fit at least single for allow)
    SpellAreaMapBounds saBounds = GetSpellAreaMapBounds(spellInfo->Id);
    if (saBounds.first != saBounds.second)
    {
        for (SpellAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            if (itr->second.IsFitToRequirements(player, zone_id, area_id))
                return SPELL_CAST_OK;
        }
        return SPELL_FAILED_REQUIRES_AREA;
    }
    return GetSpellAllowedInLocationError(spellInfo, nullptr /* zone/area already checked */, player);
}

void SpellMgr::LoadSkillLineAbilityMaps()
{
    mSkillLineAbilityMapBySpellId.clear();
    mSkillLineAbilityMapBySkillId.clear();

    const uint32 rows = sObjectMgr.GetMaxSkillLineAbilityId();
    for (uint32 row = 0; row < rows; ++row)
    {
        if (SkillLineAbilityEntry const* entry = sObjectMgr.GetSkillLineAbility(row))
        {
            mSkillLineAbilityMapBySpellId.insert(SkillLineAbilityMap::value_type(entry->spellId, entry));
            mSkillLineAbilityMapBySkillId.insert(SkillLineAbilityMap::value_type(entry->skillId, entry));
        }
    }
}

void SpellMgr::LoadSkillRaceClassInfoMap()
{
    mSkillRaceClassInfoMap.clear();

    for (uint32 i = 0; i < sSkillRaceClassInfoStore.GetNumRows(); ++i)
    {
        SkillRaceClassInfoEntry const *skillRCInfo = sSkillRaceClassInfoStore.LookupEntry(i);
        if (!skillRCInfo)
            continue;

        // not all skills really listed in ability skills list
        if (!sSkillLineStore.LookupEntry(skillRCInfo->skillId))
            continue;

        mSkillRaceClassInfoMap.insert(SkillRaceClassInfoMap::value_type(skillRCInfo->skillId, skillRCInfo));
    }
}

void SpellMgr::CheckUsedSpells(char const* table)
{
    uint32 countSpells = 0;
    uint32 countMasks = 0;

    //                                                               0       1               2               3         4           5             6          7          8         9    10
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT spellid,SpellFamilyName,SpellFamilyMask,SpellIcon,SpellVisual,SpellCategory,EffectType,EffectAura,EffectIdx,Name,Code FROM %s", table));

    if (!result)
    {
        sLog.outErrorDb("`%s` table is empty!", table);
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 spell       = fields[0].GetUInt32();
        int32  family      = fields[1].GetInt32();
        uint64 familyMask  = fields[2].GetUInt64();
        int32  spellIcon   = fields[3].GetInt32();
        int32  spellVisual = fields[4].GetInt32();
        int32  category    = fields[5].GetInt32();
        int32  effectType  = fields[6].GetInt32();
        int32  auraType    = fields[7].GetInt32();
        int32  effectIdx   = fields[8].GetInt32();
        std::string name   = fields[9].GetCppString();
        std::string code   = fields[10].GetCppString();

        // checks of correctness requirements itself

        if (family < -1 || family > SPELLFAMILY_UNK3)
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellFamily value(%u), skipped.", table, spell, family);
            continue;
        }

        // TODO: spellIcon check need dbc loading
        if (spellIcon < -1)
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellIcon value(%u), skipped.", table, spell, spellIcon);
            continue;
        }

        // TODO: spellVisual check need dbc loading
        if (spellVisual < -1)
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellVisual value(%u), skipped.", table, spell, spellVisual);
            continue;
        }

        // TODO: for spellCategory better check need dbc loading
        if (category < -1 || (category >= 0 && sSpellCategoriesStore.find(category) == sSpellCategoriesStore.end()))
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellCategory value(%u), skipped.", table, spell, category);
            continue;
        }

        if (effectType < -1 || effectType >= TOTAL_SPELL_EFFECTS)
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellEffect type value(%u), skipped.", table, spell, effectType);
            continue;
        }

        if (auraType < -1 || auraType >= TOTAL_AURAS)
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellAura type value(%u), skipped.", table, spell, auraType);
            continue;
        }

        if (effectIdx < -1 || effectIdx >= 3)
        {
            sLog.outError("Table '%s' for spell %u have wrong EffectIdx value(%u), skipped.", table, spell, effectIdx);
            continue;
        }

        // now checks of requirements

        if (spell)
        {
            ++countSpells;

            SpellEntry const* spellEntry = sSpellMgr.GetSpellEntry(spell);
            if (!spellEntry)
            {
                sLog.outError("Spell %u '%s' not exist but used in %s.", spell, name.c_str(), code.c_str());
                continue;
            }

            if (family >= 0 && spellEntry->SpellFamilyName != uint32(family))
            {
                sLog.outError("Spell %u '%s' family(%u) <> %u but used in %s.", spell, name.c_str(), spellEntry->SpellFamilyName, family, code.c_str());
                continue;
            }

            if (familyMask != UI64LIT(0xFFFFFFFFFFFFFFFF))
            {
                if (familyMask == UI64LIT(0x0000000000000000))
                {
                    if (spellEntry->SpellFamilyFlags)
                    {
                        sLog.outError("Spell %u '%s' not fit to (" UI64FMTD ") but used in %s.",
                                      spell, name.c_str(), familyMask, code.c_str());
                        continue;
                    }

                }
                else
                {
                    if (!spellEntry->IsFitToFamilyMask(familyMask))
                    {
                        sLog.outError("Spell %u '%s' not fit to (" I64FMT ") but used in %s.", spell, name.c_str(), familyMask, code.c_str());
                        continue;
                    }

                }
            }

            if (spellIcon >= 0 && spellEntry->SpellIconID != uint32(spellIcon))
            {
                sLog.outError("Spell %u '%s' icon(%u) <> %u but used in %s.", spell, name.c_str(), spellEntry->SpellIconID, spellIcon, code.c_str());
                continue;
            }

            if (spellVisual >= 0 && spellEntry->SpellVisual != uint32(spellVisual))
            {
                sLog.outError("Spell %u '%s' visual(%u) <> %u but used in %s.", spell, name.c_str(), spellEntry->SpellVisual, spellVisual, code.c_str());
                continue;
            }

            if (category >= 0 && spellEntry->Category != uint32(category))
            {
                sLog.outError("Spell %u '%s' category(%u) <> %u but used in %s.", spell, name.c_str(), spellEntry->Category, category, code.c_str());
                continue;
            }

            if (effectIdx >= EFFECT_INDEX_0)
            {
                if (effectType >= 0 && spellEntry->Effect[effectIdx] != uint32(effectType))
                {
                    sLog.outError("Spell %u '%s' effect%d <> %u but used in %s.", spell, name.c_str(), effectIdx + 1, effectType, code.c_str());
                    continue;
                }

                if (auraType >= 0 && spellEntry->EffectApplyAuraName[effectIdx] != uint32(auraType))
                {
                    sLog.outError("Spell %u '%s' aura%d <> %u but used in %s.", spell, name.c_str(), effectIdx + 1, auraType, code.c_str());
                    continue;
                }

            }
            else
            {
                if (effectType >= 0 && !spellEntry->HasEffect(SpellEffects(effectType)))
                {
                    sLog.outError("Spell %u '%s' not have effect %u but used in %s.", spell, name.c_str(), effectType, code.c_str());
                    continue;
                }

                if (auraType >= 0 && !spellEntry->HasAura(AuraType(auraType)))
                {
                    sLog.outError("Spell %u '%s' not have aura %u but used in %s.", spell, name.c_str(), auraType, code.c_str());
                    continue;
                }
            }
        }
        else
        {
            ++countMasks;

            bool found = false;
            for (uint32 spellId = 1; spellId < sSpellMgr.GetMaxSpellId(); ++spellId)
            {
                SpellEntry const* spellEntry = sSpellMgr.GetSpellEntry(spellId);
                if (!spellEntry)
                    continue;

                if (family >= 0 && spellEntry->SpellFamilyName != uint32(family))
                    continue;

                if (familyMask != UI64LIT(0xFFFFFFFFFFFFFFFF))
                {
                    if (familyMask == UI64LIT(0x0000000000000000))
                    {
                        if (spellEntry->SpellFamilyFlags)
                            continue;
                    }
                    else
                    {
                        if (!spellEntry->IsFitToFamilyMask(familyMask))
                            continue;
                    }
                }

                if (spellIcon >= 0 && spellEntry->SpellIconID != uint32(spellIcon))
                    continue;

                if (spellVisual >= 0 && spellEntry->SpellVisual != uint32(spellVisual))
                    continue;

                if (category >= 0 && spellEntry->Category != uint32(category))
                    continue;

                if (effectIdx >= 0)
                {
                    if (effectType >= 0 && spellEntry->Effect[effectIdx] != uint32(effectType))
                        continue;

                    if (auraType >= 0 && spellEntry->EffectApplyAuraName[effectIdx] != uint32(auraType))
                        continue;
                }
                else
                {
                    if (effectType >= 0 && !spellEntry->HasEffect(SpellEffects(effectType)))
                        continue;

                    if (auraType >= 0 && !spellEntry->HasAura(AuraType(auraType)))
                        continue;
                }

                found = true;
                break;
            }

            if (!found)
            {
                if (effectIdx >= 0)
                    sLog.outError("Spells '%s' not found for family %i (" I64FMT ") icon(%i) visual(%i) category(%i) effect%d(%i) aura%d(%i) but used in %s",
                                  name.c_str(), family, familyMask, spellIcon, spellVisual, category, effectIdx + 1, effectType, effectIdx + 1, auraType, code.c_str());
                else
                    sLog.outError("Spells '%s' not found for family %i (" I64FMT ") icon(%i) visual(%i) category(%i) effect(%i) aura(%i) but used in %s",
                                  name.c_str(), family, familyMask, spellIcon, spellVisual, category, effectType, auraType, code.c_str());
                continue;
            }
        }
    } while (result->NextRow());

    sLog.outString("Checked %u spells and %u spell masks.", countSpells, countMasks);
}

bool SpellArea::IsFitToRequirements(Player const* player, uint32 newZone, uint32 newArea) const
{
    if (gender != GENDER_NONE)
    {
        // not in expected gender
        if (!player || gender != player->GetGender())
            return false;
    }

    if (raceMask)
    {
        // not in expected race
        if (!player || !(raceMask & player->GetRaceMask()))
            return false;
    }

    if (areaId)
    {
        // not in expected zone
        if (newZone != areaId && newArea != areaId)
            return false;
    }

    if (questStart)
    {
        // not in expected required quest state
        if (!player || ((!questStartCanActive || !player->IsActiveQuest(questStart)) && !player->GetQuestRewardStatus(questStart)))
            return false;
    }

    if (questEnd)
    {
        // not in expected forbidden quest state
        if (!player || player->GetQuestRewardStatus(questEnd))
            return false;
    }

    if (auraSpell)
    {
        // not have expected aura
        if (!player)
            return false;
        if (auraSpell > 0)
            // have expected aura
            return player->HasAura(auraSpell);
        else
            // not have expected aura
            return !player->HasAura(-auraSpell);
    }

    return true;
}

void SpellMgr::LoadSpellAffects()
{
    mSpellAffectMap.clear(); // need for reload case

    //                                                               0      1         2
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT entry, effectId, SpellFamilyMask FROM spell_affect"));
    if (!result)
    {
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint8 effectId = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(entry);

        if (!spellInfo)
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` does not exist", entry);
            continue;
        }

        if (effectId >= MAX_EFFECT_INDEX)
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` have invalid effect index (%u)", entry, effectId);
            continue;
        }

        if (spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA || (
                    spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
                    spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
                    spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER &&
                    spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_OVERRIDE_CLASS_SCRIPTS))
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` have not SPELL_AURA_ADD_FLAT_MODIFIER (%u) or SPELL_AURA_ADD_PCT_MODIFIER (%u) or SPELL_AURA_ADD_TARGET_TRIGGER (%u) or SPELL_AURA_OVERRIDE_CLASS_SCRIPTS (%u) for effect index (%u)", entry, SPELL_AURA_ADD_FLAT_MODIFIER, SPELL_AURA_ADD_PCT_MODIFIER, SPELL_AURA_ADD_TARGET_TRIGGER, SPELL_AURA_OVERRIDE_CLASS_SCRIPTS, effectId);
            continue;
        }

        uint64 spellAffectMask = fields[2].GetUInt64();

        // Spell.dbc have own data for low part of SpellFamilyMask
        if (spellInfo->EffectItemType[effectId])
        {
            if (static_cast<uint64>(spellInfo->EffectItemType[effectId]) == spellAffectMask)
            {
                sLog.outErrorDb("Spell %u listed in `spell_affect` have redundant (same with EffectItemType%d) data for effect index (%u) and not needed, skipped.", entry, effectId + 1, effectId);
                continue;
            }
        }

        mSpellAffectMap.insert(SpellAffectMap::value_type((entry << 8) + effectId, spellAffectMask));
    } while (result->NextRow());

    for (uint32 id = 0; id < sSpellMgr.GetMaxSpellId(); ++id)
    {
        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(id);
        if (!spellInfo)
            continue;

        for (int effectId = 0; effectId < MAX_EFFECT_INDEX; ++effectId)
        {
            if (spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA || (
                        spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
                        spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
                        spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER))
                continue;

            if (spellInfo->EffectItemType[effectId] != 0)
                continue;

            if (mSpellAffectMap.find((id << 8) + effectId) !=  mSpellAffectMap.end())
                continue;

            sLog.outErrorDb("Spell %u (%s) misses spell_affect for effect %u", id, spellInfo->SpellName[sWorld.GetDefaultDbcLocale()].c_str(), effectId);
        }
    }
}

void SpellMgr::LoadExistingSpellIds()
{
    mExistingSpellsSet.clear();

    Field* fields;
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT DISTINCT `entry` FROM `spell_template`"));

    if (result)
    {
        do
        {
            fields = result->Fetch();
            uint32 id = fields[0].GetUInt32();
            mExistingSpellsSet.insert(id);
        } while (result->NextRow());
    }
}

namespace SpellInternal
{
    bool IsSpellAppliesAura(SpellEntry const *spellInfo)
    {
        for (uint32 i : spellInfo->Effect)
        {
            if (SpellEffects(i) == SPELL_EFFECT_APPLY_AURA || SpellEffects(i) == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                return true;
        }
        return false;
    }

    // Spells that apply damage or heal over time
    // Returns false for periodic and direct mixed spells (immolate, etc)
    bool IsSpellAppliesPeriodicAura(SpellEntry const *spellInfo)
    {
        bool periodic = false;
        bool direct = false;
        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            switch (spellInfo->Effect[i])
            {
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                case SPELL_EFFECT_POWER_DRAIN:
                case SPELL_EFFECT_HEALTH_LEECH:
                case SPELL_EFFECT_ENVIRONMENTAL_DAMAGE:
                case SPELL_EFFECT_POWER_BURN:
                case SPELL_EFFECT_HEAL:
                    direct = true;
                    break;
                case SPELL_EFFECT_APPLY_AURA:
                    switch (spellInfo->EffectApplyAuraName[i])
                    {
                        case SPELL_AURA_PERIODIC_DAMAGE:
                        case SPELL_AURA_PERIODIC_HEAL:
                        case SPELL_AURA_PERIODIC_ENERGIZE:
                        case SPELL_AURA_OBS_MOD_HEALTH:
                        case SPELL_AURA_PERIODIC_LEECH:
                        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
                        case SPELL_AURA_PERIODIC_MANA_LEECH:
                        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                        case SPELL_AURA_POWER_BURN_MANA:
                        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                            periodic = true;
                        default:
                            break;
                    }
                default:
                    break;
            }
        }
        return periodic && !direct;
    }

    bool IsPassiveSpellStackableWithRanks(SpellEntry const* spellInfo)
    {
        if (!spellInfo->IsPassiveSpell())
            return false;

        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (SpellEffects(spellInfo->Effect[i]) == SPELL_EFFECT_APPLY_AURA || SpellEffects(spellInfo->Effect[i]) == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                return false;
        }

        return true;
    }
    
    bool IsHealSpell(SpellEntry const *spellInfo)
    {
        // Holy Light/Flash of Light
        if (spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN)
        {
            if (spellInfo->IsFitToFamilyMask<CF_PALADIN_FLASH_OF_LIGHT2>() ||
                spellInfo->IsFitToFamilyMask<CF_PALADIN_HOLY_LIGHT2>())
                return true;
        }

        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            switch (spellInfo->Effect[i])
            {
                case SPELL_EFFECT_HEAL:
                case SPELL_EFFECT_HEAL_MAX_HEALTH:
                    return true;
                case SPELL_EFFECT_APPLY_AURA:
                case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
                case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
                case SPELL_EFFECT_APPLY_AREA_AURA_RAID:
                case SPELL_EFFECT_APPLY_AREA_AURA_PET:
                {
                    switch (spellInfo->EffectApplyAuraName[i])
                    {
                        case SPELL_AURA_PERIODIC_HEAL:
                            return true;
                    }
                    break;
                }
            }
        }

        return false;
    }

    bool IsDirectDamageSpell(SpellEntry const* spellInfo)
    {
        for (uint32 i : spellInfo->Effect)
        {
            if (IsDirectDamageEffect(i))
                return true;
        }

        return false;
    }

    bool IsSpellWithCasterSourceTargetsOnly(SpellEntry const* spellInfo)
    {
        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            uint32 targetA = spellInfo->EffectImplicitTargetA[i];
            if (targetA && !IsCasterSourceTarget(targetA))
                return false;

            uint32 targetB = spellInfo->EffectImplicitTargetB[i];
            if (targetB && !IsCasterSourceTarget(targetB))
                return false;

            if (!targetA && !targetB)
                return false;
        }
        return true;
    }

    bool IsAreaOfEffectSpell(SpellEntry const *spellInfo)
    {
        return
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_0])) ||
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetB[EFFECT_INDEX_0])) ||
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_1])) ||
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetB[EFFECT_INDEX_1])) ||
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_2])) ||
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetB[EFFECT_INDEX_2]));
    }

    bool HasAreaAuraEffect(SpellEntry const *spellInfo)
    {
        for (uint32 i : spellInfo->Effect)
            if (IsAreaAuraEffect(i))
                return true;

        return false;
    }

    bool IsDismountSpell(SpellEntry const *spellInfo)
    {
        for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if ((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA) && (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MECHANIC_IMMUNITY) && (spellInfo->EffectMiscValue[i] == MECHANIC_MOUNT))
                return true;
        }
        return false;
    }

    bool IsCharmSpell(SpellEntry const *spellInfo)
    {
        return spellInfo->HasAura(SPELL_AURA_MOD_CHARM) || spellInfo->HasAura(SPELL_AURA_MOD_POSSESS);
    }

    bool IsReflectableSpell(SpellEntry const* spellInfo)
    {
        return spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC && !spellInfo->HasAttribute(SPELL_ATTR_IS_ABILITY)
            && !spellInfo->HasAttribute(SPELL_ATTR_EX_CANT_BE_REFLECTED) && !spellInfo->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY)
            && !spellInfo->HasAttribute(SPELL_ATTR_PASSIVE) && !spellInfo->IsPositiveSpell();
    }
}

void SpellMgr::AssignInternalSpellFlags()
{
    for (auto& pSpellEntry : mSpellEntryMap)
    {
        if (pSpellEntry)
        {
            if (SpellInternal::IsSpellAppliesAura(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_APPLIES_AURA;

            if (SpellInternal::IsSpellAppliesPeriodicAura(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_APPLIES_PERIODIC_AURA;

            if (SpellInternal::IsPassiveSpellStackableWithRanks(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_PASSIVE_STACK_WITH_RANKS;

            if (pSpellEntry->IsPositiveSpell(nullptr, nullptr))
                pSpellEntry->Internal |= SPELL_INTERNAL_POSITIVE;

            if (SpellInternal::IsHealSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_HEAL;

            if (SpellInternal::IsDirectDamageSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_DIRECT_DAMAGE;

            if (SpellInternal::IsSpellWithCasterSourceTargetsOnly(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_CASTER_SOURCE_TARGETS;

            if (SpellInternal::IsAreaOfEffectSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_AOE;

            if (SpellInternal::HasAreaAuraEffect(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_AOE_AURA;

            if (SpellInternal::IsDismountSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_DISMOUNT;

            if (SpellInternal::IsCharmSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_CHARM;

            if (SpellInternal::IsReflectableSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_REFLECTABLE;
        }
    }
}

void ParseTooltip(SpellEntry* entry)
{
    auto tooltip = entry->ToolTip[0];

    std::string parsedTooltip = "";

    for (auto i = 0; i < tooltip.length(); ++i)
    {
        char ch = tooltip[i];
        if (ch != '$')
        {
            parsedTooltip += ch;
            continue;
        }

        ++i;
        char expression = tooltip[i];
        ++i;
        int index = tooltip[i] - '0' - 1;

        switch (expression)
        {
            case 't':
            {
                //periodic.
                parsedTooltip += std::to_string(entry->EffectAmplitude[index] / 1000);
            }break;

            case 's':
            {
                //basepoints

                auto dieSides = entry->EffectDieSides[index];
                auto val = 0;
                auto val2 = 0;
                bool range = false;
                if (dieSides == 0 || dieSides == 1)
                    val = entry->EffectBasePoints[index] + dieSides;
                else
                {
                    val = entry->EffectBasePoints[index] + 1;
                    val2 = entry->EffectBasePoints[index] + dieSides;
                    range = true;
                }

                if (val < 0)
                    val *= -1;

                if (range)
                    parsedTooltip += std::to_string(val) + std::string{ " to " } + std::to_string(val2);
                else
                    parsedTooltip += std::to_string(val);
            }break;
        }
    }
    entry->ParsedTooltip = parsedTooltip;
}

void SpellMgr::LoadSpells()
{
    uint32 oldMSTime = WorldTimer::getMSTime();

    // Getting the maximum ID.
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT MAX(`entry`) FROM `spell_template`"));

    if (!result)
    {
        return;
    }
    auto fields = result->Fetch();
    uint32 maxEntry = fields[0].GetUInt32() + 1;

    // Actually loading the spells.
    result.reset(WorldDatabase.Query("SELECT * FROM `spell_template`"));

    if (!result)
    {
        return;
    }
    
    mSpellEntryMap.resize(maxEntry);

    do
    {
        fields = result->Fetch();

        std::unique_ptr<SpellEntry> spell = std::make_unique<SpellEntry>();

        uint32 spellId = fields[0].GetUInt32();

        spell->Id = spellId;
        spell->School = fields[1].GetUInt32();
        spell->Category = fields[2].GetUInt32();
        spell->Dispel = fields[4].GetUInt32();
        spell->Mechanic = fields[5].GetUInt32();
        spell->Attributes = fields[6].GetUInt32();
        spell->AttributesEx = fields[7].GetUInt32();
        spell->AttributesEx2 = fields[8].GetUInt32();
        spell->AttributesEx3 = fields[9].GetUInt32();
        spell->AttributesEx4 = fields[10].GetUInt32();
        spell->Stances = fields[11].GetUInt32();
        spell->StancesNot = fields[12].GetUInt32();
        spell->Targets = fields[13].GetUInt32();
        spell->TargetCreatureType = fields[14].GetUInt32();
        spell->RequiresSpellFocus = fields[15].GetUInt32();
        spell->CasterAuraState = fields[16].GetUInt32();
        spell->TargetAuraState = fields[17].GetUInt32();
        spell->CastingTimeIndex = fields[18].GetUInt32();
        spell->RecoveryTime = fields[19].GetUInt32();
        spell->CategoryRecoveryTime = fields[20].GetUInt32();
        spell->InterruptFlags = fields[21].GetUInt32();
        spell->AuraInterruptFlags = fields[22].GetUInt32();
        spell->ChannelInterruptFlags = fields[23].GetUInt32();
        spell->procFlags = fields[24].GetUInt32();
        spell->procChance = fields[25].GetUInt32();
        spell->procCharges = fields[26].GetUInt32();
        spell->maxLevel = fields[27].GetUInt32();
        spell->baseLevel = fields[28].GetUInt32();
        spell->spellLevel = fields[29].GetUInt32();
        spell->DurationIndex = fields[30].GetUInt32();
        spell->powerType = fields[31].GetUInt32();
        spell->manaCost = fields[32].GetUInt32();
        spell->manaCostPerlevel = fields[33].GetUInt32();
        spell->manaPerSecond = fields[34].GetUInt32();
        spell->manaPerSecondPerLevel = fields[35].GetUInt32();
        spell->rangeIndex = fields[36].GetUInt32();
        spell->speed = fields[37].GetFloat();
        spell->StackAmount = fields[39].GetUInt32();
        spell->Totem[0] = fields[40].GetUInt32();
        spell->Totem[1] = fields[41].GetUInt32();
        spell->Reagent[0] = fields[42].GetInt32();
        spell->Reagent[1] = fields[43].GetInt32();
        spell->Reagent[2] = fields[44].GetInt32();
        spell->Reagent[3] = fields[45].GetInt32();
        spell->Reagent[4] = fields[46].GetInt32();
        spell->Reagent[5] = fields[47].GetInt32();
        spell->Reagent[6] = fields[48].GetInt32();
        spell->Reagent[7] = fields[49].GetInt32();
        spell->ReagentCount[0] = fields[50].GetUInt32();
        spell->ReagentCount[1] = fields[51].GetUInt32();
        spell->ReagentCount[2] = fields[52].GetUInt32();
        spell->ReagentCount[3] = fields[53].GetUInt32();
        spell->ReagentCount[4] = fields[54].GetUInt32();
        spell->ReagentCount[5] = fields[55].GetUInt32();
        spell->ReagentCount[6] = fields[56].GetUInt32();
        spell->ReagentCount[7] = fields[57].GetUInt32();
        spell->EquippedItemClass = fields[58].GetInt32();
        spell->EquippedItemSubClassMask = fields[59].GetInt32();
        spell->EquippedItemInventoryTypeMask = fields[60].GetInt32();
        spell->Effect[0] = fields[61].GetUInt32();
        spell->Effect[1] = fields[62].GetUInt32();
        spell->Effect[2] = fields[63].GetUInt32();
        spell->EffectDieSides[0] = fields[64].GetInt32();
        spell->EffectDieSides[1] = fields[65].GetInt32();
        spell->EffectDieSides[2] = fields[66].GetInt32();
        spell->EffectBaseDice[0] = fields[67].GetUInt32();
        spell->EffectBaseDice[1] = fields[68].GetUInt32();
        spell->EffectBaseDice[2] = fields[69].GetUInt32();
        spell->EffectDicePerLevel[0] = fields[70].GetFloat();
        spell->EffectDicePerLevel[1] = fields[71].GetFloat();
        spell->EffectDicePerLevel[2] = fields[72].GetFloat();
        spell->EffectRealPointsPerLevel[0] = fields[73].GetFloat();
        spell->EffectRealPointsPerLevel[1] = fields[74].GetFloat();
        spell->EffectRealPointsPerLevel[2] = fields[75].GetFloat();
        spell->EffectBasePoints[0] = fields[76].GetInt32();
        spell->EffectBasePoints[1] = fields[77].GetInt32();
        spell->EffectBasePoints[2] = fields[78].GetInt32();
        spell->EffectBonusCoefficient[0] = fields[79].GetFloat();
        spell->EffectBonusCoefficient[1] = fields[80].GetFloat();
        spell->EffectBonusCoefficient[2] = fields[81].GetFloat();

        spell->EffectMechanic[0] = fields[82].GetUInt32();
        spell->EffectMechanic[1] = fields[83].GetUInt32();
        spell->EffectMechanic[2] = fields[84].GetUInt32();
        spell->EffectImplicitTargetA[0] = fields[85].GetUInt32();
        spell->EffectImplicitTargetA[1] = fields[86].GetUInt32();
        spell->EffectImplicitTargetA[2] = fields[87].GetUInt32();
        spell->EffectImplicitTargetB[0] = fields[88].GetUInt32();
        spell->EffectImplicitTargetB[1] = fields[89].GetUInt32();
        spell->EffectImplicitTargetB[2] = fields[90].GetUInt32();
        spell->EffectRadiusIndex[0] = fields[91].GetUInt32();
        spell->EffectRadiusIndex[1] = fields[92].GetUInt32();
        spell->EffectRadiusIndex[2] = fields[93].GetUInt32();
        spell->EffectApplyAuraName[0] = fields[94].GetUInt32();
        spell->EffectApplyAuraName[1] = fields[95].GetUInt32();
        spell->EffectApplyAuraName[2] = fields[96].GetUInt32();
        spell->EffectAmplitude[0] = fields[97].GetUInt32();
        spell->EffectAmplitude[1] = fields[98].GetUInt32();
        spell->EffectAmplitude[2] = fields[99].GetUInt32();
        spell->EffectMultipleValue[0] = fields[100].GetFloat();
        spell->EffectMultipleValue[1] = fields[101].GetFloat();
        spell->EffectMultipleValue[2] = fields[102].GetFloat();
        spell->EffectChainTarget[0] = fields[103].GetUInt32();
        spell->EffectChainTarget[1] = fields[104].GetUInt32();
        spell->EffectChainTarget[2] = fields[105].GetUInt32();
        spell->EffectItemType[0] = fields[106].GetUInt32();
        spell->EffectItemType[1] = fields[107].GetUInt32();
        spell->EffectItemType[2] = fields[108].GetUInt32();
        spell->EffectMiscValue[0] = fields[109].GetInt32();
        spell->EffectMiscValue[1] = fields[110].GetInt32();
        spell->EffectMiscValue[2] = fields[111].GetInt32();
        spell->EffectTriggerSpell[0] = fields[112].GetUInt32();
        spell->EffectTriggerSpell[1] = fields[113].GetUInt32();
        spell->EffectTriggerSpell[2] = fields[114].GetUInt32();
        spell->EffectPointsPerComboPoint[0] = fields[115].GetFloat();
        spell->EffectPointsPerComboPoint[1] = fields[116].GetFloat();
        spell->EffectPointsPerComboPoint[2] = fields[117].GetFloat();
        spell->SpellVisual = fields[118].GetUInt32();
        spell->SpellIconID = fields[120].GetUInt32();
        spell->activeIconID = fields[121].GetUInt32();
        spell->spellPriority = fields[122].GetUInt32();
        spell->SpellName[0] = fields[123].GetCppString();
        spell->Rank[0] = fields[125].GetCppString();
        spell->ToolTip[0] = fields[129].GetCppString();
        spell->ManaCostPercentage = fields[131].GetUInt32();
        spell->StartRecoveryCategory = fields[132].GetUInt32();
        spell->StartRecoveryTime = fields[133].GetUInt32();
        spell->MinTargetLevel = fields[134].GetUInt32();
        spell->MaxTargetLevel = fields[135].GetUInt32();
        spell->SpellFamilyName = fields[136].GetUInt32();
        spell->SpellFamilyFlags = fields[137].GetUInt64();
        spell->MaxAffectedTargets = fields[138].GetUInt32();
        spell->DmgClass = fields[139].GetUInt32();
        spell->PreventionType = fields[140].GetUInt32();
        spell->DmgMultiplier[0] = fields[142].GetFloat();
        spell->DmgMultiplier[1] = fields[143].GetFloat();
        spell->DmgMultiplier[2] = fields[144].GetFloat();
        spell->Custom = fields[148].GetUInt32();
        ParseTooltip(spell.get());

     
        spell->InitCachedValues();
        mSpellEntryMap[spellId] = std::move(spell);

    } while (result->NextRow());

    if (sWorld.getConfig(CONFIG_BOOL_LOAD_LOCALES))
    {
        // Load localized texts.
        //                                        0        1            2            3            4            5            6            7                   8                   9                   10                  11                  12                  13                  14                  15                  16                  17                  18                  19                      20                      21                      22                      23                      24
        result.reset(WorldDatabase.Query("SELECT `entry`, `name_loc1`, `name_loc2`, `name_loc3`, `name_loc4`, `name_loc5`, `name_loc6`, `nameSubtext_loc1`, `nameSubtext_loc2`, `nameSubtext_loc3`, `nameSubtext_loc4`, `nameSubtext_loc5`, `nameSubtext_loc6`, `description_loc1`, `description_loc2`, `description_loc3`, `description_loc4`, `description_loc5`, `description_loc6`, `auraDescription_loc1`, `auraDescription_loc2`, `auraDescription_loc3`, `auraDescription_loc4`, `auraDescription_loc5`, `auraDescription_loc6` FROM `locales_spell`"));
        if (result)
        {
            do
            {
                fields = result->Fetch();
                uint32 spellId = fields[0].GetUInt32();
                if ((spellId > maxEntry) || (!mSpellEntryMap[spellId]))
                    continue;

                mSpellEntryMap[spellId]->SpellName[1] = fields[1].GetCppString();
                mSpellEntryMap[spellId]->SpellName[2] = fields[2].GetCppString();
                mSpellEntryMap[spellId]->SpellName[3] = fields[3].GetCppString();
                mSpellEntryMap[spellId]->SpellName[4] = fields[4].GetCppString();
                mSpellEntryMap[spellId]->SpellName[5] = fields[5].GetCppString();
                mSpellEntryMap[spellId]->SpellName[6] = fields[6].GetCppString();
                mSpellEntryMap[spellId]->Rank[1] = fields[7].GetCppString();
                mSpellEntryMap[spellId]->Rank[2] = fields[8].GetCppString();
                mSpellEntryMap[spellId]->Rank[3] = fields[9].GetCppString();
                mSpellEntryMap[spellId]->Rank[4] = fields[10].GetCppString();
                mSpellEntryMap[spellId]->Rank[5] = fields[11].GetCppString();
                mSpellEntryMap[spellId]->Rank[6] = fields[12].GetCppString();

                mSpellEntryMap[spellId]->ToolTip[1] = fields[19].GetCppString();
                mSpellEntryMap[spellId]->ToolTip[2] = fields[20].GetCppString();
                mSpellEntryMap[spellId]->ToolTip[3] = fields[21].GetCppString();
                mSpellEntryMap[spellId]->ToolTip[4] = fields[22].GetCppString();
                mSpellEntryMap[spellId]->ToolTip[5] = fields[23].GetCppString();
                mSpellEntryMap[spellId]->ToolTip[6] = fields[24].GetCppString();

            } while (result->NextRow());
        }
    }
}
