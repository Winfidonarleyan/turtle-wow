
-- Changes by GHEOR
REPLACE INTO `creature` VALUES (2579070,61650,0,0,0,1,2426.41,-2478.66,108.937,4.22854,120,120,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2579071,61651,0,0,0,1,2440.13,-2512.93,110.25,1.50926,120,120,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2579071;
DELETE FROM creature_addon WHERE guid=2579071;
DELETE FROM creature_movement WHERE id=2579071;
DELETE FROM game_event_creature WHERE guid=2579071;
DELETE FROM game_event_creature_data WHERE guid=2579071;
DELETE FROM creature_battleground WHERE guid=2579071;
REPLACE INTO `creature` VALUES (2579072,61651,0,0,0,1,2440.08,-2510.32,110.392,1.11187,120,120,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2579073,61652,0,0,0,1,2488.91,-2508.35,109.94,6.06228,120,120,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2579074,61653,0,0,0,1,2527.66,-2520.77,108.641,1.51404,120,120,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2579075,61655,0,0,0,1,2525.76,-2498.49,108.418,5.87143,120,120,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2579076,61654,0,0,0,1,2453.75,-2467.25,109.798,6.0018,120,120,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2579077,60607,0,0,0,1,2526.26,-2516.04,109.171,3.7375,120,120,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2579077;
DELETE FROM creature_addon WHERE guid=2579077;
DELETE FROM creature_movement WHERE id=2579077;
DELETE FROM game_event_creature WHERE guid=2579077;
DELETE FROM game_event_creature_data WHERE guid=2579077;
DELETE FROM creature_battleground WHERE guid=2579077;
REPLACE INTO `creature` VALUES (2579078,61645,0,0,0,1,-6318.09,-4920.58,13.5645,2.36325,120,120,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2579079,61646,0,0,0,1,-6323.96,-4919.81,13.3448,2.86905,120,120,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2579079;
DELETE FROM creature_addon WHERE guid=2579079;
DELETE FROM creature_movement WHERE id=2579079;
DELETE FROM game_event_creature WHERE guid=2579079;
DELETE FROM game_event_creature_data WHERE guid=2579079;
DELETE FROM creature_battleground WHERE guid=2579079;
DELETE FROM creature WHERE guid=2579069;
DELETE FROM creature_addon WHERE guid=2579069;
DELETE FROM creature_movement WHERE id=2579069;
DELETE FROM game_event_creature WHERE guid=2579069;
DELETE FROM game_event_creature_data WHERE guid=2579069;
DELETE FROM creature_battleground WHERE guid=2579069;
REPLACE INTO `creature` VALUES (2579080,61646,0,0,0,1,-6369.12,-4920.62,20.0002,0.109942,120,120,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2579081,61647,0,0,0,1,-6357.85,-4916.76,18.1162,0.245816,120,120,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2579081;
DELETE FROM creature_addon WHERE guid=2579081;
DELETE FROM creature_movement WHERE id=2579081;
DELETE FROM game_event_creature WHERE guid=2579081;
DELETE FROM game_event_creature_data WHERE guid=2579081;
DELETE FROM creature_battleground WHERE guid=2579081;
REPLACE INTO `creature` VALUES (2579082,61648,0,0,0,1,-6362.5,-4916.94,12.8644,5.41295,120,120,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2579082;
DELETE FROM creature_addon WHERE guid=2579082;
DELETE FROM creature_movement WHERE id=2579082;
DELETE FROM game_event_creature WHERE guid=2579082;
DELETE FROM game_event_creature_data WHERE guid=2579082;
DELETE FROM creature_battleground WHERE guid=2579082;
REPLACE INTO `creature` VALUES (2579083,61647,0,0,0,1,-6360.47,-4916.67,7.80291,4.10369,120,120,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2579084,61649,0,0,0,1,-6370.04,-4924.33,20.0942,0.873347,120,120,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2579085,61648,0,0,0,1,-6357.84,-4924.45,0.489248,1.00294,120,120,0,100,100,0,0,0);

-- Changes by JOE
REPLACE INTO `gameobject` VALUES ( 5016774, 31445, 0, 1966.3, 2831.79, 3.62081, 3.13233, 0, 0, 0.999989, 0.00463182, 300, 300, 100, 1, 0, 0);
REPLACE INTO `gameobject` VALUES ( 5016775, 31445, 0, 1965.51, 2831.89, 8.38719, 3.12025, 0, 0, 0.999943, 0.0106705, 300, 300, 100, 1, 0, 0);

-- Changes by VOJI
UPDATE `creature` SET `spawntimesecsmin`=120, `spawntimesecsmax`=120 WHERE `guid`=2577406;
