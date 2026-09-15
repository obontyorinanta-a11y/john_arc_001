#ifndef UNITS_HPP
#define UNITS_HPP

// -----------------------------------------------------------------
//  Units.hpp - every walking thing that is not Joan.
//
//  Enemies and companions share one struct and one update loop; the
//  only difference is the 'side' field and who they hunt.  That is
//  what makes the Oriflamme Call ability nearly free to implement.
//
//  Enemy ranks:
//     SOLDIER    the line infantry of level 01, balanced
//     VETERAN    heavier, longer reach, takes double damage from arrows
//     ARCHER     fragile, shoots from range, dies fast to the sword
//     COMMANDER  the boss, three phases, and the only rank that pays
//                no HP reward when it dies
// -----------------------------------------------------------------

#include <math.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "Assets.hpp"
#include "Audio.hpp"
#include "World.hpp"
#include "Effects.hpp"
#include "Projectiles.hpp"
#include "Player.hpp"

// U_L2_HEAVY is appended rather than inserted so every existing rank keeps
// its value and level 01 is untouched.
enum UnitType  { U_SOLDIER = 0, U_VETERAN, U_ARCHER, U_COMMANDER, U_COMPANION,
                 U_L2_HEAVY };
enum UnitState { US_SLEEP = 0, US_WALK, US_ATTACK, US_HURT, US_DEAD };

struct Unit {
	int active;
	int type;
	int side;              // SIDE_PLAYER for companions, SIDE_ENEMY otherwise
	int state;

	double x, y, vx, vy;
	double w, h;
	int facing;

	int hp, maxHp;
	int attackTimer;       // windup before the blow lands
	int attackCooldown;
	int hurtTimer;
	int deathTimer;
	int lifetime;          // companions fade away
	int lastHitSwing;      // so one sword swing cannot hit twice

	int phase;             // boss only
	double speed;
	double scale;
};

Unit units[MAX_UNITS];

// Body-box fractions from tools/measure_art.py. The soldier art faces LEFT,
// which is the way it needs to face while walking toward Joan, so it is drawn
// unmirrored on the approach and mirrored only if a unit ends up behind her.
const CharArt ART_HEAVY_UP     = { 0.8125, 0.9906, 0.4938 };
const CharArt ART_HEAVY_DOWN   = { 0.8906, 0.9187, 0.5098 };
const CharArt ART_SOLDIER_UP   = { 0.7844, 0.9938, 0.5179 };
const CharArt ART_SOLDIER_DOWN = { 0.9664, 0.9958, 0.6329 };
const CharArt ART_COMMANDER    = { 0.8984, 0.9883, 0.5284 };

// ---------------- table of ranks ----------------

int unitMaxHp(int type)
{
	switch (type) {
	case U_SOLDIER:   return SOLDIER_HP;
	case U_VETERAN:   return VETERAN_HP;
	case U_ARCHER:    return ARCHER_HP;
	case U_COMMANDER: return COMMANDER_HP;
	case U_COMPANION: return 1200;
	case U_L2_HEAVY:  return L2_HEAVY_HP;
	}
	return SOLDIER_HP;
}

double unitSpeedOf(int type)
{
	switch (type) {
	case U_SOLDIER:   return 1.45;
	case U_VETERAN:   return 1.05;
	case U_ARCHER:    return 1.25;
	case U_COMMANDER: return 1.30;
	case U_COMPANION: return 2.30;
	case U_L2_HEAVY:  return 1.15;
	}
	return 1.2;
}

double unitScaleOf(int type)
{
	switch (type) {
	case U_SOLDIER:   return 1.00;
	case U_VETERAN:   return 1.18;
	case U_ARCHER:    return 0.86;
	case U_COMMANDER: return 1.55;
	case U_COMPANION: return 0.92;
	case U_L2_HEAVY:  return 1.16;
	}
	return 1.0;
}

int unitDamageOf(int type)
{
	switch (type) {
	case U_SOLDIER:   return SOLDIER_DMG;
	case U_VETERAN:   return VETERAN_DMG;
	case U_ARCHER:    return ARCHER_DMG;
	case U_COMMANDER: return COMMANDER_DMG;
	case U_COMPANION: return 300;
	case U_L2_HEAVY:  return L2_HEAVY_DMG;
	}
	return 10;
}

int unitScoreOf(int type)
{
	switch (type) {
	case U_SOLDIER:  return SCORE_SOLDIER;
	case U_VETERAN:   return SCORE_VETERAN;
	case U_ARCHER: return SCORE_ARCHER;
	case U_COMMANDER:   return SCORE_COMMANDER;
	case U_L2_HEAVY:    return SCORE_L2_HEAVY;
	}
	return 0;
}

// Ticks of raised sword before the blow actually lands. This is the tell
// the player reacts to - 18 ticks is 360 ms. Do not shorten it to raise
// difficulty; raise damage instead, or the fight stops being a decision.
int unitWindupOf(int type)
{
	switch (type) {
	case U_VETERAN:   return 22;
	case U_COMMANDER: return 26;
	case U_L2_HEAVY:  return 22;
	}
	return 18;
}

int unitCooldownOf(int type)
{
	switch (type) {
	case U_VETERAN:   return 72;
	case U_COMMANDER: return 70;
	case U_L2_HEAVY:  return 68;
	}
	return 60;
}

// how far a unit reaches with its weapon
double unitReachOf(int type)
{
	switch (type) {
	case U_VETERAN:   return 96;
	case U_COMMANDER: return 130;
	case U_ARCHER:    return 420;   // shooting distance
	case U_COMPANION: return 72;
	case U_L2_HEAVY:  return 86;
	}
	return 62;
}

// the counter triangle
double damageMultiplier(int unitType, int fromArrow)
{
	if (unitType == U_VETERAN   &&  fromArrow) return 2.0;
	if (unitType == U_ARCHER && !fromArrow) return 2.0;
	if (unitType == U_VETERAN   && !fromArrow) return 0.75;
	return 1.0;
}

const char* unitName(int type)
{
	switch (type) {
	case U_SOLDIER:   return "ENGLISH SOLDIER";
	case U_VETERAN:   return "VETERAN";
	case U_ARCHER:    return "CROSSBOW";
	case U_COMMANDER: return "THE ENGLISH COMMANDER";
	case U_COMPANION: return "COMPANION";
	case U_L2_HEAVY:  return "MONTCLAIR HEAVY";
	}
	return "";
}

// ---------------- housekeeping ----------------

void clearUnits()
{
	int i;
	for (i = 0; i < MAX_UNITS; i++) units[i].active = 0;
}

Unit* spawnUnit(int type, int side, double x)
{
	int i;
	for (i = 0; i < MAX_UNITS; i++) {
		if (units[i].active) continue;

		units[i].active   = 1;
		units[i].type     = type;
		units[i].side     = side;
		units[i].state    = (side == SIDE_ENEMY) ? US_SLEEP : US_WALK;
		units[i].x        = x;
		units[i].y        = GROUND_Y;
		units[i].vx = units[i].vy = 0;
		units[i].scale    = unitScaleOf(type);
		units[i].w        = 52 * units[i].scale;
		units[i].h        = 120 * units[i].scale;
		units[i].facing   = -1;
		units[i].maxHp    = unitMaxHp(type);
		units[i].hp       = units[i].maxHp;
		units[i].speed    = unitSpeedOf(type);
		units[i].attackTimer    = 0;
		units[i].attackCooldown = randRange(0, 40);
		units[i].hurtTimer   = 0;
		units[i].deathTimer  = 0;
		units[i].lifetime    = (type == U_COMPANION) ? ALLY_LIFETIME : 0;
		units[i].lastHitSwing = -1;
		units[i].phase       = 0;
		return &units[i];
	}
	return 0;
}

Rect unitRect(int i)
{
	return makeRect(units[i].x - units[i].w / 2, units[i].y, units[i].w, units[i].h);
}

int countLiveEnemies()
{
	int i, n = 0;
	for (i = 0; i < MAX_UNITS; i++)
		if (units[i].active && units[i].side == SIDE_ENEMY && units[i].state != US_DEAD) n++;
	return n;
}

// ---------------- damage ----------------

// -----------------------------------------------------------------
//  killEnemy - what happens when an enemy reaches 0 hp.
//
//  Killing pays score and sometimes a coin. It does NOT pay health:
//  Joan's only source of healing on this road is an HP box, which is
//  what keeps the 5000 she starts with a budget she has to spend
//  carefully rather than a pool that refills itself.
// -----------------------------------------------------------------
void killEnemy(int i)
{
	units[i].hp         = 0;
	units[i].state      = US_DEAD;
	units[i].deathTimer = 40;

	if (units[i].side != SIDE_ENEMY) return;   // companions score nothing

	levelKills++;
	player.kills++;
	score += unitScoreOf(units[i].type);

	fxText(units[i].x, units[i].y + units[i].h + 14, "SLAIN", COL_GOLD);
	fxSpark(units[i].x, units[i].y + units[i].h * 0.5, COL_GOLD, 10);

	if (units[i].type == U_COMMANDER) {
		bossAlive = 0;
		shakeScreen(12);
		fxRing(units[i].x, units[i].y + units[i].h / 2, COL_GOLD, 260, 60);
		fxRing(units[i].x, units[i].y + units[i].h / 2, COL_WHITE, 200, 46);
	}

	// the fallen drop coin now and then
	if (randRange(0, 100) < 55)
		addPickup(PU_COIN, units[i].x, GROUND_Y + 40);
}

void damageUnit(int i, int amount, int fromArrow)
{
	if (!units[i].active || units[i].state == US_DEAD) return;

	amount = (int)(amount * damageMultiplier(units[i].type, fromArrow));
	if (amount < 1) amount = 1;

	units[i].hp -= amount;
	units[i].hurtTimer = 12;
	fxNumber(units[i].x, units[i].y + units[i].h, amount,
	         damageMultiplier(units[i].type, fromArrow) > 1.0 ? COL_GOLD : COL_WHITE);
	fxSpark(units[i].x, units[i].y + units[i].h * 0.6, COL_GOLD, 6);
	sfx("hit");

	if (units[i].hp <= 0) killEnemy(i);
}

// ---------------- AI ----------------

// finds the nearest opposing target; returns -1 for none, -2 means "the player"
int findTarget(int i)
{
	int k, best = -1;
	double bestD = 1e9;

	if (units[i].side == SIDE_ENEMY) return -2;   // enemies always hunt Joan

	for (k = 0; k < MAX_UNITS; k++) {
		if (!units[k].active || units[k].state == US_DEAD) continue;
		if (units[k].side == units[i].side) continue;
		double d = fabs(units[k].x - units[i].x);
		if (d < bestD) { bestD = d; best = k; }
	}
	return best;
}

void updateUnits()
{
	int i;

	for (i = 0; i < MAX_UNITS; i++) {
		if (!units[i].active) continue;

		if (units[i].state == US_DEAD) {
			units[i].deathTimer--;
			if (units[i].deathTimer <= 0) units[i].active = 0;
			continue;
		}

		if (units[i].hurtTimer > 0) units[i].hurtTimer--;
		if (units[i].attackCooldown > 0) units[i].attackCooldown--;

		// companions expire
		if (units[i].type == U_COMPANION) {
			if (--units[i].lifetime <= 0) {
				units[i].state = US_DEAD;
				units[i].deathTimer = 30;
				continue;
			}
		}

		// Wake up when Joan gets close - except the commander, who is deaf
		// to aggro range. Only crossing the finish line wakes him, which is
		// what stops the boss wandering up the road to meet the player.
		if (units[i].state == US_SLEEP) {
			if (units[i].type == U_COMMANDER) continue;
			if (fabs(player.x - units[i].x) < AGGRO_RANGE) units[i].state = US_WALK;
			else continue;
		}

		// pick a target
		double tx, ty;
		int t = findTarget(i);
		if (t == -2) {
			tx = player.x; ty = player.y;
			if (player.state == PS_DEAD) { units[i].state = US_WALK; }
		} else if (t >= 0) {
			tx = units[t].x; ty = units[t].y;
		} else {
			// no enemy left: companions walk forward with Joan
			tx = player.x + 90; ty = player.y;
		}

		double dx = tx - units[i].x;
		double dist = fabs(dx);
		double reach = unitReachOf(units[i].type);

		units[i].facing = (dx >= 0) ? 1 : -1;

		// ---- attacking ----
		if (units[i].attackTimer > 0) {
			units[i].attackTimer--;
			units[i].vx = 0;

			if (units[i].attackTimer == 0) {
				// The blow lands here. For everything with a sword that
				// just means leaving state US_ATTACK with attackTimer at 1,
				// which is the tick Combat.hpp tests the swing box on.
				if (units[i].type == U_ARCHER) {
					spawnArrow(units[i].x + units[i].facing * 22,
					           units[i].y + units[i].h * 0.55,
					           units[i].facing, SIDE_ENEMY, unitDamageOf(U_ARCHER));
					sfx("arrow");
				}

				units[i].attackCooldown = unitCooldownOf(units[i].type);
				if (units[i].type == U_COMMANDER && units[i].phase >= 2)
					units[i].attackCooldown = 52;      // enraged, swings faster
				units[i].state = US_WALK;
			}
			continue;
		}

		// ---- approach ----
		if (dist > reach * 0.8) {
			units[i].vx = units[i].speed * ((dx >= 0) ? 1 : -1);
			units[i].state = US_WALK;
		} else {
			units[i].vx = 0;
			if (units[i].attackCooldown <= 0) {
				units[i].state = US_ATTACK;
				units[i].attackTimer = unitWindupOf(units[i].type);
			}
		}

		// boss phases: angrier as its health falls
		if (units[i].type == U_COMMANDER) {
			int newPhase = (units[i].hp < units[i].maxHp / 3) ? 2
			             : (units[i].hp < units[i].maxHp * 2 / 3) ? 1 : 0;
			if (newPhase != units[i].phase) {
				units[i].phase = newPhase;
				units[i].speed = unitSpeedOf(U_COMMANDER) + newPhase * 0.45;
				fxRing(units[i].x, units[i].y + units[i].h / 2, COL_RED, 200, 40);
				fxText(units[i].x, units[i].y + units[i].h + 20, "ENRAGED", COL_RED);
				shakeScreen(6);
				sfx("ult");
			}
		}

		units[i].x += units[i].vx;

		// stay inside the level
		units[i].x = clampd(units[i].x, 40, levelLength - 40);

		// simple gravity so nothing floats if it ever leaves the ground
		units[i].vy -= GRAVITY;
		units[i].y += units[i].vy;
		if (units[i].y <= GROUND_Y) { units[i].y = GROUND_Y; units[i].vy = 0; }
	}
}

// ---------------- drawing ----------------

void unitTint(int type, int hurt, double* r, double* g, double* b)
{
	switch (type) {
	case U_SOLDIER:     *r = 1.00; *g = 1.00; *b = 1.00; break;
	case U_VETERAN:      *r = 0.72; *g = 0.82; *b = 1.05; break;   // steel blue
	case U_ARCHER:    *r = 1.05; *g = 0.86; *b = 0.62; break;   // leather
	case U_COMMANDER: *r = 1.06; *g = 0.74; *b = 0.70; break;   // blood-cast steel
	case U_COMPANION: *r = 0.78; *g = 0.90; *b = 1.00; break;   // French blue
	case U_L2_HEAVY:  *r = 0.92; *g = 0.80; *b = 0.94; break;   // bruised violet
	default:          *r = *g = *b = 1.0;
	}
	if (hurt) { *r = 2.0; *g = 1.6; *b = 1.6; }                 // white flash
}

void drawUnits()
{
	int i;
	for (i = 0; i < MAX_UNITS; i++) {
		if (!units[i].active) continue;
		if (!onScreen(units[i].x, 300)) continue;

		double  sx    = worldToScreenX(units[i].x);
		double  bodyH = units[i].h * 1.24;
		double  alpha = 1.0;
		double  tr, tg, tb;
		double  bob = 0;
		int     flip, id;
		CharArt art;

		// ---- pose ----
		// The raised sword IS the wind-up warning. An enemy that only
		// changed pose on the tick it hit would give the player nothing
		// to read, so the sprite swaps the instant attackTimer starts.
		if (units[i].type == U_COMPANION) {
			id = SPR_PLAYER_JOAN_RUN_00;  art = ART_JOAN_RUN;
		} else if (units[i].type == U_L2_HEAVY) {
			if (units[i].attackTimer > 0) { id = SPR_ENEMY_HEAVY_SWORDUP;   art = ART_HEAVY_UP; }
			else                          { id = SPR_ENEMY_HEAVY_SWORDDOWN; art = ART_HEAVY_DOWN; }
		} else if (units[i].attackTimer > 0) {
			id = SPR_ENEMY_SOLDIER_SWORDUP;   art = ART_SOLDIER_UP;
		} else {
			id = SPR_ENEMY_SOLDIER_SWORDDOWN; art = ART_SOLDIER_DOWN;
		}

		// The enemy art is drawn facing LEFT and the companion art facing
		// RIGHT, so they mirror on opposite conditions.
		if (units[i].type == U_COMPANION) flip = (units[i].facing > 0) ? 0 : 1;
		else                              flip = (units[i].facing > 0) ? 1 : 0;

		if (units[i].state == US_DEAD) alpha = units[i].deathTimer / 40.0;
		if (units[i].type == U_COMPANION && units[i].lifetime < 120)
			alpha *= units[i].lifetime / 120.0;

		if (units[i].state == US_WALK && fabs(units[i].vx) > 0.1)
			bob = fabs(sin(uiTime * 0.16 + i)) * 3.5;
		if (units[i].attackTimer > 0) bob = -3;   // crouch into the swing

		unitTint(units[i].type, units[i].hurtTimer > 6, &tr, &tg, &tb);

		// shadow
		fillRectAlpha(sx - units[i].w * 0.42, GROUND_Y - 3,
		              units[i].w * 0.84, 7, COL_PANEL, 0.45);

		drawCharacterTint(id, art, sx, units[i].y + bob, bodyH,
		                  flip, alpha, tr, tg, tb);

		// health pip above the head, only once damaged, never for the boss
		// (the boss has its own bar at the top of the screen)
		if (units[i].state != US_DEAD && units[i].hp < units[i].maxHp
		    && units[i].type != U_COMMANDER) {
			double bw = units[i].w * 1.05;
			double f  = (double)units[i].hp / units[i].maxHp;
			double by = units[i].y + bodyH + 10;
			fillRectAlpha(sx - bw / 2, by, bw, 6, COL_PANEL, 0.8);
			setColor(units[i].side == SIDE_ENEMY ? COL_RED : COL_BLUE);
			iFilledRectangle(sx - bw / 2, by, bw * f, 6);
		}

		// a veteran is the same soldier drawn larger, so it gets a steel
		// pennant to make the rank readable at a glance
		if ((units[i].type == U_VETERAN || units[i].type == U_L2_HEAVY)
		    && units[i].state != US_DEAD) {
			double px = sx - units[i].facing * 30;
			setColor(COL_GREY_DARK);
			iFilledRectangle(px, units[i].y + 10, 4, bodyH * 0.95);
			drawDiamond(px + 2, units[i].y + bodyH * 1.0, 8, COL_GREY);
		}

		// the wind-up flashes red so the player can react to it
		if (units[i].attackTimer > 0) {
			double t = 1.0 - (double)units[i].attackTimer
			                 / unitWindupOf(units[i].type);
			fillRectAlpha(sx - units[i].w / 2, units[i].y, units[i].w, bodyH,
			              COL_RED, 0.10 + 0.16 * t);
		}
	}
}

#endif
