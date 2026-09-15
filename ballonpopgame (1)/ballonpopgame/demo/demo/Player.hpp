#ifndef PLAYER_HPP
#define PLAYER_HPP

// -----------------------------------------------------------------
//  Player.hpp - Joan herself.
//  Movement, gravity, jumping, the sword swing, the bow, health and
//  the four stage hero progression.
//
//  Abilities only raise a flag here.  Combat.hpp performs the part
//  that touches other systems (healing allies, summoning companions),
//  which keeps this file free of dependencies on the unit array.
// -----------------------------------------------------------------

#include <math.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "GameState.hpp"
#include "Input.hpp"
#include "Assets.hpp"
#include "Audio.hpp"
#include "World.hpp"
#include "Effects.hpp"
#include "Projectiles.hpp"

enum PlayerState { PS_IDLE = 0, PS_RUN, PS_JUMP, PS_ATTACK, PS_HURT, PS_DEAD };
enum HeroPath    { PATH_NONE = 0, PATH_BLADE, PATH_BOW };

struct Player {
	double x, y;
	double vx, vy;
	int facing;            // +1 right, -1 left
	int onGround;
	int state;

	int hp, maxHp;
	int invuln;

	int swingTimer;        // >0 while the blade is out
	int swingCooldown;
	int swingId;           // rises every swing so one swing hits once
	int bowCooldown;

	int healCooldown;
	int summonCooldown;
	int ultCooldown;

	int castHeal;          // flags consumed by Combat.hpp
	int castSummon;
	int castUlt;

	int stage;             // 1..4, the Path of the Maid
	int gold;
	int deathTimer;

	int shootTimer;        // >0 while the draw-and-release animation runs
	int shootCooldown;     // stops F from firing every frame while held
	int arrowFired;        // one arrow per draw, latched on the release frame
	int arrowsFired;       // shown on the HUD

	int coins;             // level 01 coin counter, shown on the HUD
	int kills;             // shown on the mission complete panel
	int autoAdvance;       // 1 while she is walking forward on her own
	double hpBarShown;     // HUD bar chases hp instead of snapping to it

	double runPhase;
};

Player player;

// Body-box fractions of the exported art, from tools/measure_art.py.
// The twelve run frames are within a pixel of each other, so one set covers
// the whole cycle - and because the anchor is constant, the small up and down
// the artist drew INTO the frames still reads as the run's bounce.
// Level 02 archer. All ten source frames share one 356x386 canvas, so a
// single set of fractions covers both animations and the pose can change
// mid-stride without the figure moving.
const CharArt ART_ARCHER_RUN     = { 0.6859, 0.9047, 0.4851 };
const CharArt ART_ARCHER_SHOOT   = { 0.6973, 0.9131, 0.4571 };

const CharArt ART_JOAN_RUN       = { 0.8145, 0.8653, 0.5766 };
const CharArt ART_JOAN_SWORDUP   = { 0.8151, 0.9193, 0.4787 };
const CharArt ART_JOAN_SWORDDOWN = { 0.9510, 0.9860, 0.4504 };

Rect playerRect()
{
	return makeRect(player.x - PLAYER_W / 2, player.y, PLAYER_W, PLAYER_H);
}

// the arc the sword sweeps while swingTimer is running
Rect swordRect()
{
	double reach = SWORD_REACH * (heroPath == PATH_BLADE ? 1.15 : 1.0);
	double x = (player.facing > 0) ? player.x + 8 : player.x - 8 - reach;
	return makeRect(x, player.y + 18, reach, PLAYER_H - 34);
}

int playerSwordDamage()
{
	int d = SWORD_DAMAGE;
	if (heroPath == PATH_BLADE) d = (int)(d * 1.45);
	if (heroPath == PATH_BOW)   d = (int)(d * 0.75);
	d += (player.stage - 1) * 6;
	return d;
}

int playerArrowDamage()
{
	int d = ARROW_DAMAGE;
	if (heroPath == PATH_BOW) d = (int)(d * 1.5);
	d += (player.stage - 1) * 4;
	return d;
}

double playerSpeed()
{
	double s = PLAYER_SPEED;
	if (heroPath == PATH_BOW) s += 0.7;
	if (player.stage >= 3)    s += 0.6;
	return s;
}

void resetPlayer(double startX)
{
	player.x = startX;
	player.y = GROUND_Y;
	player.vx = player.vy = 0;
	player.facing = 1;
	player.onGround = 1;
	player.state = PS_IDLE;
	player.maxHp = PLAYER_MAX_HP + (player.stage - 1) * 20;
	player.hp = player.maxHp;
	player.invuln = 0;
	player.swingTimer = 0;
	player.swingCooldown = 0;
	player.swingId = 0;
	player.bowCooldown = 0;
	player.healCooldown = 0;
	player.summonCooldown = 0;
	player.ultCooldown = 0;
	player.castHeal = player.castSummon = player.castUlt = 0;
	player.deathTimer = 0;
	player.runPhase = 0;
	player.autoAdvance = 1;
	player.hpBarShown = player.hp;
	player.shootTimer = 0;
	player.shootCooldown = 0;
	player.arrowFired = 0;
}

void newRunPlayer()
{
	player.stage = 1;
	player.gold  = 0;
	player.coins = 0;
	player.kills = 0;
	player.arrowsFired = 0;
	resetPlayer(140);
}

// stage rises between levels: 1 -> 2 after the path is chosen, then 3, then 4
void setHeroStage(int s)
{
	player.stage = clampi(s, 1, 4);
	player.maxHp = PLAYER_MAX_HP + (player.stage - 1) * 20;
	if (player.hp > player.maxHp) player.hp = player.maxHp;
}

void damagePlayer(int amount)
{
	if (player.invuln > 0 || player.state == PS_DEAD) return;

	player.hp -= amount;
	player.invuln = PLAYER_INVULN;
	fxNumber(player.x, player.y + PLAYER_H, amount, COL_RED);
	fxSpark(player.x, player.y + PLAYER_H * 0.5, COL_RED, 8);
	shakeScreen(7);

	if (player.hp <= 0) {
		player.hp = 0;
		player.state = PS_DEAD;
		player.deathTimer = 90;
		sfx("death");
	} else {
		player.state = PS_HURT;
		player.vx = -player.facing * 4.0;
		sfx("hurt");
	}
}

// Returns the amount that was actually absorbed, which is what the caller
// needs in order to tell the player how much healing the cap threw away.
int healPlayer(int amount)
{
	int room, gained;

	if (player.state == PS_DEAD) return 0;

	room   = player.maxHp - player.hp;
	gained = (amount < room) ? amount : room;
	if (gained < 0) gained = 0;

	player.hp += gained;
	return gained;
}

// Joan carries a bow on the Montclair and a sword everywhere else. One
// predicate, so nothing else in the file has to know which level it is.
int playerHasBow()
{
	return (currentLevel == 2);
}

// ---------------- input ----------------
// Called from fixedUpdate. Only reads keys and sets intentions.

void readPlayerInput()
{
	int left, right;

	if (player.state == PS_DEAD) return;

	left  = keyHeld('a') || keyHeld('A') || specialHeld(GLUT_KEY_LEFT);
	right = keyHeld('d') || keyHeld('D') || specialHeld(GLUT_KEY_RIGHT);

	player.vx = 0;
	if (left && !right)  { player.vx = -playerSpeed(); player.facing = -1; }
	if (right && !left)  { player.vx =  playerSpeed(); player.facing =  1; }

	// Joan marches on her own, but only while nobody is holding a key and
	// only while no enemy is close - autoAdvance is cleared by Game.hpp the
	// moment one is, so she never walks onto a raised sword. A held key
	// always wins, so the player is never fighting the autopilot.
	if (!left && !right && player.autoAdvance && player.swingTimer <= 0) {
		player.vx     = PLAYER_AUTO_SPEED;
		player.facing = 1;
	}

	// jump
	if ((keyTapped(KEY_SPACE) || keyTappedCh('w', 'W') || specialTapped(GLUT_KEY_UP))
	    && player.onGround) {
		player.vy = PLAYER_JUMP;
		player.onGround = 0;
		fxDust(player.x, GROUND_Y, 6);
		sfx("jump");
	}

	// bow - F on level 02. keyTapped is a rising edge, and shootCooldown is
	// a second guard, so holding F cannot spray arrows every frame.
	if (playerHasBow()) {
		if (keyTappedCh('f', 'F')
		    && player.shootCooldown <= 0 && player.shootTimer <= 0) {
			player.shootTimer   = SHOOT_TICKS;
			player.shootCooldown = SHOOT_TICKS + ARROW_COOLDOWN;
			player.arrowFired   = 0;
		}
		return;             // no sword, no abilities on this level
	}

	// sword
	if (keyTappedCh('j', 'J') && player.swingCooldown <= 0) {
		player.swingTimer   = SWING_TICKS;
		player.swingCooldown = SWING_COOLDOWN;
		player.swingId++;
		sfx("swing");
	}

	// bow, unlocked once a path is chosen
	if (keyTappedCh('k', 'K') && player.bowCooldown <= 0 && player.stage >= 2) {
		spawnArrow(player.x + player.facing * 26, player.y + PLAYER_H * 0.55,
		           player.facing, SIDE_PLAYER, playerArrowDamage());
		player.bowCooldown = BOW_COOLDOWN - (heroPath == PATH_BOW ? 10 : 0);
		sfx("arrow");
	}

	// abilities
	if (keyTappedCh('q', 'Q') && player.healCooldown <= 0 && player.stage >= 2)
		player.castHeal = 1;

	if (keyTappedCh('e', 'E') && player.summonCooldown <= 0 && player.stage >= 3)
		player.castSummon = 1;

	if (keyTappedCh('r', 'R') && player.ultCooldown <= 0 && player.stage >= 4)
		player.castUlt = 1;
}

// ---------------- simulation ----------------

void updatePlayer()
{
	int i;
	Rect pr;

	if (player.state == PS_DEAD) {
		if (player.deathTimer > 0) player.deathTimer--;
		return;
	}

	// timers
	if (player.swingTimer   > 0) player.swingTimer--;
	if (player.swingCooldown > 0) player.swingCooldown--;
	if (player.shootCooldown > 0) player.shootCooldown--;

	if (player.shootTimer > 0) {
		player.shootTimer--;

		// The arrow leaves on the release frame, not on the key press, so
		// the shot the player sees is the shot that exists in the world.
		if (!player.arrowFired && player.shootTimer <= SHOOT_TICKS / 4) {
			player.arrowFired = 1;
			player.arrowsFired++;
			spawnArrow(player.x + player.facing * 34,
			           player.y + PLAYER_H * 0.62,
			           player.facing, SIDE_PLAYER, ARROW_DAMAGE_L2);
			sfx("arrow");
		}
	}
	if (player.bowCooldown  > 0) player.bowCooldown--;
	if (player.invuln       > 0) player.invuln--;
	if (player.healCooldown > 0) player.healCooldown--;
	if (player.summonCooldown > 0) player.summonCooldown--;
	if (player.ultCooldown  > 0) player.ultCooldown--;

	// horizontal move, then push out of barricades
	player.x += player.vx;

	pr = playerRect();
	for (i = 0; i < MAX_OBSTACLES; i++) {
		if (!obstacles[i].active) continue;
		if (!rectsOverlap(pr, obstacleRect(i))) continue;
		// only block if she is not above it
		if (player.y + 6 >= obstacles[i].y + obstacles[i].h) continue;

		if (player.vx > 0) player.x = obstacles[i].x - PLAYER_W / 2;
		else if (player.vx < 0) player.x = obstacles[i].x + obstacles[i].w + PLAYER_W / 2;
		pr = playerRect();
	}

	player.x = clampd(player.x, PLAYER_W / 2, levelLength - PLAYER_W / 2);

	// gravity
	player.vy -= GRAVITY;
	if (player.vy < -TERMINAL_V) player.vy = -TERMINAL_V;
	player.y += player.vy;

	// land on a barricade
	player.onGround = 0;
	pr = playerRect();
	for (i = 0; i < MAX_OBSTACLES; i++) {
		if (!obstacles[i].active) continue;
		double top = obstacles[i].y + obstacles[i].h;
		if (player.vy <= 0 &&
		    player.x + PLAYER_W / 2 > obstacles[i].x &&
		    player.x - PLAYER_W / 2 < obstacles[i].x + obstacles[i].w &&
		    player.y <= top && player.y > top - 22) {
			player.y = top;
			player.vy = 0;
			player.onGround = 1;
		}
	}

	// land on the ground
	if (player.y <= GROUND_Y) {
		if (!player.onGround && player.vy < -6) fxDust(player.x, GROUND_Y, 5);
		player.y = GROUND_Y;
		player.vy = 0;
		player.onGround = 1;
	}

	// animation state
	if (player.swingTimer > 0)        player.state = PS_ATTACK;
	else if (!player.onGround)        player.state = PS_JUMP;
	else if (fabs(player.vx) > 0.1)   player.state = PS_RUN;
	else                              player.state = PS_IDLE;

	if (player.state == PS_RUN) player.runPhase += 0.22;
	else                        player.runPhase *= 0.85;

	// the HUD bar slides toward the real value so a big kill reward reads as
	// motion rather than as a number that simply changed
	player.hpBarShown += (player.hp - player.hpBarShown) * 0.16;
	if (fabs(player.hpBarShown - player.hp) < 1.0) player.hpBarShown = player.hp;
}

// ---------------- drawing ----------------

void drawPlayer()
{
	double sx    = worldToScreenX(player.x);
	double alpha = 1.0;
	double lean  = 0;
	int    flip  = (player.facing > 0) ? 0 : 1;   // the art is drawn facing right
	int    id;
	CharArt art;

	if (player.state == PS_DEAD) {
		double t = player.deathTimer / 90.0;
		alpha = 0.25 + 0.75 * t;
	} else if (player.invuln > 0 && (player.invuln / 4) % 2 == 0) {
		alpha = 0.45;
	}

	// shadow
	fillRectAlpha(sx - 26, GROUND_Y - 3, 52, 7, COL_PANEL, 0.45);

	// ---- level 02: the archer ----
	// Drawing and shooting are the same sprite sheet, so the changeover is
	// just a different animation and the figure never jumps.
	if (playerHasBow()) {
		if (player.shootTimer > 0) {
			// walk the four shoot frames evenly across SHOOT_TICKS:
			// lift, nock, full draw, release
			int f = (SHOOT_TICKS - player.shootTimer) * 4 / SHOOT_TICKS;
			if (f > 3) f = 3;
			if (f < 0) f = 0;
			id  = SPR_PLAYER_ARCHER_SHOOT_00 + f;
			art = ART_ARCHER_SHOOT;
		} else if (player.state == PS_RUN) {
			id  = animSprite(ANIM_PLAYER_ARCHER_RUN, animFrame);
			art = ART_ARCHER_RUN;
		} else if (player.state == PS_JUMP) {
			id  = SPR_PLAYER_ARCHER_RUN_02;
			art = ART_ARCHER_RUN;
			lean = 6;
		} else {
			id  = SPR_PLAYER_ARCHER_RUN_00;
			art = ART_ARCHER_RUN;
		}

		drawCharacter(id, art, sx, player.y + lean, ARCHER_ART_H, flip, alpha);
		return;
	}

	// ---- pose ----
	// A swing is two drawings, not one: the blade goes up for the first half
	// of SWING_TICKS and comes down for the second. The changeover lands on
	// the same tick Combat.hpp starts testing the blade box, so the strike
	// the player sees is the strike that does the damage.
	if (player.swingTimer > 0) {
		if (player.swingTimer > SWING_TICKS / 2) {
			id  = SPR_PLAYER_JOAN_SWORDUP;
			art = ART_JOAN_SWORDUP;
		} else {
			id  = SPR_PLAYER_JOAN_SWORDDOWN;
			art = ART_JOAN_SWORDDOWN;
		}
	} else if (player.state == PS_RUN) {
		// animFrame ticks every ANIM_TICK_MS (120 ms) - twelve frames make
		// one stride in 1.44 s
		id  = animSprite(ANIM_PLAYER_JOAN_RUN, animFrame);
		art = ART_JOAN_RUN;
	} else if (player.state == PS_JUMP) {
		id   = SPR_PLAYER_JOAN_RUN_04;      // mid-stride reads well in the air
		art  = ART_JOAN_RUN;
		lean = 6;
	} else {
		id  = SPR_PLAYER_JOAN_RUN_00;
		art = ART_JOAN_RUN;
	}

	drawCharacter(id, art, sx, player.y + lean, PLAYER_ART_H, flip, alpha);

	// a bright arc through the swing, so the reach of the blade is readable
	if (player.swingTimer > 0) {
		double t     = 1.0 - (double)player.swingTimer / SWING_TICKS;
		double reach = SWORD_REACH;
		double a0    = (player.facing > 0) ? -0.9 + t * 2.0 : 3.9 - t * 2.0;
		double cx    = sx + player.facing * 10;
		double cy    = player.y + PLAYER_H * 0.52;
		int k;

		beginBlend();
		for (k = 0; k < 5; k++) {
			double a  = a0 - player.facing * k * 0.12;
			double x1 = cx + cos(a) * 18;
			double y1 = cy + sin(a) * 18;
			double x2 = cx + cos(a) * reach;
			double y2 = cy + sin(a) * reach;
			glColor4f(1.0f, 0.95f, 0.75f, (float)(0.42 - k * 0.075));
			iLine(x1, y1, x2, y2);
		}
		endBlend();
	}
}

#endif
