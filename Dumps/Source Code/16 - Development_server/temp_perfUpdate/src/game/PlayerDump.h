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

#ifndef _PLAYER_DUMP_H
#define _PLAYER_DUMP_H

#include <string>
#include <map>
#include <set>

enum DumpTableType
{
    DTT_CHARACTER,      //    -> guid, name                 // characters

    DTT_CHAR_TABLE,     //                                  // character_action, character_aura, character_homebind,
                                                            // character_queststatus, character_reputation,
                                                            // character_spell, character_spell_cooldown, character_ticket,
                                                            // character_tutorial

    DTT_INVENTORY,      //    -> item guids collection      // character_inventory

    DTT_MAIL,           //    -> mail ids collection        // mail
                        //    -> item_text

    DTT_MAIL_ITEM,      // <- mail ids                      // mail_items
                        //    -> item guids collection

    DTT_ITEM,           // <- item guids                    // item_instance
                        //    -> item_text

    DTT_ITEM_GIFT,      // <- item guids                    // character_gifts

    DTT_ITEM_LOOT,      // <- item guids                    // item_loot

    DTT_PET,            //    -> pet guids collection       // character_pet
    DTT_PET_TABLE,      // <- pet guids                     // pet_aura, pet_spell, pet_spell_cooldown
    DTT_ITEM_TEXT,      // <- item_text                     // item_text
};

enum DumpReturn
{
    DUMP_SUCCESS,
    DUMP_FILE_OPEN_ERROR,
    DUMP_TOO_MANY_CHARS,
    DUMP_UNEXPECTED_END,
    DUMP_FILE_BROKEN,
};

class PlayerDump
{
    protected:
        PlayerDump() {}
};

class PlayerDumpWriter : public PlayerDump
{
    public:
        PlayerDumpWriter() {}

        std::string GetDump(uint32 guid);
        DumpReturn WriteDump(std::string const& file, uint32 guid);
        DumpReturn ReturnDump(std::string& dump, uint32 guid);
    private:
        typedef std::set<uint32> GUIDs;

        void DumpTableContent(std::string& dump, uint32 guid, char const*tableFrom, char const*tableTo, DumpTableType type);
        std::string GenerateWhereStr(char const* field, GUIDs const& guids, GUIDs::const_iterator& itr);
        std::string GenerateWhereStr(char const* field, uint32 guid);

        GUIDs pets;
        GUIDs mails;
        GUIDs items;
        GUIDs texts;
};

class PlayerDumpReader : public PlayerDump
{
    public:
        PlayerDumpReader() {}

        DumpReturn LoadDump(std::string const& file, uint32 account, std::string name, uint32 guid);
        DumpReturn LoadStringDump(std::string const& data, uint32 account, std::string& name, uint32& guid);
};

#endif
