
-- Changes by DRAGUNOVI
REPLACE INTO `creature` VALUES (2577927,12236,0,0,0,1,-5336.11,-252.861,41.1511,5.00943,120,120,5,100,100,1,1,200);
DELETE FROM creature WHERE guid=2577927;
DELETE FROM creature_addon WHERE guid=2577927;
DELETE FROM creature_movement WHERE id=2577927;
DELETE FROM game_event_creature WHERE guid=2577927;
DELETE FROM game_event_creature_data WHERE guid=2577927;
DELETE FROM creature_battleground WHERE guid=2577927;

-- Changes by VOJI
REPLACE INTO `creature` VALUES (2577928,61558,0,0,0,0,-1457.43,1719.22,94.8886,1.1366,120,120,0,100,100,0,0,0);
