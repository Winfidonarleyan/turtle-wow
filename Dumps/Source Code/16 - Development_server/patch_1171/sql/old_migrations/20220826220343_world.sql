
-- Changes by SHANG
REPLACE INTO `creature` VALUES (2572202,60897,0,0,0,1,-1621.16,-4921.6,15.3782,1.93596,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572204,60941,0,0,0,1,-3665.48,-3458.77,38.2179,5.2296,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572204,60941,0,0,0,1,-3665.48,-3458.77,38.2179,5.2296,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572204,60941,0,0,0,1,-3665.48,-3458.77,38.2179,5.2296,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572205,60939,0,0,0,1,-3646.18,-3422.61,37.1957,2.1246,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572206,60940,0,0,0,1,-3644.41,-3418.13,37.1918,3.42533,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572207,60939,0,0,0,1,-3634.31,-3424.43,37.1967,0.47144,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572207,60939,0,0,0,1,-3634.31,-3424.43,37.1967,0.47144,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572208,60940,0,0,0,1,-3652.06,-3445.82,37.1806,4.69609,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572208,60940,0,0,0,1,-3652.06,-3445.82,37.1806,4.69609,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572209,60940,0,0,0,1,-3667.88,-3386.71,36.6192,1.68173,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572210,60939,0,0,0,1,-3685.89,-3388.99,37.2034,2.01002,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572211,60939,0,0,0,1,-3696.69,-3401.41,37.1866,4.03704,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572211,60939,0,0,0,1,-3696.69,-3401.41,37.1866,4.03704,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572212,60940,0,0,0,1,-3716.72,-3416.8,36.5954,4.35827,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572212,60940,0,0,0,1,-3716.72,-3416.8,36.5954,4.35827,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572213,60939,0,0,0,1,-3715.61,-3444.42,36.9414,5.69423,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572213,60939,0,0,0,1,-3715.61,-3444.42,36.9414,5.69423,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572214,60940,0,0,0,1,-3673,-3475.87,39.5863,5.48061,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572214,60940,0,0,0,1,-3673,-3475.87,39.5863,5.48061,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572215,60939,0,0,0,1,-3710.28,-3469.47,37.2867,1.96263,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572215,60939,0,0,0,1,-3710.28,-3469.47,37.2867,1.96263,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572216,60940,0,0,0,1,-3696.54,-3468.4,38.194,4.47119,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572217,60939,0,0,0,1,-3699.6,-3461.12,38.1971,5.53226,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572218,60939,0,0,0,1,-3693.18,-3437.96,37.4727,2.36868,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572218,60939,0,0,0,1,-3693.18,-3437.96,37.4727,2.36868,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572219,60940,0,0,0,1,-3674.58,-3419.86,37.3402,0.771963,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572219,60940,0,0,0,1,-3674.58,-3419.86,37.3402,0.771963,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572220,60939,0,0,0,1,-3737.12,-3455.18,40.6902,5.26694,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572220,60939,0,0,0,1,-3737.12,-3455.18,40.6902,5.26694,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572221,60940,0,0,0,1,-3762.25,-3454.33,38.6336,2.24315,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572221,60940,0,0,0,1,-3762.25,-3454.33,38.6336,2.24315,300,300,5,100,100,1,0,0);
DELETE FROM creature WHERE guid=2572093;
DELETE FROM creature_addon WHERE guid=2572093;
DELETE FROM creature_movement WHERE id=2572093;
DELETE FROM game_event_creature WHERE guid=2572093;
DELETE FROM game_event_creature_data WHERE guid=2572093;
DELETE FROM creature_battleground WHERE guid=2572093;
DELETE FROM creature WHERE guid=2572092;
DELETE FROM creature_addon WHERE guid=2572092;
DELETE FROM creature_movement WHERE id=2572092;
DELETE FROM game_event_creature WHERE guid=2572092;
DELETE FROM game_event_creature_data WHERE guid=2572092;
DELETE FROM creature_battleground WHERE guid=2572092;
UPDATE creature SET position_x = '-3817.568604', position_y = '-3403.351318', position_z = '38.359112', orientation = '3.369416' WHERE guid = '2572091';
DELETE FROM creature WHERE guid=73955;
DELETE FROM creature_addon WHERE guid=73955;
DELETE FROM creature_movement WHERE id=73955;
DELETE FROM game_event_creature WHERE guid=73955;
DELETE FROM game_event_creature_data WHERE guid=73955;
DELETE FROM creature_battleground WHERE guid=73955;
REPLACE INTO `creature` VALUES (2572222,60939,0,0,0,1,-3809.69,-3387.49,39.2141,1.57164,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572222,60939,0,0,0,1,-3809.69,-3387.49,39.2141,1.57164,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572223,60940,0,0,0,1,-3780.63,-3405.6,37.6576,5.49627,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572223,60940,0,0,0,1,-3780.63,-3405.6,37.6576,5.49627,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572224,60940,0,0,0,1,-3749.3,-3431.97,39.1981,2.32169,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572225,60940,0,0,0,1,-3741.75,-3419.77,37.1167,3.18462,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572226,60939,0,0,0,1,-3738.19,-3424.58,36.9649,2.9757,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572226,60939,0,0,0,1,-3738.19,-3424.58,36.9649,2.9757,300,300,0,100,100,0,0,0);
DELETE FROM creature_movement WHERE id=2572226;
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,1, -3738.194824,-3424.584473,36.964870, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,2, -3752.210449,-3421.621338,37.546917, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,3, -3760.068604,-3420.402588,37.894867, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,4, -3774.885010,-3421.285889,37.889847, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,5, -3797.177002,-3422.188721,37.966866, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,6, -3813.009766,-3418.528076,37.746044, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,7, -3831.882812,-3411.346436,37.840950, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,8, -3836.944336,-3411.297119,37.784588, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,9, -3824.238770,-3414.585693,37.900509, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,10, -3810.607910,-3420.588379,37.799072, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,11, -3793.813721,-3422.417480,37.963161, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,12, -3759.096924,-3420.613525,37.886978, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572226,13, -3746.650635,-3423.513184,37.291138, 100);
REPLACE INTO `creature` VALUES (2572226,60939,0,0,0,1,-3738.19,-3424.58,36.9649,2.9757,300,300,0,100,100,2,0,0);
REPLACE INTO `creature` VALUES (2572227,60939,0,0,0,1,-3626.49,-3375.36,41.9308,5.9288,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572227,60939,0,0,0,1,-3626.49,-3375.36,41.9308,5.9288,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572228,60940,0,0,0,1,-3607.28,-3401.09,37.0408,4.74207,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572228,60940,0,0,0,1,-3607.28,-3401.09,37.0408,4.74207,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572229,60939,0,0,0,1,-3651.06,-3418.86,37.1505,6.11965,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572230,60939,0,0,0,1,-3658.49,-3396.33,37.1829,3.14928,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572231,60946,0,0,0,0,-8832.39,635.773,94.3848,2.28096,25,25,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2572231;
DELETE FROM creature_addon WHERE guid=2572231;
DELETE FROM creature_movement WHERE id=2572231;
DELETE FROM game_event_creature WHERE guid=2572231;
DELETE FROM game_event_creature_data WHERE guid=2572231;
DELETE FROM creature_battleground WHERE guid=2572231;
REPLACE INTO `creature` VALUES (2572232,60946,0,0,0,0,182.106,-4723.19,8.92936,0.799677,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572233,60946,0,0,0,0,-18.6592,-4692.35,7.40162,3.31531,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572233,60946,0,0,0,0,-18.6592,-4692.35,7.40162,3.31531,300,300,5,100,100,1,0,0);
DELETE FROM creature WHERE guid=2572232;
DELETE FROM creature_addon WHERE guid=2572232;
DELETE FROM creature_movement WHERE id=2572232;
DELETE FROM game_event_creature WHERE guid=2572232;
DELETE FROM game_event_creature_data WHERE guid=2572232;
DELETE FROM creature_battleground WHERE guid=2572232;
REPLACE INTO `gameobject` VALUES ( 5009650, 2010918, 0, 213.529, -4715.42, 1.85234, 5.5342, 0, 0, 0.3658, -0.930693, 300, 300, 100, 1, 0, 0);
REPLACE INTO `gameobject` VALUES ( 5009651, 2010919, 0, 215.335, -4714.39, 1.62705, 5.27266, 0, 0, 0.484037, -0.875048, 300, 300, 100, 1, 0, 0);
DELETE FROM gameobject WHERE guid = '5009650';
DELETE FROM game_event_gameobject WHERE guid = '5009650';
DELETE FROM gameobject_battleground WHERE guid = '5009650';
DELETE FROM gameobject WHERE guid = '5009651';
DELETE FROM game_event_gameobject WHERE guid = '5009651';
DELETE FROM gameobject_battleground WHERE guid = '5009651';
UPDATE creature SET position_x = '-140.532303', position_y = '-4641.928223', position_z = '5.570496', orientation = '1.433632' WHERE guid = '2572233';
UPDATE `creature_template` set `scale` = 1.000000 where entry = 14738;
REPLACE INTO `creature` VALUES (2572234,60942,0,0,0,1,-2644.92,-3331.35,33.2828,0.95488,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572235,60942,0,0,0,1,-2647.09,-3329.8,33.3079,0.91561,25,25,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2572235;
DELETE FROM creature_addon WHERE guid=2572235;
DELETE FROM creature_movement WHERE id=2572235;
DELETE FROM game_event_creature WHERE guid=2572235;
DELETE FROM game_event_creature_data WHERE guid=2572235;
DELETE FROM creature_battleground WHERE guid=2572235;
REPLACE INTO `creature` VALUES (2572236,60943,0,0,0,1,-2647.23,-3329.76,33.3064,0.966661,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572237,60943,0,0,0,1,-2648.34,-3331.38,33.2736,0.966661,25,25,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2572237;
DELETE FROM creature_addon WHERE guid=2572237;
DELETE FROM creature_movement WHERE id=2572237;
DELETE FROM game_event_creature WHERE guid=2572237;
DELETE FROM game_event_creature_data WHERE guid=2572237;
DELETE FROM creature_battleground WHERE guid=2572237;
DELETE FROM creature WHERE guid=2572236;
DELETE FROM creature_addon WHERE guid=2572236;
DELETE FROM creature_movement WHERE id=2572236;
DELETE FROM game_event_creature WHERE guid=2572236;
DELETE FROM game_event_creature_data WHERE guid=2572236;
DELETE FROM creature_battleground WHERE guid=2572236;
DELETE FROM creature WHERE guid=2572234;
DELETE FROM creature_addon WHERE guid=2572234;
DELETE FROM creature_movement WHERE id=2572234;
DELETE FROM game_event_creature WHERE guid=2572234;
DELETE FROM game_event_creature_data WHERE guid=2572234;
DELETE FROM creature_battleground WHERE guid=2572234;
REPLACE INTO `creature` VALUES (2572238,60943,0,0,0,1,-2579.47,-3237.68,33.974,4.16288,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572238,60943,0,0,0,1,-2579.47,-3237.68,33.974,4.16288,300,300,4,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572239,60944,0,0,0,1,-2555.15,-3220.47,34.7723,0.181699,25,25,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2572239;
DELETE FROM creature_addon WHERE guid=2572239;
DELETE FROM creature_movement WHERE id=2572239;
DELETE FROM game_event_creature WHERE guid=2572239;
DELETE FROM game_event_creature_data WHERE guid=2572239;
DELETE FROM creature_battleground WHERE guid=2572239;
REPLACE INTO `creature` VALUES (2572240,60942,0,0,0,1,-2555.15,-3220.47,34.7723,0.181699,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572240,60942,0,0,0,1,-2555.15,-3220.47,34.7723,0.181699,300,300,54,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572240,60942,0,0,0,1,-2555.15,-3220.47,34.7723,0.181699,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572241,60941,0,0,0,1,-2524.35,-3196.38,34.212,6.15465,25,25,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2572241;
DELETE FROM creature_addon WHERE guid=2572241;
DELETE FROM creature_movement WHERE id=2572241;
DELETE FROM game_event_creature WHERE guid=2572241;
DELETE FROM game_event_creature_data WHERE guid=2572241;
DELETE FROM creature_battleground WHERE guid=2572241;
REPLACE INTO `creature` VALUES (2572242,60943,0,0,0,1,-2524.35,-3196.38,34.212,6.15465,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572242,60943,0,0,0,1,-2524.35,-3196.38,34.212,6.15465,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572243,60942,0,0,0,1,-2514.78,-3229.16,35.0896,4.15424,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572243,60942,0,0,0,1,-2514.78,-3229.16,35.0896,4.15424,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572244,60943,0,0,0,1,-2524.36,-3252.35,40.1374,1.63704,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572244,60943,0,0,0,1,-2524.36,-3252.35,40.1374,1.63704,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572245,60942,0,0,0,1,-2551.9,-3246.74,33.8268,3.95725,25,25,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2572245;
DELETE FROM creature_addon WHERE guid=2572245;
DELETE FROM creature_movement WHERE id=2572245;
DELETE FROM game_event_creature WHERE guid=2572245;
DELETE FROM game_event_creature_data WHERE guid=2572245;
DELETE FROM creature_battleground WHERE guid=2572245;
REPLACE INTO `creature` VALUES (2572246,60943,0,0,0,1,-2551.9,-3246.74,33.8268,3.95725,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572246,60943,0,0,0,1,-2551.9,-3246.74,33.8268,3.95725,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572247,60943,0,0,0,1,-2580.79,-3246.5,34.0383,4.62797,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572248,60942,0,0,0,1,-2590.55,-3263.98,34.8971,0.91745,25,25,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2572248;
DELETE FROM creature_addon WHERE guid=2572248;
DELETE FROM creature_movement WHERE id=2572248;
DELETE FROM game_event_creature WHERE guid=2572248;
DELETE FROM game_event_creature_data WHERE guid=2572248;
DELETE FROM creature_battleground WHERE guid=2572248;
REPLACE INTO `creature` VALUES (2572249,60943,0,0,0,1,-2590.23,-3263.94,34.8945,4.18314,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572249,60943,0,0,0,1,-2590.23,-3263.94,34.8945,4.18314,300,300,0,100,100,0,0,0);
DELETE FROM creature_movement WHERE id=2572249;
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,1, -2590.228027,-3263.935791,34.894489, 100);
REPLACE INTO `creature` VALUES (2572249,60943,0,0,0,1,-2590.23,-3263.94,34.8945,4.18314,300,300,0,100,100,0,0,0);
DELETE FROM creature_movement WHERE id=2572249;
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,1, -2590.228027,-3263.935791,34.894489, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,2, -2595.207520,-3271.480225,35.047569, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,3, -2603.030518,-3280.894043,34.943703, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,4, -2607.349365,-3286.663818,34.895660, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,5, -2615.118408,-3298.694580,34.857941, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,6, -2635.682373,-3310.496338,34.638771, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,7, -2641.197266,-3317.890137,34.169559, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,8, -2645.312988,-3328.715088,33.372044, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,9, -2649.077881,-3337.222168,33.261909, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,10, -2662.247314,-3353.710693,33.227161, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,11, -2648.694336,-3335.461182,33.252304, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,12, -2641.228516,-3318.869629,34.128242, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,13, -2634.797852,-3309.783447,34.660191, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,14, -2616.010010,-3299.052002,34.842369, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,15, -2608.154297,-3288.951660,34.905617, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572249,16, -2596.423584,-3272.393311,34.996811, 100);
REPLACE INTO `creature` VALUES (2572249,60943,0,0,0,1,-2590.23,-3263.94,34.8945,4.18314,300,300,0,100,100,2,0,0);
REPLACE INTO `creature` VALUES (2572250,60943,0,0,0,1,-2620.05,-3292.94,34.1228,4.09595,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572251,60943,0,0,0,1,-2653.99,-3327.89,33.5992,5.53784,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572252,60943,0,0,0,1,-2660.69,-3329.92,33.8349,4.2812,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572253,60944,0,0,0,1,-2604.13,-3263.2,34.733,1.47481,25,25,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2572253;
DELETE FROM creature_addon WHERE guid=2572253;
DELETE FROM creature_movement WHERE id=2572253;
DELETE FROM game_event_creature WHERE guid=2572253;
DELETE FROM game_event_creature_data WHERE guid=2572253;
DELETE FROM creature_battleground WHERE guid=2572253;
REPLACE INTO `creature` VALUES (2572254,60942,0,0,0,1,-2536.29,-3204.65,34.8223,4.83397,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572255,60942,0,0,0,1,-2548.28,-3236.32,33.7468,0.587323,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572255,60942,0,0,0,1,-2548.28,-3236.32,33.7468,0.587323,300,300,0,100,100,0,0,0);
DELETE FROM creature_movement WHERE id=2572255;
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,1, -2548.275146,-3236.324951,33.746769, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,2, -2535.027588,-3227.602539,34.352604, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,3, -2519.329834,-3214.674805,33.544815, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,4, -2510.215332,-3203.782715,32.233994, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,5, -2494.556396,-3192.063965,32.690140, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,6, -2480.884521,-3178.260010,32.910675, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,7, -2467.902344,-3175.492188,33.780682, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,8, -2456.889893,-3167.767578,35.451424, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,9, -2466.955566,-3175.114990,33.891876, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,10, -2479.432861,-3176.603760,32.961529, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,11, -2487.763428,-3186.471680,32.818596, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,12, -2498.074707,-3193.478760,32.617889, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,13, -2508.289795,-3201.144531,32.129044, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,14, -2517.608643,-3215.016357,33.423546, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,15, -2524.857666,-3219.496826,34.126720, 100);
INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation) VALUES (2572255,16, -2536.727295,-3228.430664,34.229973, 100);
REPLACE INTO `creature` VALUES (2572255,60942,0,0,0,1,-2548.28,-3236.32,33.7468,0.587323,300,300,0,100,100,2,0,0);
REPLACE INTO `creature` VALUES (2572256,60943,0,0,0,1,-2490.15,-3207.28,34.5164,6.28303,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572256,60943,0,0,0,1,-2490.15,-3207.28,34.5164,6.28303,300,300,7,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572257,60942,0,0,0,1,-2496.08,-3165.19,32.982,0.546763,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572258,60942,0,0,0,1,-2494.16,-3160.22,32.982,5.19632,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572259,60943,0,0,0,1,-2470.73,-3156.1,34.3281,1.39416,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572259,60943,0,0,0,1,-2470.73,-3156.1,34.3281,1.39416,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572260,60942,0,0,0,1,-2449.66,-3152.88,35.8596,6.08298,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572261,60943,0,0,0,1,-2443.47,-3150.38,35.8599,4.12603,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572262,60943,0,0,0,1,-2444.01,-3156.42,35.8599,2.06436,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572263,60942,0,0,0,1,-2444.25,-3112.73,34.777,2.25364,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572263,60942,0,0,0,1,-2444.25,-3112.73,34.777,2.25364,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572264,60943,0,0,0,1,-2473.09,-3132.26,34.7419,3.90612,25,25,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2572264;
DELETE FROM creature_addon WHERE guid=2572264;
DELETE FROM creature_movement WHERE id=2572264;
DELETE FROM game_event_creature WHERE guid=2572264;
DELETE FROM game_event_creature_data WHERE guid=2572264;
DELETE FROM creature_battleground WHERE guid=2572264;
REPLACE INTO `creature` VALUES (2572265,60943,0,0,0,1,-2461.06,-3111.66,34.8119,4.49988,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572266,60943,0,0,0,1,-2423.64,-3131.29,35.8513,3.2723,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572267,60943,0,0,0,1,-2417.81,-3142.65,35.8582,3.9878,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572268,60944,0,0,0,1,-2403.08,-3128.15,36.7622,3.58804,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572269,60943,0,0,0,1,-2420.03,-3114.88,36.7624,5.89082,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572270,60942,0,0,0,1,-2414.37,-3113.99,36.7624,3.88963,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572271,60942,0,0,0,1,-2413.31,-3127.27,36.7624,5.27664,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572271,60942,0,0,0,1,-2413.31,-3127.27,36.7624,5.27664,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572272,60942,0,0,0,1,-2401.7,-3145.93,36.7624,5.10739,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572273,60943,0,0,0,1,-2417.51,-3175.74,37.4317,5.00313,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572273,60943,0,0,0,1,-2417.51,-3175.74,37.4317,5.00313,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572274,60943,0,0,0,1,-2437.39,-3205.01,33.0942,1.74922,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572275,60913,0,0,0,0,-6569.82,-3467.95,304.625,2.37747,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572275,60913,0,0,0,0,-6569.82,-3467.95,304.625,2.37747,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572276,60912,0,0,0,0,-6569.42,-3485.55,313.997,5.41581,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572276,60912,0,0,0,0,-6569.42,-3485.55,313.997,5.41581,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572277,60914,0,0,0,0,-6578.3,-3500.13,312.683,3.42718,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572277,60914,0,0,0,0,-6578.3,-3500.13,312.683,3.42718,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572278,60913,0,0,0,0,-6566.43,-3514.8,314.793,5.56346,25,25,0,100,100,0,0,0);
DELETE FROM creature WHERE guid=2572278;
DELETE FROM creature_addon WHERE guid=2572278;
DELETE FROM creature_movement WHERE id=2572278;
DELETE FROM game_event_creature WHERE guid=2572278;
DELETE FROM game_event_creature_data WHERE guid=2572278;
DELETE FROM creature_battleground WHERE guid=2572278;
REPLACE INTO `creature` VALUES (2572279,60915,0,0,0,0,-6565.19,-3514.96,315.376,5.71112,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572279,60915,0,0,0,0,-6565.19,-3514.96,315.376,5.71112,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572280,60914,0,0,0,0,-6551.36,-3463.44,313.066,0.1827,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572280,60914,0,0,0,0,-6551.36,-3463.44,313.066,0.1827,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572281,60913,0,0,0,0,-6546.91,-3487.15,292.678,5.42524,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572281,60913,0,0,0,0,-6546.91,-3487.15,292.678,5.42524,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572282,60914,0,0,0,0,-6565.66,-3489.87,292.867,5.64671,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572282,60914,0,0,0,0,-6565.66,-3489.87,292.867,5.64671,300,300,5,100,100,1,0,0);
REPLACE INTO `creature` VALUES (2572283,60918,0,0,0,0,-7279.67,-3853.53,344.303,3.16929,25,25,0,100,100,0,0,0);
REPLACE INTO `creature` VALUES (2572283,60918,0,0,0,0,-7279.67,-3853.53,344.303,3.16929,300,300,5,100,100,1,0,0);
REPLACE INTO `gameobject` VALUES ( 5009652, 180914, 0, -7305.01, -3873.4, 342.4, 0.322218, 0, 0, 0.160413, 0.98705, 300, 300, 100, 1, 0, 0);
DELETE FROM gameobject WHERE guid = '5009652';
DELETE FROM game_event_gameobject WHERE guid = '5009652';
DELETE FROM gameobject_battleground WHERE guid = '5009652';
REPLACE INTO `gameobject` VALUES ( 5009653, 181734, 1, 438.674, -701.735, 68.75, 3.59766, 0, 0, 0.974112, -0.226065, 300, 300, 100, 1, 0, 0);
DELETE FROM gameobject WHERE guid = '5009653';
DELETE FROM game_event_gameobject WHERE guid = '5009653';
DELETE FROM gameobject_battleground WHERE guid = '5009653';
REPLACE INTO `gameobject` VALUES ( 5009654, 181734, 1, 432.114, -702.558, 68.75, 3.24423, 0, 0, 0.998683, -0.0512985, 300, 300, 100, 1, 0, 0);
REPLACE INTO `game_tele` (`id`, `position_x`, `position_y`, `position_z`, `orientation`, `map`, `name`) VALUES (847,1337.366699,1741.015991,142.226669,0.008433,1,'powdertown');
REPLACE INTO `game_tele` (`id`, `position_x`, `position_y`, `position_z`, `orientation`, `map`, `name`) VALUES (848,1799.763550,2121.331055,69.850037,0.020206,1,'blacksand');
REPLACE INTO `game_tele` (`id`, `position_x`, `position_y`, `position_z`, `orientation`, `map`, `name`) VALUES (849,897.672119,-808.482849,168.256531,4.259002,1,'baelhardul');
REPLACE INTO `game_tele` (`id`, `position_x`, `position_y`, `position_z`, `orientation`, `map`, `name`) VALUES (850,376.194336,-517.377136,85.919296,0.452168,1,'bramblethorn');
