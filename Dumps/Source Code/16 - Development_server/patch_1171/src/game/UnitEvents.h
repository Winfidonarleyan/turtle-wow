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

#ifndef _UNITEVENTS
#define _UNITEVENTS

#include "Common.h"

class ThreatContainer;
class ThreatManager;
class HostileReference;

//==============================================================
//==============================================================

enum UnitThreatEventType
{
    // Player/Pet changed on/offline status
    UEV_THREAT_REF_ONLINE_STATUS        = 1<<0,

    // Threat for Player/Pet changed
    UEV_THREAT_REF_THREAT_CHANGE        = 1<<1,

    // Player/Pet will be removed from list (dead) [for internal use]
    UEV_THREAT_REF_REMOVE_FROM_LIST     = 1<<2,

    // Player/Pet entered/left  water or some other place where it is/was not accessible for the creature
    UEV_THREAT_REF_ASSECCIBLE_STATUS    = 1<<3,

    // Threat list is going to be sorted (if dirty flag is set)
    UEV_THREAT_SORT_LIST                = 1<<4,

    // New target should be fetched, could tbe the current target as well
    UEV_THREAT_SET_NEXT_TARGET          = 1<<5,

    // A new victim (target) was set. Could be nullptr
    UEV_THREAT_VICTIM_CHANGED           = 1<<6,
};

#define UEV_THREAT_REF_EVENT_MASK ( UEV_THREAT_REF_ONLINE_STATUS | UEV_THREAT_REF_THREAT_CHANGE | UEV_THREAT_REF_REMOVE_FROM_LIST | UEV_THREAT_REF_ASSECCIBLE_STATUS)
#define UEV_THREAT_MANAGER_EVENT_MASK (UEV_THREAT_SORT_LIST | UEV_THREAT_SET_NEXT_TARGET | UEV_THREAT_VICTIM_CHANGED)
#define UEV_ALL_EVENT_MASK (0xffffffff)

// Future use
//#define UEV_UNIT_EVENT_MASK (UEV_UNIT_KILLED | UEV_UNIT_HEALTH_CHANGE)

//==============================================================

class UnitBaseEvent
{
    private:
        uint32 iType;
    public:
        UnitBaseEvent(uint32 pType) { iType = pType; }
        uint32 getType() const { return iType; }
        bool matchesTypeMask(uint32 pMask) const { return iType & pMask; }

        void setType(uint32 pType) { iType = pType; }

};

//==============================================================

class ThreatRefStatusChangeEvent : public UnitBaseEvent
{
    private:
        HostileReference* iHostileReference;
        union
        {
            float iFValue;
            int32 iIValue;
            bool iBValue;
        };
        ThreatManager* iThreatManager;
    public:
        ThreatRefStatusChangeEvent(uint32 pType) : UnitBaseEvent(pType), iThreatManager(nullptr) { iHostileReference = nullptr; }

        ThreatRefStatusChangeEvent(uint32 pType, HostileReference* pHostileReference) : UnitBaseEvent(pType), iThreatManager(nullptr) { iHostileReference = pHostileReference; }

        ThreatRefStatusChangeEvent(uint32 pType, HostileReference* pHostileReference, float pValue) : UnitBaseEvent(pType), iThreatManager(nullptr) { iHostileReference = pHostileReference; iFValue = pValue; }

        ThreatRefStatusChangeEvent(uint32 pType, HostileReference* pHostileReference, bool pValue) : UnitBaseEvent(pType), iThreatManager(nullptr) { iHostileReference = pHostileReference; iBValue = pValue; }

        int32 getIValue() const { return iIValue; }

        float getFValue() const { return iFValue; }

        bool getBValue() const { return iBValue; }

        void setBValue(bool pValue) { iBValue = pValue; }

        HostileReference* getReference() const { return iHostileReference; }

        void setThreatManager(ThreatManager* pThreatManager) { iThreatManager = pThreatManager; }

        ThreatManager* GetThreatManager() const { return iThreatManager; }
};

//==============================================================

class ThreatManagerEvent : public ThreatRefStatusChangeEvent
{
    private:
        ThreatContainer* iThreatContainer;
    public:
        ThreatManagerEvent(uint32 pType) : ThreatRefStatusChangeEvent(pType), iThreatContainer(nullptr) {}
        ThreatManagerEvent(uint32 pType, HostileReference* pHostileReference) : ThreatRefStatusChangeEvent(pType, pHostileReference), iThreatContainer(nullptr) {}

        void setThreatContainer(ThreatContainer* pThreatContainer) { iThreatContainer = pThreatContainer; }

        ThreatContainer* getThreatContainer() const { return iThreatContainer; }
};

//==============================================================
#endif
