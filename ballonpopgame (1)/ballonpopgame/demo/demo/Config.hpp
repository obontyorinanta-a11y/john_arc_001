#ifndef CONFIG_HPP
#define CONFIG_HPP

// -----------------------------------------------------------------
//  Joan of Arc : Banner of Orleans
//  Config.hpp - every tunable number of the game lives here.
//  Nothing in this file depends on any other game file.
// -----------------------------------------------------------------

#pragma warning(disable:4996)   // sprintf / string literal warnings (VS2013)

// ---- window ----
const int SCREEN_WIDTH  = 1200;
const int SCREEN_HEIGHT = 675;

// ---- timers (milliseconds) ----
const int LOGIC_TICK_MS = 20;    // simulation clock
const int ANIM_TICK_MS  = 120;   // animation frame clock
const int KEY_POLL_MS   = 8;     // how often the framework calls fixedUpdate()

// ---- screen transition ----
const int FADE_TICKS = 14;       // fade-in length, counted in logic ticks

// ---- level select ----
const int LOCK_MSG_TICKS = 130;  // how long "LEVEL LOCKED" stays up (~2.6 s)
const int LOCK_MSG_FADE  = 30;   // ticks it spends fading out at the end

// ---- progression ----
const int TOTAL_LEVELS = 3;
const int START_LIVES  = 3;

// ---- palette (0-255 per channel) ----
struct Color { int r, g, b; };

const Color COL_SKY_TOP     = {  10,  14,  32 };
const Color COL_SKY_BOTTOM  = {  74,  86, 122 };
const Color COL_HILL_FAR    = {  34,  42,  68 };
const Color COL_HILL_NEAR   = {  16,  22,  40 };
const Color COL_GROUND      = {  22,  20,  18 };

const Color COL_GOLD        = { 214, 176,  70 };
const Color COL_GOLD_DIM    = { 132, 104,  36 };
const Color COL_BLUE        = {  38,  60, 122 };
const Color COL_BLUE_DARK   = {  18,  28,  62 };
const Color COL_RED         = { 156,  44,  40 };

const Color COL_WHITE       = { 238, 236, 228 };
const Color COL_GREY        = { 150, 154, 166 };
const Color COL_GREY_DARK   = {  86,  90, 102 };
const Color COL_PANEL       = {  10,  13,  26 };
const Color COL_FLAME       = { 236, 156,  52 };
const Color COL_RED_LIGHT   = { 226, 96,  86 };

// ---- world ----
const double GROUND_Y   = 96;     // top surface of the ground band
const double GRAVITY    = 0.62;
const double TERMINAL_V = 22.0;

// ---- player ----
const double PLAYER_W        = 44;    // hitbox, deliberately slimmer than the art
const double PLAYER_H        = 118;
const double PLAYER_ART_H    = 150;   // drawn height of Joan's BODY, sword excluded
const double PLAYER_SPEED    = 4.5;   // while a direction key is held
const double PLAYER_AUTO_SPEED = 2.2; // while advancing on her own
const double AUTO_STOP_RANGE = 220;   // auto-advance halts inside this range
const double PLAYER_JUMP     = 13.6;
const int    PLAYER_MAX_HP   = 5000;
const int    PLAYER_INVULN   = 45;    // ticks of mercy after being hit
const int    SWING_TICKS     = 16;    // how long a sword swing lasts
const int    SWING_COOLDOWN  = 26;
const int    BOW_COOLDOWN    = 34;
const int    HEAL_COOLDOWN   = 900;
const int    SUMMON_COOLDOWN = 1100;
const int    ULT_COOLDOWN    = 1800;
const double SWORD_REACH     = 86;

// Playtest note: at 490 a soldier died in three swings, which is 1.5 s of
// exposure - too short for the rest of the group to land anything, and a
// full clear of the level finished at 98% health. At 340 a soldier takes
// five swings and the commander sixteen, which roughly triples the time
// Joan spends inside enemy reach without touching any number the design
// brief fixed.
const int    SWORD_DAMAGE    = 340;
const int    ARROW_DAMAGE    = 260;

// ---- health ----
// Kills pay no health at all. An HP box is the only way Joan heals, so the
// 5000 she starts the road with is a budget, and the boxes are top-ups
// rather than a refill.
const int HP_BOX_HEAL = 100;          // one health box

// ---- enemy ranks (level 01) ----
const int SOLDIER_HP   = 1469, SOLDIER_DMG   =  50;
const int VETERAN_HP   = 2100, VETERAN_DMG   =  90;
const int ARCHER_HP    =  900, ARCHER_DMG    =  60;

// The commander is a bigger, tougher soldier rather than a different
// creature: same artwork, 1.55x the size, and enough hp to take eight clean
// swings at SWORD_DAMAGE. That is a long enough fight to feel like a duel
// while still leaving a player who reaches the arena with a reasonable
// health bar heavily favoured to finish it.
const int COMMANDER_HP = 2600, COMMANDER_DMG = 150;

// ---- level 01 geometry ----
const double LEVEL01_LENGTH = 6400;
const double LEVEL01_GATE   = 5900;   // the banner that starts the boss fight

// -----------------------------------------------------------------
//  LEVEL 02 - THE MONTCLAIR
//
//  An archery level. Joan runs the road with a bow instead of a sword,
//  shoots at everything walking the other way, and finishes against the
//  Hydra. Every number below is a dial; nothing is hard-coded elsewhere.
// -----------------------------------------------------------------

// Playtest note: at 7200 the road took four minutes to walk before the boss
// even appeared, which is a long time to spend on the part of the level that
// is not the interesting part. 5600 keeps every section and cuts the padding.
const double LEVEL02_LENGTH = 5600;
const double LEVEL02_GATE   = 5000;   // crossing this wakes the Hydra
const double ARCHER_ART_H   = 170;    // drawn height of the archer's body

// ---- the bow ----
//
// Level 02 is an archery level, so the arrow is tuned by KILL COUNT rather
// than by a damage figure picked in isolation. One arrow is exactly a
// soldier's health, which makes the rest fall out on its own:
//
//     level01_enemy_sword*   SOLDIER_HP  1469  ->  1 arrow
//     level02_sorwd*         L2_HEAVY_HP 2800  ->  2 arrows (1469 x 2 = 2938)
//     the Hydra              see below         -> 10 arrows
//
// Writing it as SOLDIER_HP rather than as the number 1469 means the two
// stay locked together: retune the soldier and the one-arrow rule holds.
// This constant is used ONLY by the level 02 bow, so level 01's sword
// balance is untouched by any of it.
const int    ARROW_DAMAGE_L2 = SOLDIER_HP;   // 1469 - one arrow, one soldier
const int    ARROW_COOLDOWN  = 20;    // 0.4 s between shots
const int    SHOOT_TICKS     = 24;    // length of draw-and-release, in ticks
const double ARROW_SPEED_L2  = 15.5;

// ---- the Montclair heavy: the stronger of the two foot enemies ----
const int    L2_HEAVY_HP  = 2800;     // vs SOLDIER_HP 1469
const int    L2_HEAVY_DMG =  120;     // vs SOLDIER_DMG 50

// ---- the Hydra ----
// A three-head volley at 220 each was 660 damage a breath, which killed a
// player who arrived at half health in four breaths - too fast to learn the
// pattern. 150 with a longer gap leaves room to misjudge a jump twice.
const int    HYDRA_ARROWS_TO_KILL = 10;
const int    HYDRA_MAX_HP        = ARROW_DAMAGE_L2 * HYDRA_ARROWS_TO_KILL;
const int    HYDRA_FIRE_DAMAGE   =  150;
const int    HYDRA_FIRE_COOLDOWN =  110;   // 2.2 s between volleys
const double HYDRA_FIRE_SPEED    =  6.2;
const double HYDRA_BODY_H        =  300;   // drawn height, dwarfs the archer

// How far in front of the Hydra Joan is held. She fights it from here and
// cannot get past it: walking through the boss put her on its right, where
// every arrow she loosed flew away from it and the fight could not be won.
const double HYDRA_STANDOFF      =  150;
const double HYDRA_ARENA_BACK    =  420;   // room behind her, for dodging
const int    MAX_FIREBALLS       =   16;

// ---- level 02 scoring ----
const int SCORE_L2_HEAVY  =   60;
const int SCORE_HYDRA_HIT =    5;
const int SCORE_HYDRA     = 2000;

// ---- units ----
const int MAX_UNITS     = 56;
const int MAX_PROJ      = 72;
const int MAX_PICKUPS   = 48;
const int MAX_OBSTACLES = 28;
const int MAX_EFFECTS   = 96;

const double AGGRO_RANGE   = 640;   // how close before a unit wakes up
const double ALLY_AURA     = 220;   // companions hit harder inside this ring
const int    ALLY_LIFETIME = 1000;

// ---- scoring ----
const int SCORE_SOLDIER   = 25;
const int SCORE_VETERAN   = 40;
const int SCORE_ARCHER    = 35;
const int SCORE_COMMANDER = 750;
const int SCORE_COIN      = 10;

// ---- fonts (GLUT bitmap fonts are the only ones iGraphics offers) ----
#define FONT_BIG    GLUT_BITMAP_TIMES_ROMAN_24
#define FONT_MED    GLUT_BITMAP_HELVETICA_18
#define FONT_SMALL  GLUT_BITMAP_9_BY_15

#endif
