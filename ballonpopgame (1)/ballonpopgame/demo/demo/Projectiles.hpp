#ifndef PROJECTILES_HPP
#define PROJECTILES_HPP

// -----------------------------------------------------------------
//  Projectiles.hpp - arrows and the boss shockwave.
//  A projectile carries the faction that fired it, so one array and
//  one update loop serve both sides.
// -----------------------------------------------------------------

#include <math.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "World.hpp"
#include "Effects.hpp"

enum Faction  { SIDE_PLAYER = 0, SIDE_ENEMY };
enum ProjType { PROJ_ARROW = 0, PROJ_SHOCK };

struct Projectile {
	int active;
	int type;
	int side;
	double x, y, vx, vy;
	double w, h;
	int damage;
	int life;
	int gravity;
};

Projectile projectiles[MAX_PROJ];

void clearProjectiles()
{
	int i;
	for (i = 0; i < MAX_PROJ; i++) projectiles[i].active = 0;
}

// registration for the arrow sprite, from tools/measure_art.py
const CharArt ART_ARROW = { 1.0, 1.0, 0.5 };

void spawnArrow(double x, double y, double dirX, int side, int damage)
{
	int i;
	for (i = 0; i < MAX_PROJ; i++) {
		if (projectiles[i].active) continue;
		projectiles[i].active  = 1;
		projectiles[i].type    = PROJ_ARROW;
		projectiles[i].side    = side;
		projectiles[i].x       = x;
		projectiles[i].y       = y;
		projectiles[i].vx      = dirX * (side == SIDE_PLAYER ? ARROW_SPEED_L2 : 8.0);
		projectiles[i].vy      = 0;
		projectiles[i].w       = 26;
		projectiles[i].h       = 5;
		projectiles[i].damage  = damage;
		projectiles[i].life    = 150;
		projectiles[i].gravity = 0;
		return;
	}
}

void spawnShock(double x, double y, double dirX, int damage)
{
	int i;
	for (i = 0; i < MAX_PROJ; i++) {
		if (projectiles[i].active) continue;
		projectiles[i].active  = 1;
		projectiles[i].type    = PROJ_SHOCK;
		projectiles[i].side    = SIDE_ENEMY;
		projectiles[i].x       = x;
		projectiles[i].y       = y;
		projectiles[i].vx      = dirX * 6.0;
		projectiles[i].vy      = 0;
		projectiles[i].w       = 40;
		projectiles[i].h       = 34;
		projectiles[i].damage  = damage;
		projectiles[i].life    = 130;
		projectiles[i].gravity = 0;
		return;
	}
}

Rect projRect(int i)
{
	return makeRect(projectiles[i].x - projectiles[i].w / 2,
	                projectiles[i].y - projectiles[i].h / 2,
	                projectiles[i].w, projectiles[i].h);
}

void updateProjectiles()
{
	int i, k;
	for (i = 0; i < MAX_PROJ; i++) {
		if (!projectiles[i].active) continue;

		projectiles[i].x += projectiles[i].vx;
		projectiles[i].y += projectiles[i].vy;
		if (projectiles[i].gravity) projectiles[i].vy -= 0.25;

		// stopped by a barricade
		for (k = 0; k < MAX_OBSTACLES; k++) {
			if (!obstacles[k].active) continue;
			if (rectsOverlap(projRect(i), obstacleRect(k))) {
				fxSpark(projectiles[i].x, projectiles[i].y, COL_GOLD_DIM, 4);
				projectiles[i].active = 0;
				break;
			}
		}
		if (!projectiles[i].active) continue;

		if (projectiles[i].y < GROUND_Y - 4) {
			fxDust(projectiles[i].x, GROUND_Y, 3);
			projectiles[i].active = 0;
			continue;
		}
		if (--projectiles[i].life <= 0) projectiles[i].active = 0;
		if (!onScreen(projectiles[i].x, 420)) projectiles[i].active = 0;
	}
}

void drawProjectiles()
{
	int i;
	for (i = 0; i < MAX_PROJ; i++) {
		if (!projectiles[i].active) continue;
		double sx = worldToScreenX(projectiles[i].x);
		double sy = projectiles[i].y;
		double dir = (projectiles[i].vx >= 0) ? 1.0 : -1.0;

		if (projectiles[i].type == PROJ_ARROW) {
			// The real arrow artwork points right, so an arrow travelling
			// left is mirrored. A faint streak behind it sells the speed.
			double aw = 46, ah = aw * 12.0 / 57.0;

			beginBlend();
			glColor4f(1.0f, 0.92f, 0.62f, 0.22f);
			iFilledRectangle(sx - dir * 34, sy - 1.5, 34, 3);
			endBlend();

			drawSpriteEx(SPR_ITEMS_ARROW_BOLT, sx - aw / 2, sy - ah / 2,
			             aw, ah, (dir < 0) ? 1 : 0, 1.0);
		} else {
			double t = projectiles[i].life / 130.0;
			fillRectAlpha(sx - 20, GROUND_Y, 40, 34, COL_RED, 0.35 * t + 0.25);
			setColor(COL_FLAME);
			iLine(sx - 18, GROUND_Y + 2, sx, GROUND_Y + 30);
			iLine(sx + 18, GROUND_Y + 2, sx, GROUND_Y + 30);
		}
	}
}

#endif
