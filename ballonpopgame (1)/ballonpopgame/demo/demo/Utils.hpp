#ifndef UTILS_H
#define UTILS_H

// -----------------------------------------------------------------
//  Utils.hpp - small maths / geometry helpers used everywhere.
//  SCREEN_WIDTH and SCREEN_HEIGHT now live in Config.hpp.
// -----------------------------------------------------------------

#include <math.h>
#include <stdlib.h>
#include "Config.hpp"

// axis aligned box, world or screen space (iGraphics origin = bottom-left)
struct Rect {
	double x, y, w, h;
};

Rect makeRect(double x, double y, double w, double h)
{
	Rect r;
	r.x = x; r.y = y; r.w = w; r.h = h;
	return r;
}

double getEuclideanDistance(double x1, double x2, double y1, double y2)
{
	double temp = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
	return sqrt(temp);
}

// the only collision primitive the whole game needs
int rectsOverlap(Rect a, Rect b)
{
	if (a.x + a.w < b.x)  return 0;
	if (b.x + b.w < a.x)  return 0;
	if (a.y + a.h < b.y)  return 0;
	if (b.y + b.h < a.y)  return 0;
	return 1;
}

int pointInRect(double px, double py, Rect r)
{
	return (px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h);
}

int clampi(int v, int lo, int hi)
{
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

double clampd(double v, double lo, double hi)
{
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

// linear interpolation, t in [0,1]
double lerpd(double a, double b, double t)
{
	return a + (b - a) * t;
}

// random integer in [lo, hi]
int randRange(int lo, int hi)
{
	if (hi <= lo) return lo;
	return lo + rand() % (hi - lo + 1);
}

#endif
