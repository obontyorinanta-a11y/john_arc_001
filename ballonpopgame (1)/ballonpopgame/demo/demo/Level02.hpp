#ifndef LEVEL02_HPP
#define LEVEL02_HPP

// -----------------------------------------------------------------
//  Level02.hpp - THE MONTCLAIR.
//
//  Level 01 is a sword level: walk up to a man and hit him. Level 02 is
//  an archery level, so the whole shape of the fight changes - the enemy
//  is dangerous at contact and harmless at range, and the player's job is
//  to keep killing things before they close that distance.
//
//  What lives here:
//     the Hydra          - the boss, its fire, its phases, its HP bar
//     the fireballs      - its projectiles, on their own small pool
//     buildLevel02()     - the road itself
//     populateLevel02()  - who is standing on it
//     the phase machine  - running / combat / boss
//
//  What does NOT live here, because level 01 already does it properly:
//     arrows        - Projectiles.hpp, spawnArrow() with SIDE_PLAYER
//     foot enemies  - Units.hpp, U_SOLDIER and U_L2_HEAVY
//     collisions    - the arrow-versus-unit pass in Combat.hpp
//     camera, HP, effects, HUD frame, pause, game over
// -----------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "GameState.hpp"
#include "UI.hpp"
#include "Assets.hpp"
#include "Audio.hpp"
#include "World.hpp"
#include "Effects.hpp"
#include "Projectiles.hpp"
#include "Player.hpp"
#include "Units.hpp"

// registration for the boss artwork, from tools/measure_art.py
const CharArt ART_HYDRA = { 0.8893, 0.9933, 0.4267 };

// ================= the Hydra =================

struct Hydra {
	int    active;         // taking part in the level at all
	int    awake;          // woken by crossing the gate
	int    alive;
	double x, y;           // world position, feet on the ground line
	double w, h;           // collision box, deliberately tighter than the art
	int    hp, maxHp;
	int    fireCooldown;   // ticks until the next fireball
	int    hurtTimer;      // white flash after an arrow lands
	int    phase;          // 0..2, it fires faster as it weakens
	int    deathTimer;
};

Hydra hydra;

// ================= its fire =================

struct Fireball {
	int    active;
	double x, y;
	double vx, vy;
	double w, h;
	int    damage;
	int    life;           // ticks before it burns out on its own
};

Fireball fireballs[MAX_FIREBALLS];

void clearFireballs()
{
	int i;
	for (i = 0; i < MAX_FIREBALLS; i++) fireballs[i].active = 0;
}

Rect fireballRect(int i)
{
	// The collision box is smaller than the flame it is drawn as. A fireball
	// that hurts on the outer edge of its own glow feels like a cheat, and
	// the player has to be able to jump this thing.
	return makeRect(fireballs[i].x - fireballs[i].w * 0.32,
	                fireballs[i].y - fireballs[i].h * 0.30,
	                fireballs[i].w * 0.64, fireballs[i].h * 0.60);
}

void spawnFireball(double x, double y, double tx, double ty)
{
	int i;
	for (i = 0; i < MAX_FIREBALLS; i++) {
		if (fireballs[i].active) continue;

		{
			// aimed at where the player is standing right now, so jumping
			// over it actually works
			double dx = tx - x;
			double dy = ty - y;
			double d  = sqrt(dx * dx + dy * dy);
			if (d < 1.0) d = 1.0;

			fireballs[i].active = 1;
			fireballs[i].x      = x;
			fireballs[i].y      = y;
			fireballs[i].vx     = dx / d * HYDRA_FIRE_SPEED;
			fireballs[i].vy     = dy / d * HYDRA_FIRE_SPEED;
			fireballs[i].w      = 78;
			fireballs[i].h      = 40;
			fireballs[i].damage = HYDRA_FIRE_DAMAGE;
			fireballs[i].life   = 260;
		}
		return;
	}
}

void updateFireballs()
{
	int i;
	for (i = 0; i < MAX_FIREBALLS; i++) {
		if (!fireballs[i].active) continue;

		fireballs[i].x += fireballs[i].vx;
		fireballs[i].y += fireballs[i].vy;

		// a trailing ember or two
		if (randRange(0, 100) < 34)
			fxSpark(fireballs[i].x, fireballs[i].y, COL_FLAME, 1);

		if (fireballs[i].y <= GROUND_Y - 6) {
			fxDust(fireballs[i].x, GROUND_Y, 5);
			fxSpark(fireballs[i].x, GROUND_Y + 8, COL_FLAME, 6);
			fireballs[i].active = 0;
			continue;
		}
		if (--fireballs[i].life <= 0)          fireballs[i].active = 0;
		if (!onScreen(fireballs[i].x, 500))    fireballs[i].active = 0;
	}
}

void drawFireballs()
{
	int i;
	for (i = 0; i < MAX_FIREBALLS; i++) {
		if (!fireballs[i].active) continue;
		{
			double sx = worldToScreenX(fireballs[i].x);
			double sy = fireballs[i].y;

			// glow behind the sprite
			beginBlend();
			glColor4f(1.0f, 0.55f, 0.12f, 0.20f);
			iFilledCircle(sx, sy, fireballs[i].w * 0.42, 18);
			endBlend();

			// the artwork points RIGHT, so fire travelling left is mirrored
			drawSpriteEx(SPR_EFFECTS_FIREBALL,
			             sx - fireballs[i].w / 2, sy - fireballs[i].h / 2,
			             fireballs[i].w, fireballs[i].h,
			             (fireballs[i].vx < 0) ? 1 : 0, 1.0);
		}
	}
}

// ================= the boss itself =================

void clearHydra()
{
	hydra.active       = 0;
	hydra.awake        = 0;
	hydra.alive        = 0;
	hydra.x            = 0;
	hydra.y            = GROUND_Y;
	hydra.w            = 200;
	hydra.h            = 210;
	hydra.hp           = HYDRA_MAX_HP;
	hydra.maxHp        = HYDRA_MAX_HP;
	hydra.fireCooldown = HYDRA_FIRE_COOLDOWN;
	hydra.hurtTimer    = 0;
	hydra.phase        = 0;
	hydra.deathTimer   = 0;
}

void spawnHydra(double x)
{
	clearHydra();
	hydra.active = 1;
	hydra.alive  = 1;
	hydra.x      = x;
	hydra.y      = GROUND_Y;
}

Rect hydraRect()
{
	// tighter than the drawing: the tail and the outer heads should not
	// count as a target the player cannot see themselves hitting
	return makeRect(hydra.x - hydra.w / 2, hydra.y, hydra.w, hydra.h);
}

int hydraIsFighting()
{
	return (hydra.active && hydra.alive && hydra.awake);
}

void damageHydra(int amount)
{
	char buf[24];

	if (!hydraIsFighting()) return;

	hydra.hp -= amount;
	if (hydra.hp < 0) hydra.hp = 0;      // the bar never goes negative
	hydra.hurtTimer = 10;

	score += SCORE_HYDRA_HIT;
	fxNumber(hydra.x, hydra.y + hydra.h * 0.8, amount, COL_WHITE);
	fxSpark(hydra.x - 40, hydra.y + hydra.h * 0.6, COL_GOLD, 7);
	sfx("hit");

	// it grows angrier in two steps, and the only thing that changes is how
	// often it breathes - a readable escalation rather than a new attack
	{
		int newPhase = (hydra.hp < hydra.maxHp / 3) ? 2
		             : (hydra.hp < hydra.maxHp * 2 / 3) ? 1 : 0;
		if (newPhase != hydra.phase) {
			hydra.phase = newPhase;
			fxRing(hydra.x, hydra.y + hydra.h / 2, COL_RED, 240, 44);
			fxText(hydra.x, hydra.y + hydra.h + 26, "ENRAGED", COL_RED);
			shakeScreen(6);
			sfx("ult");
		}
	}

	if (hydra.hp <= 0) {
		hydra.alive      = 0;
		hydra.deathTimer = 120;
		clearFireballs();                 // nothing it already fired survives it
		score += SCORE_HYDRA;
		sprintf(buf, "+%d", SCORE_HYDRA);
		fxText(hydra.x, hydra.y + hydra.h + 30, buf, COL_GOLD);
		fxRing(hydra.x, hydra.y + hydra.h / 2, COL_GOLD,  360, 70);
		fxRing(hydra.x, hydra.y + hydra.h / 2, COL_WHITE, 280, 54);
		shakeScreen(14);
		sfx("levelup");
	}
}

// how often it breathes, tightening with each phase
int hydraFireInterval()
{
	int t = HYDRA_FIRE_COOLDOWN - hydra.phase * 22;
	return (t < 34) ? 34 : t;
}

void updateHydra()
{
	if (!hydra.active) return;

	if (hydra.hurtTimer > 0) hydra.hurtTimer--;

	if (!hydra.alive) {
		if (hydra.deathTimer > 0) hydra.deathTimer--;
		return;                            // dead things do not breathe fire
	}
	if (!hydra.awake) return;

	if (hydra.fireCooldown > 0) {
		hydra.fireCooldown--;
		return;
	}

	// three heads, so it spits three times in a burst, fanned slightly
	{
		double mouthX = hydra.x - hydra.w * 0.42;
		double mouthY = hydra.y + hydra.h * 0.66;
		spawnFireball(mouthX, mouthY, player.x, player.y + PLAYER_H * 0.45);
		if (hydra.phase >= 2)
			spawnFireball(mouthX, mouthY + 30, player.x, player.y + PLAYER_H * 0.80);
	}

	hydra.fireCooldown = hydraFireInterval();
	fxRing(hydra.x - hydra.w * 0.42, hydra.y + hydra.h * 0.66, COL_FLAME, 90, 20);
	sfx("ult");
}

void drawHydra()
{
	double sx, alpha = 1.0, tr = 1.0, tg = 1.0, tb = 1.0;
	double bob;

	if (!hydra.active) return;
	if (!onScreen(hydra.x, 700)) return;

	sx  = worldToScreenX(hydra.x);
	bob = sin(uiTime * 0.045) * 6.0;

	if (!hydra.alive) {
		alpha = clampd(hydra.deathTimer / 120.0, 0.0, 1.0);
		bob  -= (1.0 - alpha) * 26.0;      // it sinks as it fades
	}
	if (hydra.hurtTimer > 5) { tr = 2.0; tg = 1.5; tb = 1.5; }

	// shadow
	fillRectAlpha(sx - hydra.w * 0.55, GROUND_Y - 4, hydra.w * 1.10, 9,
	              COL_PANEL, 0.45 * alpha);

	// The artwork already faces LEFT, which is the way it needs to face at a
	// player arriving from the left, so it is never mirrored.
	drawCharacterTint(SPR_BOSS_HYDRA, ART_HYDRA, sx, hydra.y + bob,
	                  HYDRA_BODY_H, 0, alpha, tr, tg, tb);

	// heat haze while it is winding up to breathe
	if (hydra.alive && hydra.awake && hydra.fireCooldown < 18) {
		double t = 1.0 - hydra.fireCooldown / 18.0;
		beginBlend();
		glColor4f(1.0f, 0.5f, 0.1f, (float)(0.10 + 0.22 * t));
		iFilledCircle(sx - hydra.w * 0.42, hydra.y + HYDRA_BODY_H * 0.62,
		              16 + 26 * t, 18);
		endBlend();
	}
}

// the boss bar, only while the fight is actually on
void drawHydraHPBar()
{
	char buf[64];
	double bw = 580;
	double bx = SCREEN_WIDTH / 2 - bw / 2;
	double by = SCREEN_HEIGHT - 130;
	double f;

	if (!hydra.active || !hydra.awake) return;
	if (!hydra.alive && hydra.deathTimer <= 0) return;

	f = (double)hydra.hp / hydra.maxHp;
	if (f < 0) f = 0;

	fillRectAlpha(bx - 4, by - 4, bw + 8, 30, COL_PANEL, 0.85);
	setColor(COL_RED);
	iFilledRectangle(bx, by, bw * f, 22);
	drawFrame(makeRect(bx, by, bw, 22), COL_GOLD, 2);

	// the two phase thresholds
	setColor(COL_GOLD_DIM);
	iFilledRectangle(bx + bw * 0.333 - 1, by, 2, 22);
	iFilledRectangle(bx + bw * 0.666 - 1, by, 2, 22);

	setColor(COL_WHITE);
	drawTextCenteredBold(SCREEN_WIDTH / 2, by + 4, "THE HYDRA", FONT_SMALL);

	sprintf(buf, "%d", hydra.hp);
	setColor(COL_GREY);
	drawText(bx - 62, by + 4, buf, FONT_SMALL);

	sprintf(buf, "PHASE %d", hydra.phase + 1);
	setColor(COL_GOLD);
	drawText(bx + bw + 14, by + 4, buf, FONT_SMALL);
}

// ================= collisions unique to level 02 =================
//
// Arrow-versus-foot-enemy is already handled by resolveProjectiles() in
// Combat.hpp, which is why it is not repeated here. These are the two
// pairings that only exist in this level.

void resolveArrowsVsHydra()
{
	int p;
	if (!hydraIsFighting()) return;

	for (p = 0; p < MAX_PROJ; p++) {
		if (!projectiles[p].active) continue;
		if (projectiles[p].side != SIDE_PLAYER) continue;
		if (!rectsOverlap(projRect(p), hydraRect())) continue;

		damageHydra(projectiles[p].damage);
		projectiles[p].active = 0;      // one arrow, one hit, then it is gone
		if (!hydra.alive) return;
	}
}

void resolveFireVsPlayer()
{
	int i;
	Rect pr = playerRect();

	for (i = 0; i < MAX_FIREBALLS; i++) {
		if (!fireballs[i].active) continue;
		if (!rectsOverlap(fireballRect(i), pr)) continue;

		// Deactivated on the same tick it lands, so one fireball can only
		// ever cost the player one hit no matter how long the boxes overlap.
		fireballs[i].active = 0;
		fxSpark(fireballs[i].x, fireballs[i].y, COL_FLAME, 12);
		fxRing(player.x, player.y + PLAYER_H * 0.5, COL_FLAME, 120, 24);
		damagePlayer(fireballs[i].damage);
	}
}

// ================= the road =================

void buildLevel02()
{
	int i;

	clearObstacles();
	clearPickups();
	clearScenery();
	clearFireballs();
	clearHydra();

	levelKills  = 0;
	bossAlive   = 0;
	levelLength = LEVEL02_LENGTH;
	gateX       = LEVEL02_GATE;
	setLevelMood(2);

	// ---- health, spread evenly: there is no healing from kills ----
	addPickup(PU_HPBOX, 1100, GROUND_Y + 44);
	addPickup(PU_HPBOX, 2000, GROUND_Y + 44);
	addPickup(PU_HPBOX, 2900, GROUND_Y + 44);
	addPickup(PU_HPBOX, 3700, GROUND_Y + 44);
	addPickup(PU_HPBOX, 4400, GROUND_Y + 44);
	addPickup(PU_HPBOX, 4880, GROUND_Y + 44);   // last one before the Hydra

	// ---- coins, in runs that lead forward ----
	for (i = 0; i < 40; i++) {
		double x = 200 + i * 152.0;
		if (x > gateX - 180) break;
		addPickup(PU_COIN, x, GROUND_Y + 40 + (i % 3) * 34);
	}

	// ---- barricades to jump, which double as fireball cover ----
	addObstacle(OB_BARRICADE, 1350, 46, 74, 46);
	addObstacle(OB_BARRICADE, 2600, 46, 74, 50);
	addObstacle(OB_BARRICADE, 3900, 46, 74, 50);
	addObstacle(OB_BARRICADE, 4700, 46, 74, 54);

	// ---- scenery: thins out toward the Hydra's ground ----
	for (i = 0; i < 20; i++) {
		double x = 240 + i * 268.0;
		if (x > gateX - 300) break;
		addScenery(SC_TREE, x, 158 + (i % 5) * 30, 0.70);
		if (i % 3 != 1) addScenery(SC_TREE, x + 96, 126 + (i % 3) * 26, 0.74);
		if (i % 4 == 2) addScenery(SC_ROCK, x + 172, 24 + (i % 3) * 10, 0.90);
	}
	for (i = 1; i * 780 < levelLength; i++)
		addScenery(SC_FLAG, i * 780.0, 148, 1.0);

	// the Montclair burns
	addScenery(SC_SMOKE, 1500, 230, 0.80);
	addScenery(SC_SMOKE, 2800, 260, 0.80);
	addScenery(SC_SMOKE, 4200, 240, 0.80);
	addScenery(SC_SMOKE, 5250, 330, 0.80);
}

// -----------------------------------------------------------------
//  populateLevel02 - the progression the brief asks for, expressed as
//  placement rather than as a spawn timer.
//
//  Every enemy is placed at build time and sleeps until Joan is within
//  AGGRO_RANGE, then walks at her. From the player's seat that is exactly
//  "enemies keep coming as you advance", but it cannot leak: the pool is
//  fixed, nothing spawns forever, and a wave cannot pile up behind you
//  while you are busy.
//
//     0 - 2200   soldiers only, well spaced        - learn the bow
//  2200 - 3400   soldiers, first heavies mixed in  - learn the difference
//  3400 - 5000   mostly heavies, in pairs          - the hard stretch
// -----------------------------------------------------------------
void populateLevel02()
{
	int i;

	// --- opening: soldiers, one at a time ---
	static const double lightX[] = {
		 650,  900, 1200, 1450, 1700,
		1980, 2250, 2500, 2800, 3100
	};
	// --- heavies, appearing halfway and taking over ---
	static const double heavyX[] = {
		2350, 2900,
		3350, 3600, 3850,
		4100, 4350, 4600, 4820
	};

	int nl = (int)(sizeof(lightX) / sizeof(lightX[0]));
	int nh = (int)(sizeof(heavyX) / sizeof(heavyX[0]));

	for (i = 0; i < nl; i++) spawnUnit(U_SOLDIER,  SIDE_ENEMY, lightX[i]);
	for (i = 0; i < nh; i++) spawnUnit(U_L2_HEAVY, SIDE_ENEMY, heavyX[i]);

	// The Hydra waits past the gate and is deaf to aggro range - only
	// crossing the line wakes it.
	spawnHydra(LEVEL02_LENGTH - 300);

	levelKillGoal = countLiveEnemies();
}

#endif
