#ifndef GAME_HPP
#define GAME_HPP

// -----------------------------------------------------------------
//  Game.hpp - level flow and the gameplay frame.
//
//  startLevel()   builds a level and places its army
//  updateGame()   one tick of simulation, then the win / lose checks
//  drawGameplay() one rendered frame of the battlefield
// -----------------------------------------------------------------

#include <math.h>
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
#include "Combat.hpp"
#include "HUD.hpp"

int levelIntroTimer = 0;   // shows the level name for a moment
int bossWoke        = 0;

// ---------------- building the army ----------------

// -----------------------------------------------------------------
//  populateLevel01 - who is standing on the road.
//
//  Difficulty is carried entirely by spacing and grouping, not by making
//  a soldier stronger later on: the first soldier is alone, the last group
//  is seven deep. Enemies inside a group sit 40-90 units apart, which is
//  the only "formation" this level needs - each rank stops at its own
//  reach, so veterans naturally jab from behind a front rank of soldiers.
// -----------------------------------------------------------------
void populateLevel01()
{
	int i;

	static const double soldierX[] = {
		/* First Blood */   900,
		/* The Road    */  1350, 1600, 1880,
		/* The Press   */  2450, 2650, 2900, 3100,
		/* Shield Wall */  3750, 3960, 4150, 4340, 4520
	};
	static const double veteranX[] = {
		/* The Press      */ 3000,
		/* Shield Wall    */ 3860, 4420,
		/* Final Approach */ 5050, 5280, 5500
	};

	int ns = (int)(sizeof(soldierX) / sizeof(soldierX[0]));
	int nv = (int)(sizeof(veteranX) / sizeof(veteranX[0]));

	for (i = 0; i < ns; i++) spawnUnit(U_SOLDIER, SIDE_ENEMY, soldierX[i]);
	for (i = 0; i < nv; i++) spawnUnit(U_VETERAN, SIDE_ENEMY, veteranX[i]);

	// The commander waits past the banner. He starts asleep and, unlike
	// every other enemy, aggro range will not wake him - only crossing the
	// finish line does.
	spawnUnit(U_COMMANDER, SIDE_ENEMY, LEVEL01_LENGTH - 250);
	bossAlive = 1;

	levelKillGoal = countLiveEnemies();
}

// Is any awake enemy close enough that Joan should stop marching?
int enemyNear(double range)
{
	int i;
	for (i = 0; i < MAX_UNITS; i++) {
		if (!units[i].active || units[i].state == US_DEAD) continue;
		if (units[i].side != SIDE_ENEMY) continue;
		if (units[i].state == US_SLEEP) continue;
		if (fabs(units[i].x - player.x) < range) return 1;
	}
	return 0;
}

void wakeCommander()
{
	int i;
	for (i = 0; i < MAX_UNITS; i++) {
		if (!units[i].active || units[i].type != U_COMMANDER) continue;
		units[i].state = US_WALK;
		return;
	}
}

int commanderAlive()
{
	int i;
	for (i = 0; i < MAX_UNITS; i++) {
		if (!units[i].active) continue;
		if (units[i].type != U_COMMANDER) continue;
		if (units[i].state != US_DEAD) return 1;
	}
	return 0;
}

int bossBannerTimer = 0;

// -----------------------------------------------------------------
//  updateLevelPhase - PLAYING / COMBAT / BOSS_FIGHT.
//
//  Crossing the banner is one way. Joan is clamped into the arena, the
//  autopilot is switched off for good and the commander wakes. The level
//  does NOT end here - it ends when he falls, and nowhere else.
// -----------------------------------------------------------------
void updateLevelPhase()
{
	if (bossBannerTimer > 0) bossBannerTimer--;

	// Level 02: crossing the gate wakes the Hydra and shuts the road, the
	// same one-way door Brittany uses.
	if (currentLevel == 2) {
		if (levelPhase == PHASE_BOSS) {
			// The arena is the strip of ground IN FRONT of the Hydra. The
			// right-hand wall is the boss itself, not the end of the level:
			// letting her walk past it left her shooting away from the only
			// thing she needs to hit, and the level became unwinnable.
			double wall = hydra.x - hydra.w * 0.5 - HYDRA_STANDOFF;
			double back = gateX - HYDRA_ARENA_BACK;

			if (player.x >= wall && player.vx > 0) player.vx = 0;
			player.x = clampd(player.x, back, wall);

			// she is always looking at it, so a shot is never wasted by
			// having drifted round to face the wrong way
			if (player.vx == 0) player.facing = 1;
			return;
		}
		if (player.x >= gateX) {
			levelPhase         = PHASE_BOSS;
			player.autoAdvance = 0;
			bossBannerTimer    = 150;
			hydra.awake        = 1;
			hydra.fireCooldown = 70;      // a beat before the first breath
			shakeScreen(10);
			fxRing(player.x, player.y + PLAYER_H * 0.5, COL_FLAME, 320, 50);
			sfx("ult");
			return;
		}
		if (enemyNear(AUTO_STOP_RANGE)) {
			levelPhase = PHASE_COMBAT;  player.autoAdvance = 0;
		} else {
			levelPhase = PHASE_ADVANCE; player.autoAdvance = 1;
		}
		return;
	}

	// Level 3 still uses the old flow, where the banner itself ends the
	// level. Only Brittany and the Montclair turn it into a door.
	if (currentLevel != 1) return;

	if (levelPhase == PHASE_BOSS) {
		player.x = clampd(player.x, gateX - 120, levelLength - 80);
		return;
	}

	if (player.x >= gateX) {
		levelPhase         = PHASE_BOSS;
		player.autoAdvance = 0;
		bossBannerTimer    = 150;
		wakeCommander();
		shakeScreen(12);
		fxRing(player.x, player.y + PLAYER_H * 0.5, COL_RED, 320, 50);
		sfx("ult");
		return;
	}

	if (enemyNear(AUTO_STOP_RANGE)) {
		levelPhase         = PHASE_COMBAT;
		player.autoAdvance = 0;
	} else {
		levelPhase         = PHASE_ADVANCE;
		player.autoAdvance = 1;
	}
}

void populateLevel(int level)
{
	int i, count;
	double x;

	if (level == 1) { populateLevel01(); return; }

	if (level == 1) {
		count = 13;
		for (i = 0; i < count; i++) {
			x = 780 + i * (levelLength - 1300) / count;
			spawnUnit((i % 4 == 3) ? U_ARCHER : U_SOLDIER, SIDE_ENEMY, x);
		}
	} else if (level == 2) {
		count = 18;
		for (i = 0; i < count; i++) {
			int t = U_SOLDIER;
			if (i % 5 == 2) t = U_VETERAN;
			if (i % 5 == 4) t = U_ARCHER;
			x = 720 + i * (levelLength - 1200) / count;
			spawnUnit(t, SIDE_ENEMY, x);
		}
	} else {
		count = 10;
		for (i = 0; i < count; i++) {
			int t = (i % 3 == 0) ? U_VETERAN : (i % 3 == 1) ? U_ARCHER : U_SOLDIER;
			x = 700 + i * (levelLength - 1800) / count;
			spawnUnit(t, SIDE_ENEMY, x);
		}
		// the enemy commander waits at the end of the field
		spawnUnit(U_COMMANDER, SIDE_ENEMY, levelLength - 620);
		bossAlive = 1;
	}

	levelKillGoal = countLiveEnemies();
}

void startLevel(int level)
{
	currentLevel = clampi(level, 1, TOTAL_LEVELS);

	clearUnits();
	clearProjectiles();
	clearEffects();

	// Level 02 has its own builder in Level02.hpp; every other level goes
	// through the shared one.
	if (currentLevel == 2) {
		buildLevel02();
		populateLevel02();
	} else {
		buildLevel(currentLevel);
		populateLevel(currentLevel);
	}

	resetPlayer(140);
	// The Montclair is an archery level, outside the Path of the Maid, so
	// Joan takes it at stage 1: a flat 5000 max HP and no sword abilities.
	setHeroStage(currentLevel == 2 ? 1 : currentLevel);
	cameraX = 0;
	levelIntroTimer = 150;
	bossWoke = 0;
	bossBannerTimer = 0;
	levelPhase = PHASE_ADVANCE;
	player.coins = 0;
	player.kills = 0;

	// Started once, here, when the level begins - never from the draw path
	// and never from a per-frame check, so it cannot restart under itself.
	if (currentLevel == 1)      musicPlay("Audios/level01bg.mp3", "bgm", 1);
	else if (currentLevel == 2) musicPlay("Audios/level02bg.mp3", "bgm", 1);
	else                        musicPlay("Audios/background.mp3", "bgm", 1);
}

// ---------------- losing ----------------

void onPlayerDeath()
{
	lives--;
	if (lives > 0) {
		// stand her back up a little way behind the front
		// inside the arena she stands back up where she fell - there is
		// nowhere to retreat to
		double back = (levelPhase == PHASE_BOSS)
		            ? player.x
		            : clampd(player.x - 260, 120, levelLength - 200);
		int keepStage = player.stage;
		int keepGold  = player.gold;
		resetPlayer(back);
		player.stage = keepStage;
		player.gold  = keepGold;
		setHeroStage(keepStage);
		player.invuln = 120;
		cameraX = clampd(back - SCREEN_WIDTH * 0.36,
		                 (levelPhase == PHASE_BOSS) ? gateX - 260 : 0,
		                 levelLength - SCREEN_WIDTH);
		fxText(player.x, player.y + PLAYER_H + 30, "RISE", COL_GOLD);
	} else {
		musicStop();
		musicPlay("Audios/loosesound.mp3", "over", 0);
		if (score > highScore) highScore = score;
		changeState(STATE_GAMEOVER);
	}
}

// ---------------- one tick ----------------

void updateGame()
{
	int i;

	if (levelIntroTimer > 0) levelIntroTimer--;

	updatePlayer();
	updateUnits();
	updateProjectiles();
	if (currentLevel == 2) {
		updateHydra();
		updateFireballs();
	}
	updateEffects();
	resolveAllCollisions();
	updateLevelPhase();
	updateCamera();

	// the commander's arrival promotes Joan to her final stage
	if (currentLevel == TOTAL_LEVELS && !bossWoke) {
		for (i = 0; i < MAX_UNITS; i++) {
			if (units[i].active && units[i].type == U_COMMANDER && units[i].state != US_SLEEP) {
				bossWoke = 1;
				setHeroStage(4);
				fxText(player.x, player.y + PLAYER_H + 40, "STAGE 4", COL_GOLD);
				fxRing(player.x, player.y + PLAYER_H / 2, COL_GOLD, 300, 50);
				sfx("levelup");
				break;
			}
		}
	}

	// death
	if (player.state == PS_DEAD && player.deathTimer <= 0) {
		onPlayerDeath();
		return;
	}

	// ---- level 02: only the Hydra's death ends it ----
	if (currentLevel == 2) {
		if (levelPhase == PHASE_BOSS && !hydra.alive && hydra.deathTimer <= 0) {
			if (score > highScore) highScore = score;
			musicStop();
			musicPlay("Audios/winsound.mp3", "win", 0);
			changeState(STATE_LEVEL_COMPLETE);
		}
		return;
	}

	// ---- level 01: the banner is a trigger, not a finish ----
	// Reaching it starts the fight; only the commander's death ends it.
	if (currentLevel == 1) {
		if (levelPhase == PHASE_BOSS && !commanderAlive()) {
			if (score > highScore) highScore = score;
			musicStop();
			musicPlay("Audios/winsound.mp3", "win", 0);
			changeState(STATE_LEVEL_COMPLETE);
		}
		return;
	}

	// victory conditions
	if (currentLevel == TOTAL_LEVELS) {
		if (!bossAlive && bossWoke) {
			if (score > highScore) highScore = score;
			musicStop();
			changeState(STATE_LEVEL_COMPLETE);
		}
	} else if (player.x >= gateX) {
		if (score > highScore) highScore = score;
		musicStop();
		sfx("levelup");
		changeState(STATE_LEVEL_COMPLETE);
	}
}

// ---------------- one frame ----------------

void drawGameplay()
{
	glPushMatrix();
	glTranslatef((float)shakeOffX, (float)shakeOffY, 0.0f);

	drawParallaxBackground();
	drawScenery();
	drawGround();
	drawObstacles();
	drawGate();
	drawPickups();
	if (currentLevel == 2) drawHydra();
	drawUnits();
	drawPlayer();
	drawProjectiles();
	if (currentLevel == 2) drawFireballs();
	drawEffects();

	glPopMatrix();

	drawHUD();

	// the commander's arrival
	if (bossBannerTimer > 0) {
		double t = bossBannerTimer / 150.0;
		double a = (t > 0.75) ? (1.0 - t) * 4.0 : (t < 0.25 ? t * 4.0 : 1.0);

		fillRectAlpha(0, SCREEN_HEIGHT / 2 - 54, SCREEN_WIDTH, 108, COL_PANEL, 0.72 * a);
		fillRectAlpha(0, SCREEN_HEIGHT / 2 + 54, SCREEN_WIDTH, 2, COL_RED, a);
		fillRectAlpha(0, SCREEN_HEIGHT / 2 - 56, SCREEN_WIDTH, 2, COL_RED, a);
		setColorMix(COL_PANEL, COL_RED_LIGHT, a);
		drawTextCenteredBold(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 6,
		                     currentLevel == 2 ? "THE HYDRA"
		                                       : "THE ENGLISH COMMANDER", FONT_BIG);
		setColorMix(COL_PANEL, COL_GREY, a);
		drawTextCentered(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 30,
		                 currentLevel == 2 ? "Three heads. Keep moving."
		                                   : "The road closes behind you.", FONT_SMALL);
	}

	// level title card
	if (levelIntroTimer > 0) {
		double t = levelIntroTimer / 150.0;
		double a = (t > 0.75) ? (1.0 - t) * 4.0 : (t < 0.25 ? t * 4.0 : 1.0);
		char buf[64];

		fillRectAlpha(0, SCREEN_HEIGHT / 2 - 70, SCREEN_WIDTH, 140, COL_PANEL, 0.7 * a);
		sprintf(buf, "MISSION %d", currentLevel);
		setColor(COL_GOLD_DIM);
		drawTextCentered(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 26, buf, FONT_SMALL);
		setColor(COL_GOLD);
		drawTextCenteredBold(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 8,
		                     levelName(currentLevel), FONT_BIG);
		setColor(COL_GREY);
		drawTextCentered(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 44,
		                 currentLevel == 2 ? "Loose arrows. The Hydra waits at the end."
		                 : currentLevel == TOTAL_LEVELS ? "Cut down the commander"
		                 : "Fight through to the banner", FONT_SMALL);
	}
}

#endif
