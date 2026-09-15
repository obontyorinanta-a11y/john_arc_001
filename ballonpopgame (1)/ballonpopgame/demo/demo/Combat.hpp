#ifndef COMBAT_HPP
#define COMBAT_HPP

// -----------------------------------------------------------------
//  Combat.hpp - every collision in the game lives in this one file.
//
//      sword     vs units and barricades
//      arrows    vs units, player and barricades
//      enemy hit vs player and companions
//      player    vs pickups
//      abilities that touch more than one system
//
//  Keeping it in one place means there is exactly one file to read
//  when something takes damage that should not have.
// -----------------------------------------------------------------

#include "Config.hpp"
#include "Utils.hpp"
#include "GameState.hpp"
#include "Audio.hpp"
#include "World.hpp"
#include "Effects.hpp"
#include "Projectiles.hpp"
#include "Player.hpp"
#include "Units.hpp"

// ---------------- sword ----------------

void resolveSword()
{
	int i;
	Rect blade;

	if (player.swingTimer <= 0) return;
	blade = swordRect();

	for (i = 0; i < MAX_UNITS; i++) {
		if (!units[i].active || units[i].state == US_DEAD) continue;
		if (units[i].side != SIDE_ENEMY) continue;
		if (units[i].lastHitSwing == player.swingId) continue;   // one hit per swing
		if (!rectsOverlap(blade, unitRect(i))) continue;

		units[i].lastHitSwing = player.swingId;
		damageUnit(i, playerSwordDamage(), 0);
		shakeScreen(4);

		// knock the lighter ranks back
		if (units[i].type != U_COMMANDER)
			units[i].x += player.facing * (units[i].type == U_VETERAN ? 6 : 14);
	}

	// barricades break under the blade too
	for (i = 0; i < MAX_OBSTACLES; i++) {
		if (!obstacles[i].active) continue;
		if (!rectsOverlap(blade, obstacleRect(i))) continue;
		if (obstacles[i].hitFlash > 0) continue;

		obstacles[i].hp -= playerSwordDamage();
		obstacles[i].hitFlash = 10;
		fxSpark(obstacles[i].x + obstacles[i].w / 2,
		        obstacles[i].y + obstacles[i].h, COL_GOLD_DIM, 6);

		if (obstacles[i].hp <= 0) {
			obstacles[i].active = 0;
			fxDust(obstacles[i].x + obstacles[i].w / 2, obstacles[i].y + 10, 12);
			score += 5;
		}
	}
}

// ---------------- projectiles ----------------

void resolveProjectiles()
{
	int p, i;

	for (p = 0; p < MAX_PROJ; p++) {
		if (!projectiles[p].active) continue;
		Rect pr = projRect(p);

		if (projectiles[p].side == SIDE_PLAYER) {
			for (i = 0; i < MAX_UNITS; i++) {
				if (!units[i].active || units[i].state == US_DEAD) continue;
				if (units[i].side != SIDE_ENEMY) continue;
				if (!rectsOverlap(pr, unitRect(i))) continue;

				damageUnit(i, projectiles[p].damage, 1);
				projectiles[p].active = 0;
				break;
			}
		} else {
			// enemy fire hits Joan
			if (rectsOverlap(pr, playerRect())) {
				damagePlayer(projectiles[p].damage);
				projectiles[p].active = 0;
				continue;
			}
			// and her companions
			for (i = 0; i < MAX_UNITS; i++) {
				if (!units[i].active || units[i].state == US_DEAD) continue;
				if (units[i].side != SIDE_PLAYER) continue;
				if (!rectsOverlap(pr, unitRect(i))) continue;

				damageUnit(i, projectiles[p].damage, 1);
				projectiles[p].active = 0;
				break;
			}
		}
	}
}

// ---------------- melee from units ----------------
// A unit lands its blow on the tick its windup reaches zero, which the
// unit update signals by leaving attackTimer at 0 with state US_ATTACK.

void resolveUnitAttacks()
{
	int i, k;

	for (i = 0; i < MAX_UNITS; i++) {
		if (!units[i].active || units[i].state == US_DEAD) continue;
		if (units[i].state != US_ATTACK || units[i].attackTimer != 1) continue;
		if (units[i].type == U_ARCHER) continue;                     // ranged

		double reach = unitReachOf(units[i].type);
		Rect swing = makeRect(units[i].x + (units[i].facing > 0 ? 0 : -reach),
		                      units[i].y + 10, reach, units[i].h - 20);

		if (units[i].side == SIDE_ENEMY) {
			if (rectsOverlap(swing, playerRect()))
				damagePlayer(unitDamageOf(units[i].type));
		} else {
			int dmg = unitDamageOf(U_COMPANION);
			// companions fight harder near their commander
			if (fabs(units[i].x - player.x) < ALLY_AURA) dmg = (int)(dmg * 1.5);

			for (k = 0; k < MAX_UNITS; k++) {
				if (!units[k].active || units[k].state == US_DEAD) continue;
				if (units[k].side != SIDE_ENEMY) continue;
				if (!rectsOverlap(swing, unitRect(k))) continue;
				damageUnit(k, dmg, 0);
				break;
			}
		}
	}
}

// ---------------- body contact ----------------
// Standing inside an enemy hurts, so the player cannot simply walk through.

void resolveContact()
{
	int i;
	Rect pr = playerRect();

	for (i = 0; i < MAX_UNITS; i++) {
		if (!units[i].active || units[i].state == US_DEAD) continue;
		if (units[i].side != SIDE_ENEMY) continue;
		if (!rectsOverlap(pr, unitRect(i))) continue;

		if (player.invuln <= 0)
			damagePlayer(clampi(unitDamageOf(units[i].type) / 3, 3, 10));

		// push apart
		if (player.x < units[i].x) { player.x -= 3; units[i].x += 1; }
		else                       { player.x += 3; units[i].x -= 1; }
	}
}

// ---------------- pickups ----------------

// Coins and health boxes. Both are collected by walking through them -
// there is no pick-up button.
void resolvePickups()
{
	int i;
	int gained;
	char buf[24];
	Rect pr = playerRect();

	for (i = 0; i < MAX_PICKUPS; i++) {
		if (!pickups[i].active) continue;

		// A health box is NOT consumed at full health. Joan walks over it and
		// it stays on the road, which is the single rule that stops the most
		// annoying bug in this genre - burning a heal you did not need.
		if (pickups[i].type == PU_HPBOX && player.hp >= player.maxHp) continue;

		if (!rectsOverlap(pr, pickupRect(i))) continue;

		if (pickups[i].type == PU_COIN) {
			score += SCORE_COIN;
			player.coins++;
			player.gold++;
			fxNumber(pickups[i].x, pickups[i].y + 20, SCORE_COIN, COL_GOLD);
			sfx("coin");
		} else {
			gained = healPlayer(HP_BOX_HEAL);
			sprintf(buf, "+%d HP", gained);
			fxText(pickups[i].x, pickups[i].y + 26, buf, COL_RED_LIGHT);
			fxRing(player.x, player.y + PLAYER_H * 0.5, COL_RED_LIGHT, 130, 28);
			fxSpark(pickups[i].x, pickups[i].y, COL_RED_LIGHT, 8);
			sfx("heal");
		}
		pickups[i].active = 0;
	}
}

// ---------------- abilities ----------------

void resolveAbilities()
{
	int i;

	// Sacred Mending
	if (player.castHeal) {
		player.castHeal = 0;
		player.healCooldown = HEAL_COOLDOWN;
		healPlayer(player.maxHp / 6);
		fxRing(player.x, player.y + PLAYER_H / 2, COL_GOLD, 200, 40);
		fxText(player.x, player.y + PLAYER_H + 26, "MENDING", COL_GOLD);
		sfx("heal");

		for (i = 0; i < MAX_UNITS; i++) {
			if (!units[i].active || units[i].state == US_DEAD) continue;
			if (units[i].side != SIDE_PLAYER) continue;
			if (fabs(units[i].x - player.x) > ALLY_AURA) continue;
			units[i].hp = clampi(units[i].hp + 30, 0, units[i].maxHp);
			fxRing(units[i].x, units[i].y + units[i].h / 2, COL_GOLD, 90, 24);
		}
	}

	// Oriflamme Call
	if (player.castSummon) {
		player.castSummon = 0;
		player.summonCooldown = SUMMON_COOLDOWN;
		int n = (player.stage >= 4) ? 3 : 2;
		for (i = 0; i < n; i++) {
			Unit* u = spawnUnit(U_COMPANION, SIDE_PLAYER,
			                    player.x - player.facing * (60 + i * 46));
			if (u) fxRing(u->x, u->y + 40, COL_BLUE, 120, 30);
		}
		fxText(player.x, player.y + PLAYER_H + 26, "TO ME!", COL_BLUE);
		sfx("summon");
	}

	// the stage four ultimate
	if (player.castUlt) {
		player.castUlt = 0;
		player.ultCooldown = ULT_COOLDOWN;
		fxRing(player.x, player.y + PLAYER_H / 2, COL_GOLD, 520, 70);
		fxRing(player.x, player.y + PLAYER_H / 2, COL_WHITE, 420, 55);
		fxText(player.x, player.y + PLAYER_H + 30, "FOR FRANCE", COL_GOLD);
		shakeScreen(18);
		sfx("ult");

		for (i = 0; i < MAX_UNITS; i++) {
			if (!units[i].active || units[i].state == US_DEAD) continue;
			if (units[i].side != SIDE_ENEMY) continue;
			if (fabs(units[i].x - player.x) > 520) continue;
			damageUnit(i, 90 + player.stage * 12, 0);
		}
		// and clears incoming fire
		for (i = 0; i < MAX_PROJ; i++)
			if (projectiles[i].active && projectiles[i].side == SIDE_ENEMY)
				projectiles[i].active = 0;
	}
}

// ---------------- camera ----------------

void updateCamera()
{
	double want = player.x - SCREEN_WIDTH * 0.36;
	double lo   = 0;
	double hi   = levelLength - SCREEN_WIDTH;

	// During the boss fight the camera will not travel back down the road,
	// which is what makes the arena read as a closed space.
	//
	// The arena sits at the very END of the level, so this lower bound can
	// legitimately land past the upper one: on level 01 it asked for 5640
	// while the camera can only ever reach 5200. clampd() then returned the
	// floor for anything under 5640 and the ceiling for anything over 5200,
	// which is every possible value - so the camera flipped 440 pixels back
	// and forth once per frame and the whole screen juddered. The clamp has
	// to be collapsed, not just applied.
	if (levelPhase == PHASE_BOSS) {
		lo = gateX - 260;
		if (lo > hi) lo = hi;
	}

	cameraX += (want - cameraX) * 0.12;          // smooth follow
	cameraX = clampd(cameraX, lo, hi);
}

// ---------------- one whole frame of collisions ----------------

void resolveAllCollisions()
{
	// Level 02 is an archery level: the bow replaces the sword, the Hydra
	// is a target the unit array knows nothing about, and its fire is a
	// projectile pool of its own.
	if (currentLevel == 2) {
		resolveProjectiles();       // arrows vs the foot enemies
		resolveArrowsVsHydra();
		resolveFireVsPlayer();
		resolveUnitAttacks();
		resolveContact();
		resolvePickups();
		return;
	}

	resolveSword();
	resolveProjectiles();
	resolveUnitAttacks();
	resolveContact();
	resolvePickups();
	resolveAbilities();
}

#endif
