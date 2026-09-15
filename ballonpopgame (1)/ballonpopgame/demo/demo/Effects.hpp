#ifndef EFFECTS_HPP
#define EFFECTS_HPP

// -----------------------------------------------------------------
//  Effects.hpp - short lived visual feedback.
//  Sparks when steel lands, floating damage numbers, dust, rings.
//  Nothing here affects gameplay; it only makes hits readable.
// -----------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "World.hpp"

enum EffectType { FX_SPARK = 0, FX_NUMBER, FX_RING, FX_DUST, FX_SHOCK };

struct Effect {
	int active;
	int type;
	double x, y, vx, vy;
	int life, maxLife;
	Color colour;
	char text[12];
	double size;
};

Effect effects[MAX_EFFECTS];
double screenShake = 0;

// The offset the whole battlefield is drawn at while the screen is shaking.
// It is rolled here, on the 20 ms tick, and NOT inside the draw path: iDraw()
// runs uncapped, so rolling it per frame made the shake scale with frame rate
// and turned a thump into a blur.
double shakeOffX = 0, shakeOffY = 0;

void clearEffects()
{
	int i;
	for (i = 0; i < MAX_EFFECTS; i++) effects[i].active = 0;
	screenShake = 0;
	shakeOffX = shakeOffY = 0;
}

Effect* newEffect()
{
	int i;
	for (i = 0; i < MAX_EFFECTS; i++) {
		if (!effects[i].active) return &effects[i];
	}
	return 0;
}

void fxSpark(double x, double y, Color c, int count)
{
	int i;
	for (i = 0; i < count; i++) {
		Effect* e = newEffect();
		if (!e) return;
		e->active  = 1;
		e->type    = FX_SPARK;
		e->x = x; e->y = y;
		e->vx = randRange(-40, 40) / 10.0;
		e->vy = randRange(-10, 60) / 10.0;
		e->life = e->maxLife = randRange(14, 26);
		e->colour = c;
		e->size = randRange(2, 5);
	}
}

void fxNumber(double x, double y, int value, Color c)
{
	Effect* e = newEffect();
	if (!e) return;
	e->active = 1;
	e->type   = FX_NUMBER;
	e->x = x; e->y = y;
	e->vx = 0; e->vy = 1.5;
	e->life = e->maxLife = 46;
	e->colour = c;
	sprintf(e->text, "%d", value);
}

void fxText(double x, double y, const char* s, Color c)
{
	Effect* e = newEffect();
	if (!e) return;
	e->active = 1;
	e->type   = FX_NUMBER;
	e->x = x; e->y = y;
	e->vx = 0; e->vy = 1.0;
	e->life = e->maxLife = 80;
	e->colour = c;
	strncpy(e->text, s, 11);
	e->text[11] = 0;
}

void fxRing(double x, double y, Color c, double size, int life)
{
	Effect* e = newEffect();
	if (!e) return;
	e->active = 1;
	e->type   = FX_RING;
	e->x = x; e->y = y;
	e->vx = e->vy = 0;
	e->life = e->maxLife = life;
	e->colour = c;
	e->size = size;
}

void fxDust(double x, double y, int count)
{
	int i;
	for (i = 0; i < count; i++) {
		Effect* e = newEffect();
		if (!e) return;
		e->active = 1;
		e->type   = FX_DUST;
		e->x = x + randRange(-14, 14);
		e->y = y;
		e->vx = randRange(-15, 15) / 10.0;
		e->vy = randRange(2, 18) / 10.0;
		e->life = e->maxLife = randRange(16, 30);
		e->colour = COL_GREY_DARK;
		e->size = randRange(3, 7);
	}
}

void shakeScreen(double amount)
{
	if (amount > screenShake) screenShake = amount;
}

void updateEffects()
{
	int i;
	for (i = 0; i < MAX_EFFECTS; i++) {
		if (!effects[i].active) continue;
		effects[i].x += effects[i].vx;
		effects[i].y += effects[i].vy;
		if (effects[i].type == FX_SPARK) effects[i].vy -= 0.28;
		if (effects[i].type == FX_DUST)  effects[i].vy -= 0.06;
		if (--effects[i].life <= 0) effects[i].active = 0;
	}
	if (screenShake > 0) {
		shakeOffX = randRange(-(int)screenShake, (int)screenShake);
		shakeOffY = randRange(-(int)screenShake, (int)screenShake);
		screenShake *= 0.86;
		if (screenShake < 0.3) screenShake = 0;
	} else {
		shakeOffX = shakeOffY = 0;
	}
}

void drawEffects()
{
	int i;
	for (i = 0; i < MAX_EFFECTS; i++) {
		if (!effects[i].active) continue;

		double t  = (double)effects[i].life / effects[i].maxLife;
		double sx = worldToScreenX(effects[i].x);
		double sy = effects[i].y;

		switch (effects[i].type) {
		case FX_SPARK:
			fillRectAlpha(sx, sy, effects[i].size, effects[i].size, effects[i].colour, t);
			break;

		case FX_DUST:
			fillRectAlpha(sx, sy, effects[i].size, effects[i].size,
			              effects[i].colour, t * 0.5);
			break;

		case FX_NUMBER:
			setColor(effects[i].colour);
			drawTextBold(sx, sy, effects[i].text, FONT_SMALL);
			break;

		case FX_RING: {
			double r = effects[i].size * (1.0 - t) + 6;
			beginBlend();
			glColor4f((float)(effects[i].colour.r / 255.0),
			          (float)(effects[i].colour.g / 255.0),
			          (float)(effects[i].colour.b / 255.0), (float)(t * 0.8));
			iCircle(sx, sy, r, 26);
			iCircle(sx, sy, r * 0.82, 26);
			endBlend();
			break;
		}

		case FX_SHOCK:
			fillRectAlpha(sx - effects[i].size, sy, effects[i].size * 2, 6,
			              effects[i].colour, t);
			break;
		}
	}
}

#endif
