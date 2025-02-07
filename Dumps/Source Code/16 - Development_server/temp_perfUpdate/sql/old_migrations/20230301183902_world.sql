-- Arnok <First Aid Trainer>
INSERT INTO `gossip_menu` VALUES (4761, 5813, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`) VALUES (4761, 0, 3, 'I require training, Arnok.', 8365, 5, 16, 0, 0, 0, 0, '', 0);
UPDATE `creature_template` SET `gossip_menu_id`=4761 WHERE `entry`=3373;

-- Wrenix's Gizmotronic Apparatus
INSERT INTO `gossip_menu` VALUES (524, 1041, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (1091, 9, 2381, 0, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (524, 0, 0, 'Press the red button labeled \'E.C.A.C.\'', 3166, 1, 1, -1, 0, 52401, 0, 0, '', 0, 1091);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (524, 1, 0, 'Press the yellow button labeled \'Thieves\' Tools.\'', 3149, 1, 1, -1, 0, 52402, 0, 0, '', 0, 1091);
DELETE FROM `gossip_scripts` WHERE `id`=52401;
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (52401, 0, 15, 9977, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Wrenix\'s Gizmotronic Apparatus - Cast Spell Conjure E.C.A.C');
DELETE FROM `gossip_scripts` WHERE `id`=52402;
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (52402, 0, 15, 9949, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Wrenix\'s Gizmotronic Apparatus - Cast Spell Thieves\' Tool Rack Conjure');
UPDATE `creature_template` SET `gossip_menu_id`=524, `script_name`='' WHERE `entry`=7166;

-- Beaten Corpse
INSERT INTO `gossip_menu` VALUES (2881, 3557, 0);
INSERT INTO `gossip_menu` VALUES (2882, 3558, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (4921, 9, 4921, 0, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (2881, 0, 0, 'I inspect the body further.', 5964, 1, 1, 2882, 0, 2881, 0, 0, '', 0, 4921);
DELETE FROM `gossip_scripts` WHERE `id`=2881;
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (2881, 0, 83, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Beaten Corpse - Grant Quest Credit');
UPDATE `creature_template` SET `gossip_menu_id`=2881, `script_name`='' WHERE `entry`=10668;

-- Mor'rogal
INSERT INTO `gossip_menu` VALUES (4721, 5773, 0);
UPDATE `creature_template` SET `gossip_menu_id`=4721 WHERE `entry`=11861;

-- Andruk <Wind Rider Master>
UPDATE `creature_template` SET `gossip_menu_id`=6944, `npc_flags`=9 WHERE `entry`=11901;

-- Kil'Hiwana <Fisherman>
UPDATE `creature_template` SET `gossip_menu_id`=5665 WHERE `entry`=12961;

-- Silverwing Emissary
UPDATE `gossip_menu` SET `entry`=6512 WHERE `entry`=6508;
UPDATE `gossip_menu_option` SET `menu_id`=6512 WHERE `menu_id`=6508;
UPDATE `creature_template` SET `gossip_menu_id`=6512 WHERE `entry`=15102;

-- Booty Bay Reveler
INSERT INTO `gossip_menu` VALUES (6770, 8135, 0);
INSERT INTO `npc_text` (`ID`, `Probability0`, `Probability1`, `Probability2`, `Probability3`, `Probability4`, `Probability5`, `Probability6`, `Probability7`, `BroadcastTextID0`, `BroadcastTextID1`, `BroadcastTextID2`, `BroadcastTextID3`, `BroadcastTextID4`, `BroadcastTextID5`, `BroadcastTextID6`, `BroadcastTextID7`) VALUES (8135, 1, 1, 1, 1, 0, 0, 0, 0, 11460, 11461, 11462, 11463, 0, 0, 0, 0);
UPDATE `creature_template` SET `gossip_menu_id`=6770, `npc_flags`=1 WHERE `entry`=15723;

-- Festival Talespinner
UPDATE `creature_template` SET `gossip_menu_id`=7326 WHERE `entry`=16818;

-- Alexandra Constantine <Gryphon Master>
UPDATE `creature_template` SET `gossip_menu_id`=6944, `npc_flags`=9 WHERE `entry`=8609;

-- Tirion Fordring
INSERT INTO `gossip_menu` VALUES (3502, 4254, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (281, 9, 5742, 0, 0, 0, 0);
INSERT INTO `gossip_menu` VALUES (3502, 4673, 281);
INSERT INTO `gossip_menu` VALUES (3683, 4493, 0);
INSERT INTO `gossip_menu` VALUES (3682, 4494, 0);
INSERT INTO `gossip_menu` VALUES (3681, 4495, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (3681, 0, 0, 'I will, Tirion.', 7225, 1, 1, -1, 0, 3681, 0, 0, '', 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (3682, 0, 0, 'That is terrible.', 7223, 1, 1, 3681, 0, 0, 0, 0, '', 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (3683, 0, 0, 'Thank you, Tirion. What of your identity?', 7221, 1, 1, 3682, 0, 0, 0, 0, '', 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (3502, 0, 0, 'I am ready to hear your tale, Tirion.', 7219, 1, 1, 3683, 0, 0, 0, 0, '', 0, 281);
DELETE FROM `gossip_scripts` WHERE `id`=3681;
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (3681, 0, 7, 5742, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Tirion Fordring - Complete Quest Redemption');
UPDATE `creature_template` SET `gossip_menu_id`=3502, `script_name`='' WHERE `entry`=1855;

-- Borgus Stoutarm <Gryphon Master>
UPDATE `creature_template` SET `gossip_menu_id`=6944, `npc_flags`=9 WHERE `entry`=2299;

-- Arch Druid Fandral Staghelm
INSERT INTO `gossip_menu` VALUES (1668, 2320, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`) VALUES (1630, 0, 0, 'I\'m not a journeyman herbalist -- am I able to still assist you in your work?', 0, 1, 1, 1668, 0, 0, 0, '', 0);
REPLACE INTO `creature_spells` (`entry`, `name`, `spellId_1`, `probability_1`, `castTarget_1`, `targetParam1_1`, `targetParam2_1`, `castFlags_1`, `delayInitialMin_1`, `delayInitialMax_1`, `delayRepeatMin_1`, `delayRepeatMax_1`, `scriptId_1`, `spellId_2`, `probability_2`, `castTarget_2`, `targetParam1_2`, `targetParam2_2`, `castFlags_2`, `delayInitialMin_2`, `delayInitialMax_2`, `delayRepeatMin_2`, `delayRepeatMax_2`, `scriptId_2`, `spellId_3`, `probability_3`, `castTarget_3`, `targetParam1_3`, `targetParam2_3`, `castFlags_3`, `delayInitialMin_3`, `delayInitialMax_3`, `delayRepeatMin_3`, `delayRepeatMax_3`, `scriptId_3`, `spellId_4`, `probability_4`, `castTarget_4`, `targetParam1_4`, `targetParam2_4`, `castFlags_4`, `delayInitialMin_4`, `delayInitialMax_4`, `delayRepeatMin_4`, `delayRepeatMax_4`, `scriptId_4`, `spellId_5`, `probability_5`, `castTarget_5`, `targetParam1_5`, `targetParam2_5`, `castFlags_5`, `delayInitialMin_5`, `delayInitialMax_5`, `delayRepeatMin_5`, `delayRepeatMax_5`, `scriptId_5`, `spellId_6`, `probability_6`, `castTarget_6`, `targetParam1_6`, `targetParam2_6`, `castFlags_6`, `delayInitialMin_6`, `delayInitialMax_6`, `delayRepeatMin_6`, `delayRepeatMax_6`, `scriptId_6`, `spellId_7`, `probability_7`, `castTarget_7`, `targetParam1_7`, `targetParam2_7`, `castFlags_7`, `delayInitialMin_7`, `delayInitialMax_7`, `delayRepeatMin_7`, `delayRepeatMax_7`, `scriptId_7`, `spellId_8`, `probability_8`, `castTarget_8`, `targetParam1_8`, `targetParam2_8`, `castFlags_8`, `delayInitialMin_8`, `delayInitialMax_8`, `delayRepeatMin_8`, `delayRepeatMax_8`, `scriptId_8`) VALUES (35160, 'Darnassus - Arch Druid Fandral Staghelm', 20699, 100, 1, 0, 0, 0, 25, 25, 30, 30, 0, 20701, 100, 15, 0, 10, 0, 5, 5, 15, 15, 0, 20702, 100, 0, 0, 0, 0, 15, 15, 25, 25, 0, 27530, 100, 1, 0, 0, 0, 27, 27, 30, 30, 0, 20698, 100, 1, 0, 0, 136, 0, 2, 2, 2, 0, 20698, 100, 1, 0, 0, 64, 5, 10, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
UPDATE `creature_template` SET `gossip_menu_id`=1630, `spell_list_id`=35160, `script_name`='', `leash_range`=40 WHERE `entry`=3516;

-- Deathstalker Adamant
UPDATE `gossip_menu` SET `entry`=302 WHERE `entry`=21214;
UPDATE `gossip_menu_option` SET `menu_id`=302 WHERE `menu_id`=21214;
UPDATE `gossip_menu_option` SET `action_script_id`=302 WHERE `action_script_id`=21214;
UPDATE `gossip_scripts` SET `id`=302 WHERE `id`=21214;
UPDATE `creature_template` SET `gossip_menu_id`=302 WHERE `entry`=3849;

-- Jandria <Priest Trainer>
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4573, 4438, 94);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4573, 8244, 461);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4573, 8283, 459);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4573, 8285, 454);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4573, 8291, 458);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4573, 8296, 460);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4573, 8298, 455);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4573, 0, 3, 'I seek more training in the priestly ways.', 7157, 5, 16, 0, 0, 0, 0, 0, NULL, 0, 94);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4573, 1, 0, 'I wish to unlearn my talents.', 8271, 1, 1, 4461, 0, 0, 0, 0, NULL, 0, 163);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4573, 2, 0, 'Here, I\'d like to give you this token of my love.', 11723, 1, 1, -1, 0, 410, 0, 0, NULL, 0, 462);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4573, 3, 0, 'Here, I\'d like to give you this token of my love.', 11723, 1, 1, -1, 0, 411, 0, 0, NULL, 0, 461);
UPDATE `creature_template` SET `gossip_menu_id`=4573 WHERE `entry`=4091;

-- Lariia <Priest Trainer>
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4572, 4438, 94);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4572, 8244, 461);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4572, 8283, 459);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4572, 8285, 454);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4572, 8291, 458);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4572, 8296, 460);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (4572, 8298, 455);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4572, 0, 3, 'I seek more training in the priestly ways.', 7157, 5, 16, 0, 0, 0, 0, 0, NULL, 0, 94);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4572, 1, 0, 'I wish to unlearn my talents.', 8271, 1, 1, 4461, 0, 0, 0, 0, NULL, 0, 163);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4572, 2, 0, 'Here, I\'d like to give you this token of my love.', 11723, 1, 1, -1, 0, 410, 0, 0, NULL, 0, 462);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4572, 3, 0, 'Here, I\'d like to give you this token of my love.', 11723, 1, 1, -1, 0, 411, 0, 0, NULL, 0, 461);
UPDATE `creature_template` SET `gossip_menu_id`=4572 WHERE `entry`=4092;

-- Add script id to gossip menus.
ALTER TABLE `gossip_menu`
	ADD COLUMN `script_id` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `text_id`;

-- Aged Kodo
-- Dying Kodo
-- Ancient Kodo
-- Tamed Kodo
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (3650, 4449, 3650, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (18172, 1, 18172, 0, 0, 0, 2);
DELETE FROM `gossip_scripts` WHERE `id`=3650;
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (3650, 0, 83, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 18172, 'Tamed Kodo - Grant Quest Credit');
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (3650, 1, 14, 18172, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Tamed Kodo - Remove Aura Kodo Kombobulator');
UPDATE `creature_template` SET `gossip_menu_id`=3650, `ai_name`='EventAI', `script_name`='' WHERE `entry` IN (4700, 4701, 4702);
UPDATE `creature_template` SET `gossip_menu_id`=3650, `script_name`='' WHERE `entry`=11627;
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (11596, 20, 11596, 10, 0, 1, 2);
UPDATE `spell_template` SET `customFlags`=1024 WHERE `entry`=18377;
-- Events list for Aged Kodo
DELETE FROM `creature_ai_events` WHERE `creature_id`=4700;
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470001, 4700, 0, 11, 0, 100, 1, 0, 0, 0, 0, 470001, 0, 0, 'Aged Kodo - Set Phase to 0 on Respawn');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470002, 4700, 0, 8, 6, 100, 1, 18153, -1, 300000, 300000, 470002, 0, 0, 'Aged Kodo - Follow Player on Spell Hit by Kodo Kombobulator (Phase 0)');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470003, 4700, 11596, 1, 5, 100, 1, 3000, 3000, 3000, 3000, 470003, 0, 0, 'Aged Kodo - Check for Smeed while OOC (Phase 1)');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470004, 4700, 0, 8, 1, 100, 1, 18362, -1, 300000, 300000, 470004, 0, 0, 'Aged Kodo - Enable Gossip on Hit By Spell Kodo Kombobulator');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470005, 4700, 0, 8, 0, 100, 1, 18269, -1, 300000, 300000, 470005, 0, 0, 'Aged Kodo - Despawn on Hit by Spell Kodo Kombobulator');
-- Events list for Dying Kodo
DELETE FROM `creature_ai_events` WHERE `creature_id`=4701;
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470101, 4701, 0, 11, 0, 100, 1, 0, 0, 0, 0, 470001, 0, 0, 'Dying Kodo - Set Phase to 0 on Respawn');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470102, 4701, 0, 8, 6, 100, 1, 18153, -1, 300000, 300000, 470002, 0, 0, 'Dying Kodo - Follow Player on Spell Hit by Kodo Kombobulator (Phase 0)');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470103, 4701, 11596, 1, 5, 100, 1, 3000, 3000, 3000, 3000, 470003, 0, 0, 'Dying Kodo - Check for Smeed while OOC (Phase 1)');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470104, 4701, 0, 8, 1, 100, 1, 18362, -1, 300000, 300000, 470004, 0, 0, 'Dying Kodo - Enable Gossip on Hit By Spell Kodo Kombobulator');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470105, 4701, 0, 8, 0, 100, 1, 18269, -1, 300000, 300000, 470005, 0, 0, 'Dying Kodo - Despawn on Hit by Spell Kodo Kombobulator');
-- Events list for Ancient Kodo
DELETE FROM `creature_ai_events` WHERE `creature_id`=4702;
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470201, 4702, 0, 11, 0, 100, 1, 0, 0, 0, 0, 470001, 0, 0, 'Ancient Kodo - Set Phase to 0 on Respawn');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470202, 4702, 0, 8, 6, 100, 1, 18153, -1, 300000, 300000, 470002, 0, 0, 'Ancient Kodo - Follow Player on Spell Hit by Kodo Kombobulator (Phase 0)');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470203, 4702, 11596, 1, 5, 100, 1, 3000, 3000, 3000, 3000, 470003, 0, 0, 'Ancient Kodo - Check for Smeed while OOC (Phase 1)');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470204, 4702, 0, 8, 1, 100, 1, 18362, -1, 300000, 300000, 470004, 0, 0, 'Ancient Kodo - Enable Gossip on Hit By Spell Kodo Kombobulator');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (470205, 4702, 0, 8, 0, 100, 1, 18269, -1, 300000, 300000, 470005, 0, 0, 'Ancient Kodo - Despawn on Hit by Spell Kodo Kombobulator');
DELETE FROM `creature_ai_scripts` WHERE `id`=470001;
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470001, 0, 44, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Set Phase to 0');
DELETE FROM `creature_ai_scripts` WHERE `id`=470002;
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470002, 0, 73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Combat Stop');
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470002, 0, 27, 11627, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Update Entry to Tamed Kodo');
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470002, 0, 15, 18172, 2, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Cast Spell Kodo Kombobulator (player debuff)');
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470002, 0, 20, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1.57, 0, 'Aged Kodo - Follow Player');
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470002, 0, 44, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Set Phase to 1');
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470002, 0, 15, 18377, 2, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Cast Spell Kodo Kombobulator (5 minute despawn aura)');
DELETE FROM `creature_ai_scripts` WHERE `id`=470003;
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470003, 0, 15, 18362, 2, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Cast Spell Kodo Kombobulator');
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470003, 0, 0, 0, 0, 0, 0, 11596, 20, 8, 2, 7301, 7302, 7303, 0, 0, 0, 0, 0, 0, 'Smeed - Say Text');
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470003, 0, 44, 2, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Set Phase to 2');
DELETE FROM `creature_ai_scripts` WHERE `id`=470004;
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470004, 0, 20, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Move Idle');
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470004, 0, 4, 147, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Add Gossip Flag');
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470004, 0, 18, 60000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Despawn in 1 minute');
DELETE FROM `creature_ai_scripts` WHERE `id`=470005;
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (470005, 0, 18, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Aged Kodo - Despawn');

-- Remove cooldown from Kodo Kombobulator events.
UPDATE `creature_ai_events` SET `event_param3`=1, `event_param4`=1 WHERE `id` IN (470002, 470004, 470005, 470102, 470104, 470105, 470202, 470204, 470205);

-- Darnath Bladesinger <Warrior Trainer>
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (4579, 1217, 0, 98);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (4579, 5725, 0, 99);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (4579, 8244, 0, 461);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (4579, 8283, 0, 459);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (4579, 8285, 0, 454);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (4579, 8291, 0, 458);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (4579, 8296, 0, 460);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (4579, 8298, 0, 455);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4579, 0, 3, 'I require warrior training.', 3147, 5, 16, 0, 0, 0, 0, 0, '', 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4579, 1, 0, 'I wish to unlearn my talents.', 8271, 1, 1, 4461, 0, 0, 0, 0, '', 0, 133);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4579, 2, 0, 'Here, I\'d like to give you this token of my love.', 11723, 1, 1, -1, 0, 410, 0, 0, '', 0, 462);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (4579, 3, 0, 'Here, I\'d like to give you this token of my love.', 11723, 1, 1, -1, 0, 411, 0, 0, '', 0, 461);
UPDATE `creature_template` SET `gossip_menu_id`=4579 WHERE `entry`=7315;

-- Innkeeper Jayka <Innkeeper>
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (1582, 938, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1582, 0, 0, 'Trick or Treat!', 10693, 1, 1, -1, 0, 342, 0, 0, NULL, 0, 164);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1582, 1, 5, 'Make this inn your home.', 2822, 8, 128, 0, 0, 0, 0, 0, '', 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1582, 2, 0, 'Does that heart mean you\'re looking for love?', 11823, 1, 1, 5110, 0, 0, 0, 0, '', 0, 440);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1582, 3, 1, 'Let me browse your seasonal fare.', 8786, 3, 4, 2, 0, 0, 0, 0, '', 0, 440);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1582, 4, 1, 'I want to browse your goods.', 3370, 3, 4, 1, 0, 0, 0, 0, '', 0, 0);
UPDATE `creature_template` SET `gossip_menu_id`=1582 WHERE `entry`=7731;

-- Disciple of Naralex
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (3678, 34, 4, 4, 0, 0, 0);
DELETE FROM `script_texts` WHERE `entry` IN (-1614999, -1614981);
UPDATE `broadcast_text` SET `chat_type`=6 WHERE `entry`=2101;
UPDATE `broadcast_text` SET `chat_type`=6 WHERE `entry`=2102;
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES
(201, 698, 201, 0),
(202, 699, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (202, 0, 0, 'Let the event begin!', 2662, 1, 1, -1, 0, 202, 0, 0, NULL, 0, 3678);
DELETE FROM `gossip_scripts` WHERE `id`=201;
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (201, 0, 15, 5232, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Disciple of Naralex - Cast Mark of the Wild');
DELETE FROM `gossip_scripts` WHERE `id`=202;
INSERT INTO `gossip_scripts` (`id`, `command`, `comments`) VALUES (202, 85, 'Disciple of Naralex - Start Escort');
UPDATE `creature_template` SET `gossip_menu_id`=201 WHERE `entry`=3678;

-- Stone Watcher of Norgannon
DELETE FROM `creature_ai_events` WHERE `creature_id`=7918;
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (791801, 7918, 0, 11, 0, 100, 0, 0, 0, 0, 0, 791801, 0, 0, 'Stone Watcher of Norgannon - Say Text on Spawn');
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (791802, 7918, 0, 1, 0, 100, 0, 300000, 300000, 0, 0, 791802, 0, 0, 'Stone Watcher of Norgannon - Say Text OOC');
DELETE FROM `creature_ai_scripts` WHERE `id`=791801;
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (791801, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4049, 0, 0, 0, 0, 0, 0, 0, 0, 'Stone Watcher of Norgannon - Set Text');
DELETE FROM `creature_ai_scripts` WHERE `id`=791802;
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (791802, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4048, 0, 0, 0, 0, 0, 0, 0, 0, 'Stone Watcher of Norgannon - Say Text');
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1061, 1674);
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1062, 1676);
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1063, 1675);
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1064, 1677);
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1065, 1678);
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1066, 1679);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1061, 0, 0, 'What function do you serve?', 4054, 1, 1, 1062, 0, 0, 0, 0, NULL, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1062, 0, 0, 'What are the Plates of Uldum?', 4056, 1, 1, 1063, 0, 0, 0, 0, NULL, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1063, 0, 0, 'Where are the Plates of Uldum?', 4057, 1, 1, 1064, 0, 0, 0, 0, NULL, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1064, 0, 0, 'Excuse me?  We\'ve been "rescheduled for visitation"?  What does that mean?!', 4058, 1, 1, 1065, 0, 0, 0, 0, NULL, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1065, 0, 0, 'So... what\'s inside Uldum?', 4059, 1, 1, 1066, 0, 0, 0, 0, NULL, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1066, 0, 0, 'I will return when I have the Plates of Uldum.', 4060, 1, 1, -1, 0, 1066, 0, 0, NULL, 0, 0);
DELETE FROM `gossip_scripts` WHERE `id`=1066;
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (1066, 0, 7, 2954, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Stone Watcher of Norgannon - Complete Quest');
DELETE FROM `quest_end_scripts` WHERE `id`=2946;
INSERT INTO `quest_end_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (2946, 5, 10, 7918, 900000, 1, 30, 0, 0, 0, 0, 8, 0, 6, 1, -9619, -2815, 11.21, 0.43, 0, 'Seeing What Happens - Summon Stone Watcher of Norgannon');
DELETE FROM `quest_end_scripts` WHERE `id`=2966;
INSERT INTO `quest_end_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (2966, 5, 10, 7918, 900000, 1, 30, 0, 0, 0, 0, 8, 0, 6, 1, -9619, -2815, 11.21, 0.43, 0, 'Seeing What Happens - Summon Stone Watcher of Norgannon');
UPDATE `creature_template` SET `gossip_menu_id`=1061, `ai_name`='EventAI', `script_name`='' WHERE `entry`=7918;

-- Royal Historian Archesonus
-- INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1561, 2234);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `condition_id`) VALUES (1561, 2235, 8879);
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1562, 2238);
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1563, 2239);
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1564, 2237);
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (1565, 2236);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1561, 0, 0, 'I am ready, Historian Archesonus.', 4603, 1, 1, 1565, 0, 0, 0, 0, NULL, 0, 8879);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1562, 0, 0, 'Unbelievable! How dare they??', 4609, 1, 1, 1563, 0, 0, 0, 0, NULL, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1563, 0, 0, 'Of course I will help!', 4611, 1, 1, -1, 0, 1563, 0, 0, NULL, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1564, 0, 0, 'Interesting, continue please.', 4607, 1, 1, 1562, 0, 0, 0, 0, NULL, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (1565, 0, 0, 'That is tragic. How did this happen?', 4605, 1, 1, 1564, 0, 0, 0, 0, NULL, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (8879, 9, 3702, 1, 0, 0, 0);
DELETE FROM `gossip_scripts` WHERE `id`=1563;
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (1563, 0, 7, 3702, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Royal Historian Archesonus - Complete Quest');
UPDATE `creature_template` SET `gossip_menu_id`=1561, `script_name`='' WHERE `entry`=8879;

-- Gossip for Journal of Jandice Barov.
-- ERROR: Gameobject (Entry: 180794) has gossip_menu_id = 6799 for nonexistent menu
UPDATE `npc_text` SET `BroadcastTextID0`=11406 WHERE `ID`=8121;
INSERT INTO `npc_text` (`ID`, `BroadcastTextID0`, `Probability0`) VALUES (8122, 11410, 1);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6799, 8120, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (71, 7, 197, 280, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (73, 17, 26086, 1, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (348, -1, 71, 73, 0, 0, 0);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6799, 8121, 0, 348);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (349, 17, 26086, 0, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (350, -1, 71, 349, 0, 0, 0);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6799, 8122, 0, 350);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6799, 0, 0, '<Copy the pattern into my journal.>.', 11407, 1, 1, -1, 0, 6799, 0, 0, '', 0, 348);
DELETE FROM `gossip_scripts` WHERE `id`=6799;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(6799, 0, 0, 15, 26095, 2, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Journal of Jandice Barov - Cast Spell Felcloth Bag');
UPDATE `gameobject_template` SET `script_name`='' WHERE `entry`=180794;

-- Gossipt for Lesser Wind Stone.
-- ERROR: Gameobject (Entry: 180456) has gossip_menu_id = 6540 for nonexistent menu
-- ERROR: Gameobject (Entry: 180518) has gossip_menu_id = 6540 for nonexistent menu
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (887, 1, 24746, 0, 0, 0, 1);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6540, 7744, 6540, 887);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (888, 1, 24746, 0, 0, 0, 0);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6540, 7771, 0, 888);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6540, 0, 0, 'I am no cultist, you monster!  Come to me and face your destruction!', 10684, 1, 1, -1, 0, 654001, 0, 0, '', 0, 888);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (895, 2, 20416, 1, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (899, -1, 888, 895, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6540, 1, 0, 'Crimson Templar!  I hold your crest of beckoning.  Heed my summons!', 10685, 1, 1, -1, 0, 654002, 0, 0, '', 0, 899);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (896, 2, 20419, 1, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (903, -1, 888, 896, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6540, 2, 0, 'Earthen Templar!  I hold your crest of beckoning.  Heed my summons!', 10691, 1, 1, -1, 0, 654003, 0, 0, '', 0, 903);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (897, 2, 20418, 1, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (901, -1, 888, 897, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6540, 3, 0, 'Hoary Templar!  I hold your crest of beckoning.  Heed my summons!', 10690, 1, 1, -1, 0, 654004, 0, 0, '', 0, 901);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (898, 2, 20420, 1, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (902, -1, 888, 898, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6540, 4, 0, 'Azure Templar!  I hold your crest of beckoning.  Heed my summons!', 10692, 1, 1, -1, 0, 654005, 0, 0, '', 0, 902);
DELETE FROM `gossip_scripts` WHERE `id`=6540;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(6540, 0, 0, 13, 0, 0, 0, 0, 180502, 10, 11, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Lesser Wind Stone - Activate Wind Stone Trap');
DELETE FROM `gossip_scripts` WHERE `id`=654001;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(654001, 0, 0, 15, 24745, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Lesser Wind Stone - Cast Spell Summon Templar, Trigger');
DELETE FROM `gossip_scripts` WHERE `id`=654002;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(654002, 0, 0, 15, 24747, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Lesser Wind Stone - Cast Spell Summon Templar Fire, Trigger');
DELETE FROM `gossip_scripts` WHERE `id`=654003;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(654003, 0, 0, 15, 24759, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Lesser Wind Stone - Cast Spell Summon Templar Earth, Trigger');
DELETE FROM `gossip_scripts` WHERE `id`=654004;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(654004, 0, 0, 15, 24757, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Lesser Wind Stone - Cast Spell Summon Templar Air, Trigger');
DELETE FROM `gossip_scripts` WHERE `id`=654005;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(654005, 0, 0, 15, 24761, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Lesser Wind Stone - Cast Spell Summon Templar Water, Trigger');

-- Gossip menu for Greater Wind Stone.
-- ERROR: Gameobject (Entry: 180466) has gossip_menu_id = 6543 for nonexistent menu
INSERT INTO `npc_text` (`ID`, `BroadcastTextID0`, `Probability0`) VALUES (7775, 10749, 1);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6543, 7754, 6540, 887);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (891, 1, 24748, 0, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (892, -1, 888, 891, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (893, 1, 24782, 0, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (894, -1, 892, 893, 0, 0, 0);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6543, 7774, 0, 894);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6543, 7775, 0, 892);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6543, 7776, 0, 888);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6543, 0, 0, 'The day of judgement has come, fiend!  I challenge you to battle!', 10707, 1, 1, -1, 0, 654301, 0, 0, '', 0, 894);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (911, 2, 20447, 1, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (915, -1, 894, 911, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6543, 1, 0, 'Skaldrenox!  I hold your scepter, and charge you to enter this world!', 10708, 1, 1, -1, 0, 654302, 0, 0, '', 0, 915);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (912, 2, 20449, 1, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (916, -1, 894, 912, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6543, 2, 0, 'Kazum!  I hold your scepter!  I command you to enter this world!', 10710, 1, 1, -1, 0, 654303, 0, 0, '', 0, 916);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (913, 2, 20448, 1, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (917, -1, 894, 913, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6543, 3, 0, 'Whirlaxis!  I hold your scepter!  I command you to enter this world!', 10709, 1, 1, -1, 0, 654304, 0, 0, '', 0, 917);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (914, 2, 20450, 1, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (918, -1, 894, 914, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6543, 4, 0, 'Skwol!  I hold your scepter!  I command you to enter this world!', 10711, 1, 1, -1, 0, 654305, 0, 0, '', 0, 918);
DELETE FROM `gossip_scripts` WHERE `id`=654301;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(654301, 0, 0, 15, 24785, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Greater Wind Stone - Cast Spell Summon Royal, Trigger');
DELETE FROM `gossip_scripts` WHERE `id`=654302;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(654302, 0, 0, 15, 24787, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Greater Wind Stone - Cast Spell Summon Royal Fire, Trigger');
DELETE FROM `gossip_scripts` WHERE `id`=654303;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(654303, 0, 0, 15, 24792, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Greater Wind Stone - Cast Spell Summon Royal Earth, Trigger');
DELETE FROM `gossip_scripts` WHERE `id`=654304;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(654304, 0, 0, 15, 24791, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Greater Wind Stone - Cast Spell Summon Royal Air, Trigger');
DELETE FROM `gossip_scripts` WHERE `id`=654305;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(654305, 0, 0, 15, 24793, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Greater Wind Stone - Cast Spell Summon Royal Water, Trigger');

-- Gossip menu for Tablet of Madness.
-- ERROR: Gameobject (Entry: 180368) has gossip_menu_id = 6443 for nonexistent menu
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6443, 7635, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (351, 7, 171, 300, 0, 0, 0);
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (6443, 7636, 0, 351);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (352, 17, 24266, 1, 0, 0, 0);
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (353, -1, 351, 352, 0, 0, 0);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_broadcast_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `box_broadcast_text`, `condition_id`) VALUES (6443, 0, 0, 'Learn the recipe.', 10486, 1, 1, -1, 0, 6443, 0, 0, '', 0, 353);
DELETE FROM `gossip_scripts` WHERE `id`=6443;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(6443, 0, 0, 15, 24267, 2, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Tablet of Madness - Cast Spell Gurubashi Mojo Madness');
UPDATE `gameobject_template` SET `flags`=0, `script_name`='' WHERE `entry`=180368;

-- Gossip menu for Call to Arms!
-- ERROR: Gameobject (Entry: 179913) has gossip_menu_id = 6121 for nonexistent menu
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES (6121, 7274);

-- Gossip menu for Sha'ni Proudtusk's Remains.
-- ERROR: Gameobject (Entry: 160445) has gossip_menu_id = 2211 for nonexistent menu
INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (2211, 2850, 2211, 0);
DELETE FROM `gossip_scripts` WHERE `id`=2211;
INSERT INTO `gossip_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES
(2211, 0, 0, 10, 9136, 60000, 1, 100, 0, 0, 0, 0, 8, 0, -1, 1, -7917.15, -2610.4, 221.206, 1.95477, 3821, 'Sha\'ni Proudtusk\'s Remains - Summon Creature Sha\'ni Proudtusk'),
(2211, 0, 1, 7, 3821, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Sha\'ni Proudtusk\'s Remains - Complete Quest');
-- 3821: Target Has Incomplete Quest 3821 In Log
INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (3821, 9, 3821, 1, 0, 0, 0);
DELETE FROM `gameobject_involvedrelation` WHERE `quest`=3821;
UPDATE `creature_template` SET `faction`=876, `unit_flags`=768 WHERE `entry`=9136;
UPDATE `gameobject_template` SET `flags`=0, `script_name`='' WHERE `entry`=160445;

-- Darrowshire Spirit
DELETE FROM `creature_ai_events` WHERE `creature_id`=11064;
INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (1106401, 11064, 0, 1, 0, 100, 0, 0, 0, 0, 0, 1106401, 0, 0, 'Darrowshire Spirit - Cast Spell on Spawn');
DELETE FROM `creature_ai_scripts` WHERE `id`=1106401;
INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (1106401, 0, 15, 17321, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Darrowshire Spirit - Cast Spell Spirit Spawn-in');
UPDATE `gossip_menu` SET `script_id`=3141 WHERE `entry`=3141;
DELETE FROM `gossip_scripts` WHERE `id`=3141;
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (3141, 0, 83, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Darrowshire Spirit - Grant Quest Credit');
INSERT INTO `gossip_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (3141, 0, 18, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Darrowshire Spirit - Despawn');
UPDATE `creature_template` SET `gossip_menu_id`=3141, `script_name`='' WHERE `entry`=11064;
