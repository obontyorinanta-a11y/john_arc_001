#ifndef WORLD_HPP
#define WORLD_HPP

// -----------------------------------------------------------------
//  World.hpp - the level itself.
//
//  Holds the camera, the scrolling background, the barricades and the
//  pickups, and builds each level's contents.  Everything is stored in
//  world coordinates; only the drawing subtracts cameraX.
// -----------------------------------------------------------------

#include <math.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "GameState.hpp"
#include "Assets.hpp"
#include "Audio.hpp"

// ---------------- camera ----------------

double cameraX = 0;

double worldToScreenX(double wx) { return wx - cameraX; }

int onScreen(double wx, double margin)
{
	return (wx > cameraX - margin && wx < cameraX + SCREEN_WIDTH + margin);
}

// ---------------- obstacles ----------------

enum ObstacleType { OB_BARRICADE = 0, OB_CRATE };

struct Obstacle {
	int active;
	int type;
	double x, y, w, h;
	int hp, maxHp;
	int hitFlash;
};

Obstacle obstacles[MAX_OBSTACLES];

void clearObstacles()
{
	int i;
	for (i = 0; i < MAX_OBSTACLES; i++) obstacles[i].active = 0;
}

void addObstacle(int type, double x, double w, double h, int hp)
{
	int i;
	for (i = 0; i < MAX_OBSTACLES; i++) {
		if (!obstacles[i].active) {
			obstacles[i].active   = 1;
			obstacles[i].type     = type;
			obstacles[i].x        = x;
			obstacles[i].y        = GROUND_Y;
			obstacles[i].w        = w;
			obstacles[i].h        = h;
			obstacles[i].hp       = hp;
			obstacles[i].maxHp    = hp;
			obstacles[i].hitFlash = 0;
			return;
		}
	}
}

Rect obstacleRect(int i)
{
	return makeRect(obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h);
}

// ---------------- pickups ----------------

enum PickupType { PU_COIN = 0, PU_HPBOX };

struct Pickup {
	int active;
	int type;
	double x, y;
	double phase;
};

Pickup pickups[MAX_PICKUPS];

void clearPickups()
{
	int i;
	for (i = 0; i < MAX_PICKUPS; i++) pickups[i].active = 0;
}

void addPickup(int type, double x, double y)
{
	int i;
	for (i = 0; i < MAX_PICKUPS; i++) {
		if (!pickups[i].active) {
			pickups[i].active = 1;
			pickups[i].type   = type;
			pickups[i].x      = x;
			pickups[i].y      = y;
			pickups[i].phase  = (double)(i * 37 % 100) / 15.0;
			return;
		}
	}
}

Rect pickupRect(int i)
{
	return makeRect(pickups[i].x - 14, pickups[i].y - 14, 28, 28);
}

// ---------------- level ----------------

// The brief names six game states. Three of them - PLAYING, COMBAT and
// BOSS_FIGHT - all draw the same battlefield and differ only in music,
// camera and HUD, so making them full screen states would mean three
// near-identical copies of drawGameplay() that would drift apart within a
// week. They are a phase INSIDE STATE_PLAYING instead: same names, same
// behaviour, one draw function.
enum LevelPhase {
	PHASE_ADVANCE = 0,   // "PLAYING"    - road clear, Joan marches on her own
	PHASE_COMBAT,        // "COMBAT"     - an enemy is close, autopilot off
	PHASE_BOSS           // "BOSS_FIGHT" - past the gate, arena shut, one way
};

int levelPhase = PHASE_ADVANCE;

double levelLength   = 5200;
int    levelKills    = 0;      // enemies killed this level
int    levelKillGoal = 0;      // how many are placed in the level
int    bossAlive     = 0;
double gateX         = 0;      // the banner that ends the level

// per level colour grade applied to the background art
double levelTintR = 1.0, levelTintG = 1.0, levelTintB = 1.0;

void setLevelMood(int level)
{
	if (level == 1) {         // clear morning
		levelTintR = 1.00; levelTintG = 1.00; levelTintB = 1.00;
	} else if (level == 2) {  // the Montclair burns at dusk
		levelTintR = 1.00; levelTintG = 0.92; levelTintB = 0.86;
	} else {                  // the commander's field, storm light
		levelTintR = 0.62; levelTintG = 0.56; levelTintB = 0.72;
	}
}

// ---------------- background ----------------

void drawParallaxBackground()
{
	// Level 01 pans across ONE copy of the painting instead of tiling it.
	//
	// The tiling path below mirrors every second copy so the seams meet, and
	// on a plain landscape that works. This painting has a castle on a hill
	// and a row of standards, so a mirrored copy reads as an obvious fold
	// down the middle of the screen. Blowing the art up to 1.42x screen
	// height makes it wide enough to cover the camera's whole travel in a
	// single copy: no repeat, no seam, and the pan still gives depth.
	// Level 02 gets the same single-copy pan, with its own painting.
	if (currentLevel == 2) {
		double bH   = SCREEN_HEIGHT * 1.30;
		double bW   = bH * 1492.0 / 1054.0;          // the art's own aspect
		double span = levelLength - SCREEN_WIDTH;
		double t    = (span > 1.0) ? clampd(cameraX / span, 0.0, 1.0) : 0.0;

		drawSpriteTint(SPR_BACKGROUND_MONTCLAIR,
		               -t * (bW - SCREEN_WIDTH),
		               -(bH - SCREEN_HEIGHT) * 0.58,
		               bW, bH, 0, 1.0,
		               levelTintR, levelTintG, levelTintB);

		fillRectAlpha(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COL_PANEL, 0.14);
		return;
	}

	if (currentLevel == 1) {
		double bH   = SCREEN_HEIGHT * 1.42;
		double bW   = bH * 2.0;                       // the source art is 2:1
		double span = levelLength - SCREEN_WIDTH;
		double t    = (span > 1.0) ? clampd(cameraX / span, 0.0, 1.0) : 0.0;

		drawSpriteTint(SPR_BACKGROUND_FIELD_ORLEANS,
		               -t * (bW - SCREEN_WIDTH),
		               -(bH - SCREEN_HEIGHT) * 0.62,   // keeps the horizon high
		               bW, bH, 0, 1.0,
		               levelTintR, levelTintG, levelTintB);

		fillRectAlpha(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COL_BLUE_DARK, 0.10);
		return;
	}

	// The battlefield painting is drawn as a wide band above the ground,
	// scrolling slower than the player so distance reads as distance.
	double bandH = SCREEN_HEIGHT;
	double bandW = bandH * 2.0;                 // source art is 2:1
	double par   = cameraX * 0.35;
	double startX = -fmod(par, bandW * 2) - bandW;
	double x;
	int tile = 0;

	// The painting does not tile, so every second copy is mirrored.
	// Mirrored copies meet edge to edge and the seam stops being visible.
	for (x = startX; x < SCREEN_WIDTH + bandW; x += bandW - 1) {
		drawSpriteTint(SPR_BACKGROUND_FIELD_ORLEANS, x, 0, bandW, bandH,
		               tile % 2, 1.0, levelTintR, levelTintG, levelTintB);
		tile++;
	}

	// haze so sprites read against the painting
	fillRectAlpha(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
	              COL_BLUE_DARK, currentLevel == 3 ? 0.30 : 0.10);
}

void drawGround()
{
	int i;
	double par = cameraX * 1.0;

	// The painted battlefield already has dirt down here, so the walk band
	// is only darkened rather than painted over.
	for (i = 0; i < 12; i++) {
		double t = i / 11.0;
		fillRectAlpha(0, GROUND_Y - i * (GROUND_Y / 12.0) - 6, SCREEN_WIDTH,
		              GROUND_Y / 12.0 + 1, COL_PANEL, 0.06 + 0.34 * t);
	}

	// the line Joan actually stands on
	fillRectAlpha(0, GROUND_Y - 3, SCREEN_WIDTH, 3, COL_GOLD_DIM, 0.35);

	// scrolling ruts and stones, tied to world position so they move right
	for (i = -2; i < 26; i++) {
		double wx = i * 90.0 - fmod(par, 90.0);
		fillRectAlpha(wx, GROUND_Y - 26, 46, 5, COL_PANEL, 0.35);
		fillRectAlpha(wx + 62, GROUND_Y - 46, 9, 9, COL_PANEL, 0.3);
	}
}

void drawObstacles()
{
	int i, k;
	for (i = 0; i < MAX_OBSTACLES; i++) {
		if (!obstacles[i].active) continue;
		if (!onScreen(obstacles[i].x, 200)) continue;

		double sx = worldToScreenX(obstacles[i].x);
		double y  = obstacles[i].y;
		double w  = obstacles[i].w;
		double h  = obstacles[i].h;

		// planks
		for (k = 0; k < 4; k++) {
			setColorMix(COL_HILL_NEAR, COL_GOLD_DIM, 0.18 + 0.12 * (k % 2));
			iFilledRectangle(sx, y + k * (h / 4), w, h / 4 - 3);
		}
		// cross braces
		setColor(COL_GREY_DARK);
		iLine(sx, y, sx + w, y + h);
		iLine(sx, y + h, sx + w, y);

		if (obstacles[i].hitFlash > 0)
			fillRectAlpha(sx, y, w, h, COL_WHITE, 0.05 * obstacles[i].hitFlash);

		// damage state
		if (obstacles[i].hp < obstacles[i].maxHp) {
			double f = (double)obstacles[i].hp / obstacles[i].maxHp;
			setColor(COL_RED);
			iFilledRectangle(sx, y + h + 6, w * f, 4);
		}
	}
}

void drawPickups()
{
	int i;
	for (i = 0; i < MAX_PICKUPS; i++) {
		if (!pickups[i].active) continue;
		if (!onScreen(pickups[i].x, 120)) continue;

		double sx  = worldToScreenX(pickups[i].x);
		double bob = sin(uiTime * 0.08 + pickups[i].phase) * 6;
		double sy  = pickups[i].y + bob;

		if (pickups[i].type == PU_COIN) {
			double squash = 0.55 + 0.45 * fabs(sin(uiTime * 0.09 + pickups[i].phase));
			setColor(COL_GOLD_DIM);
			iFilledEllipse(sx, sy, 12 * squash, 12, 16);
			setColor(COL_GOLD);
			iFilledEllipse(sx, sy, 9 * squash, 9, 16);
		} else {
			setColor(COL_RED);
			iFilledRectangle(sx - 8, sy - 10, 16, 18);
			setColor(COL_WHITE);
			iFilledRectangle(sx - 3, sy - 4, 6, 6);
			setColor(COL_GREY_DARK);
			iFilledRectangle(sx - 4, sy + 8, 8, 5);
		}
	}
}

// the banner at the end of the level
void drawGate()
{
	double sx = worldToScreenX(gateX);
	int i;
	if (sx < -200 || sx > SCREEN_WIDTH + 200) return;

	// On level 02 the banner ends up inside the Hydra's arena, where it
	// draws straight over the player. It has already done its job by then -
	// the road is shut - so it stops being drawn once the fight starts.
	if (currentLevel == 2 && levelPhase == PHASE_BOSS) return;

	setColor(COL_GREY_DARK);
	iFilledRectangle(sx, GROUND_Y, 8, 250);

	for (i = 0; i < 9; i++) {
		double t = i / 9.0;
		double sway = sin(uiTime * 0.06 + t * 2.4) * (4 + 10 * t);
		setColorMix(COL_BLUE, COL_BLUE_DARK, t);
		iFilledRectangle(sx + 8 + sway, GROUND_Y + 250 - (i + 1) * 18, 92 - t * 18, 18);
	}
	drawDiamond(sx + 54, GROUND_Y + 170, 14, COL_GOLD);
	drawDiamond(sx + 54, GROUND_Y + 170, 7, COL_BLUE_DARK);
}

// ---------------- scenery ----------------
//
//  Trees, rocks, roadside standards and smoke columns. All of it is drawn
//  from primitives against a fixed table, so the road looks the same on
//  every run and none of it has to be exported as art. Trees and rocks
//  scroll at 0.7 of the camera speed, which puts them between the painted
//  background (0.35) and the ground (1.0) and gives the road real depth.

enum SceneryType { SC_TREE = 0, SC_ROCK, SC_FLAG, SC_SMOKE };

const int MAX_SCENERY = 96;

struct Scenery {
	int    active;
	int    type;
	double x;        // world position
	double scale;
	double par;      // parallax factor
};

Scenery scenery[MAX_SCENERY];

void clearScenery()
{
	int i;
	for (i = 0; i < MAX_SCENERY; i++) scenery[i].active = 0;
}

void addScenery(int type, double x, double scale, double par)
{
	int i;
	for (i = 0; i < MAX_SCENERY; i++) {
		if (scenery[i].active) continue;
		scenery[i].active = 1;
		scenery[i].type   = type;
		scenery[i].x      = x;
		scenery[i].scale  = scale;
		scenery[i].par    = par;
		return;
	}
}

// A roadside poplar, drawn as a translucent silhouette rather than a solid
// shape. The background is a painting, so an opaque geometric tree next to
// it reads as a cut-out; a dark warm shape at 70% reads as a tree standing
// in the same light. Narrow, because the French roads in the source art are
// lined with poplars, not firs.
void drawTree(double sx, double h)
{
	double tw = h * 0.055;
	int k;

	beginBlend();

	// trunk
	glColor4f(0.13f, 0.11f, 0.09f, 0.80f);
	iFilledRectangle(sx - tw / 2, GROUND_Y - 4, tw, h * 0.46);

	// four canopy tiers, narrow and overlapping, lifting toward the top
	for (k = 0; k < 4; k++) {
		double t  = k / 3.0;
		double cy = GROUND_Y + h * (0.30 + 0.17 * k);
		double cw = h * (0.30 - 0.055 * k);
		double px[3], py[3];
		px[0] = sx;          py[0] = cy + h * 0.30;
		px[1] = sx + cw / 2; py[1] = cy;
		px[2] = sx - cw / 2; py[2] = cy;
		glColor4f((float)(0.15 - 0.03 * t), (float)(0.19 - 0.04 * t),
		          (float)(0.12 - 0.02 * t), (float)(0.74 - 0.06 * t));
		iFilledPolygon(px, py, 3);
	}

	endBlend();
}

void drawRock(double sx, double h)
{
	double px[5], py[5];
	px[0] = sx - h * 0.62; py[0] = GROUND_Y - 2;
	px[1] = sx - h * 0.34; py[1] = GROUND_Y + h * 0.78;
	px[2] = sx + h * 0.12; py[2] = GROUND_Y + h;
	px[3] = sx + h * 0.58; py[3] = GROUND_Y + h * 0.40;
	px[4] = sx + h * 0.66; py[4] = GROUND_Y - 2;

	beginBlend();
	glColor4f(0.22f, 0.21f, 0.19f, 0.82f);
	iFilledPolygon(px, py, 5);
	glColor4f(0.42f, 0.41f, 0.38f, 0.55f);   // a lit edge along the top
	iLine(px[1], py[1], px[2], py[2]);
	iLine(px[2], py[2], px[3], py[3]);
	endBlend();
}

// Roadside standard. French blue on the approach, English red once past the
// gate, so the player can see whose ground they are standing on.
void drawFlagPole(double sx, double h, int english)
{
	int i;
	Color cloth = english ? COL_RED : COL_BLUE;
	double segH = h * 0.062;
	double px[4], py[4];

	// pole - pale enough to read against the dark ground band
	setColorMix(COL_GREY_DARK, COL_WHITE, 0.30);
	iFilledRectangle(sx - 1.5, GROUND_Y - 2, 3, h);
	drawDiamond(sx, GROUND_Y + h + 6, 5, COL_GOLD);

	// The cloth is built from quads rather than rectangles so it can taper
	// and so each row can lag the one above it - that lag is what makes it
	// look like fabric catching wind instead of a flat blue box.
	for (i = 0; i < 8; i++) {
		double t0 = i / 8.0, t1 = (i + 1) / 8.0;
		double y0 = GROUND_Y + h - i * segH;
		double y1 = y0 - segH;
		double s0 = sin(uiTime * 0.07 + sx * 0.013 + t0 * 2.9) * (1.5 + 11.0 * t0);
		double s1 = sin(uiTime * 0.07 + sx * 0.013 + t1 * 2.9) * (1.5 + 11.0 * t1);
		double w0 = h * (0.26 - 0.15 * t0);
		double w1 = h * (0.26 - 0.15 * t1);

		px[0] = sx + s0;      py[0] = y0;
		px[1] = sx + w0 + s0; py[1] = y0;
		px[2] = sx + w1 + s1; py[2] = y1;
		px[3] = sx + s1;      py[3] = y1;

		setColorMix(cloth, COL_PANEL, 0.10 + t0 * 0.45);
		iFilledPolygon(px, py, 4);
	}

	// gold fringe down the flying edge
	setColor(COL_GOLD_DIM);
	for (i = 0; i < 8; i++) {
		double t0 = i / 8.0, t1 = (i + 1) / 8.0;
		double s0 = sin(uiTime * 0.07 + sx * 0.013 + t0 * 2.9) * (1.5 + 11.0 * t0);
		double s1 = sin(uiTime * 0.07 + sx * 0.013 + t1 * 2.9) * (1.5 + 11.0 * t1);
		iLine(sx + h * (0.26 - 0.15 * t0) + s0, GROUND_Y + h - i * segH,
		      sx + h * (0.26 - 0.15 * t1) + s1, GROUND_Y + h - (i + 1) * segH);
	}
}

// War atmosphere: a column of smoke that drifts and thins as it rises.
void drawSmoke(double sx, double h)
{
	int k;
	beginBlend();
	for (k = 0; k < 9; k++) {
		double t  = k / 8.0;
		double y  = GROUND_Y + 10 + t * h;
		double r  = h * (0.07 + 0.16 * t);
		double dx = sin(uiTime * 0.012 + k * 0.7 + sx * 0.01) * (6 + 26 * t);
		glColor4f(0.42f, 0.40f, 0.38f, (float)(0.30 * (1.0 - t)));
		iFilledCircle(sx + dx, y, r, 16);
	}
	// embers at the base
	for (k = 0; k < 3; k++) {
		double f = 0.6 + 0.4 * sin(uiTime * 0.3 + k);
		glColor4f(0.92f, 0.55f, 0.18f, (float)(0.35 * f));
		iFilledCircle(sx + (k - 1) * 9, GROUND_Y + 6, 7 * f, 14);
	}
	endBlend();
}

void drawScenery()
{
	int i;
	double sx;

	// smoke first: it belongs behind the trees
	for (i = 0; i < MAX_SCENERY; i++) {
		if (!scenery[i].active || scenery[i].type != SC_SMOKE) continue;
		sx = scenery[i].x - cameraX * scenery[i].par;
		if (sx < -300 || sx > SCREEN_WIDTH + 300) continue;
		drawSmoke(sx, scenery[i].scale);
	}

	for (i = 0; i < MAX_SCENERY; i++) {
		if (!scenery[i].active || scenery[i].type == SC_SMOKE) continue;
		sx = scenery[i].x - cameraX * scenery[i].par;
		if (sx < -260 || sx > SCREEN_WIDTH + 260) continue;

		if (scenery[i].type == SC_TREE)      drawTree(sx, scenery[i].scale);
		else if (scenery[i].type == SC_ROCK) drawRock(sx, scenery[i].scale);
		else                                 drawFlagPole(sx, scenery[i].scale,
		                                                  scenery[i].x > gateX - 300);
	}
}

// ---------------- level building ----------------

// -----------------------------------------------------------------
//  buildLevel01 - the road to Brittany.
//
//  Ten named sections along 6400 units. Coins run in lines of three to
//  eight and always lead forward, because a line of coins is the clearest
//  way to say "this way" without drawing an arrow on the screen. They are
//  never placed inside an enemy group: the player should never have to
//  choose between fighting well and collecting.
//
//  The enemies are placed separately, by populateLevel01() in Game.hpp.
// -----------------------------------------------------------------
void buildLevel01()
{
	int i;

	clearObstacles();
	clearPickups();
	clearScenery();

	levelKills  = 0;
	bossAlive   = 0;
	levelLength = LEVEL01_LENGTH;
	gateX       = LEVEL01_GATE;
	setLevelMood(1);

	// ---- coins, section by section ----
	{
		static const double coinX[] = {
			/* Muster         */  140,  260,  380,  500,
			/* First Blood    */  740,  880, 1020,
			/* The Road       */ 1300, 1440, 1580, 1720, 1840, 1960,
			/* The Well       */ 2080, 2210,
			/* The Press      */ 2420, 2600, 2820, 3040,
			/* Coin Run       */ 3250, 3300, 3350, 3400, 3450, 3500, 3550, 3595,
			/* Shield Wall    */ 3900, 4300,
			/* Final Approach */ 5000, 5220, 5440
		};
		int n = (int)(sizeof(coinX) / sizeof(coinX[0]));

		for (i = 0; i < n; i++) {
			// the coin run arcs up over the road; everything else stays low
			double y = (coinX[i] >= 3250 && coinX[i] <= 3595)
			         ? GROUND_Y + 46 + 52 * sin((i - 19) * 0.45)
			         : GROUND_Y + 40 + (i % 3) * 30;
			addPickup(PU_COIN, coinX[i], y);
		}
	}

	// ---- health ----
	// With no healing from kills these are the entire supply, so there is one
	// waiting after every stretch that costs Joan something, and two before
	// the arena.
	addPickup(PU_HPBOX, 1120, GROUND_Y + 44);   // after the first soldier
	addPickup(PU_HPBOX, 2150, GROUND_Y + 44);   // The Well,   before the first group
	addPickup(PU_HPBOX, 3160, GROUND_Y + 44);   // after The Press
	addPickup(PU_HPBOX, 4180, GROUND_Y + 44);   // inside the shield wall
	addPickup(PU_HPBOX, 4750, GROUND_Y + 44);   // The Chapel, after the shield wall
	addPickup(PU_HPBOX, 5700, GROUND_Y + 44);   // last one before the banner

	// ---- barricades: something to break or jump ----
	addObstacle(OB_BARRICADE, 1150, 46, 74, 42);
	addObstacle(OB_BARRICADE, 2340, 46, 74, 46);
	addObstacle(OB_BARRICADE, 3680, 46, 74, 50);
	addObstacle(OB_BARRICADE, 4980, 46, 74, 50);

	// ---- trees and rocks, stopping short of the killing field ----
	for (i = 0; i < 20; i++) {
		double x = 240 + i * 305.0;
		if (x > gateX - 260) break;
		addScenery(SC_TREE, x, 168 + (i % 5) * 34, 0.70);
		if (i % 4 != 2) addScenery(SC_TREE, x + 96, 132 + (i % 3) * 28, 0.74);
		if (i % 3 == 1) addScenery(SC_ROCK, x + 190, 24 + (i % 3) * 10, 0.90);
	}

	// ---- roadside standards every 800 units ----
	for (i = 1; i * 800 < levelLength; i++)
		addScenery(SC_FLAG, i * 800.0, 148, 1.0);

	// ---- burning ground: three columns, the last one over the arena ----
	addScenery(SC_SMOKE, 2700, 210, 0.80);
	addScenery(SC_SMOKE, 4450, 250, 0.80);
	addScenery(SC_SMOKE, 6050, 300, 0.80);
}

void buildLevel(int level)
{
	int i;
	int count;

	if (level == 1) { buildLevel01(); return; }

	clearObstacles();
	clearPickups();
	clearScenery();
	levelKills = 0;
	bossAlive  = 0;
	setLevelMood(level);

	if (level == 1)      levelLength = 5400;
	else if (level == 2) levelLength = 6600;
	else                 levelLength = 4600;

	gateX = levelLength - 260;

	// barricades: something to break or jump
	count = (level == 3) ? 3 : 4 + level;
	for (i = 0; i < count; i++) {
		double x = 900 + i * (levelLength - 1600) / count + randRange(-90, 90);
		addObstacle(OB_BARRICADE, x, 46, 74, 34 + level * 8);
	}

	// coins in little clusters, plus the odd flask
	count = 14 + level * 4;
	for (i = 0; i < count && i < MAX_PICKUPS - 6; i++) {
		double x = 520 + i * (levelLength - 900) / count + randRange(-60, 60);
		double y = GROUND_Y + 34 + (i % 3) * 46;
		addPickup(PU_COIN, x, y);
	}
	for (i = 0; i < 3; i++) {
		double x = 1400 + i * (levelLength - 2000) / 3;
		addPickup(PU_HPBOX, x, GROUND_Y + 40);
	}
}

#endif
