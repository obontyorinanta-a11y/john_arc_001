// -----------------------------------------------------------------
//  Joan of Arc : Banner of Orleans
//  CSE-1200 Software Development I  -  AUST
//
//  iMain.cpp - framework callbacks only.
//  Every callback is a dispatcher; the real work lives in the modules.
//  The iGraphics framework files are never edited.
//
//  Build phase 1: game state machine + menu family.
// -----------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "iGraphics.h"     // must come first, it defines the framework
#include "Config.hpp"
#include "Utils.hpp"
#include "GameState.hpp"
#include "Campaign.hpp"
#include "Input.hpp"
#include "Assets.hpp"
#include "UI.hpp"
#include "Audio.hpp"
#include "World.hpp"
#include "Effects.hpp"
#include "Projectiles.hpp"
#include "Player.hpp"
#include "Units.hpp"
#include "Level02.hpp"
#include "Combat.hpp"
#include "HUD.hpp"
#include "Game.hpp"
#include "Screens.hpp"

// ---------------- rendering ----------------
// Called continuously by the framework's idle loop. Drawing only:
// never move or change anything from here.

void iDraw()
{
	iClear();
	updateCurrentHover();
	drawCurrentScreen();
}

// ---------------- simulation clock (20 ms) ----------------
// UI time always advances so menus keep animating; gameplay physics
// (added in later phases) only runs while STATE_PLAYING.

void gameUpdate()
{
	uiTime++;
	if (fadeTimer > 0) fadeTimer--;

	if (gameState != STATE_PLAYING) return;

	updateGame();
}

// ---------------- animation clock (120 ms) ----------------

void animationTick()
{
	animFrame++;
}

// ---------------- input (16 ms, framework driven) ----------------
// Reads the held-key arrays, turns them into single presses, then
// snapshots the keyboard for the next frame. Nothing else belongs here.

void fixedUpdate()
{
	handleCurrentKeys();
	inputEndFrame();
}

// ---------------- mouse ----------------
// The framework already flips y, so these coordinates match the drawing
// coordinates directly (origin is bottom-left).

void iMouse(int button, int state, int mx, int my)
{
	if (state != GLUT_DOWN) return;

	// in the field the mouse is a weapon, everywhere else it is a pointer
	if (gameState == STATE_PLAYING) {
		if (button == GLUT_LEFT_BUTTON && player.swingCooldown <= 0) {
			player.swingTimer    = SWING_TICKS;
			player.swingCooldown = SWING_COOLDOWN;
			player.swingId++;
			sfx("swing");
		}
		if (button == GLUT_RIGHT_BUTTON && player.bowCooldown <= 0 && player.stage >= 2) {
			spawnArrow(player.x + player.facing * 26, player.y + PLAYER_H * 0.55,
			           player.facing, SIDE_PLAYER, playerArrowDamage());
			player.bowCooldown = BOW_COOLDOWN;
			sfx("arrow");
		}
		return;
	}

	if (button == GLUT_LEFT_BUTTON) handleCurrentClick(mx, my);
}

void iMouseMove(int mx, int my)
{
	// drag handling - unused for now
}

void iPassiveMouseMove(int mx, int my)
{
	// hover is read from iMouseX / iMouseY inside iDraw
}

// ---------------- entry point ----------------

int main()
{
	srand((unsigned int)time(0));

	// timers must be registered before the window is created
	iSetTimer(LOGIC_TICK_MS, gameUpdate);
	iSetTimer(ANIM_TICK_MS, animationTick);

	// 4th argument = keyboard sampling rate. The framework polls the key
	// arrays this often and calls fixedUpdate(); a shorter interval means
	// a very quick tap is less likely to be missed between two polls.
	iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT,
	            (char*)"Joan of Arc - Banner of Orleans", KEY_POLL_MS);

	// textures need a live OpenGL context, so this must sit between
	// iInitialize() and iStart()
	loadAllAssets();

	iStart();
	return 0;
}
