#ifndef UI_HPP
#define UI_HPP

// -----------------------------------------------------------------
//  UI.hpp - drawing helpers shared by every screen.
//  Pure primitives for now (no images are loaded in this phase).
// -----------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "GameState.hpp"
#include "Assets.hpp"

// ---------------- colour helpers ----------------

void setColor(Color c)
{
	iSetColor(c.r, c.g, c.b);
}

void setColorMix(Color a, Color b, double t)
{
	iSetColor(lerpd(a.r, b.r, t), lerpd(a.g, b.g, t), lerpd(a.b, b.b, t));
}

// Translucent drawing needs real alpha blending, which the framework
// does not switch on by default. Enable it only around our own call.
void beginBlend()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void endBlend()
{
	glDisable(GL_BLEND);
}

void fillRectAlpha(double x, double y, double w, double h, Color c, double a)
{
	beginBlend();
	glColor4f((float)(c.r / 255.0), (float)(c.g / 255.0), (float)(c.b / 255.0), (float)a);
	iFilledRectangle(x, y, w, h);
	endBlend();
}

// ---------------- text helpers ----------------
// iText() takes a non-const char*, so every literal gets cast in one place.

int textWidth(const char* s, void* font)
{
	int w = 0, i;
	for (i = 0; s[i]; i++) w += glutBitmapWidth(font, s[i]);
	return w;
}

void drawText(double x, double y, const char* s, void* font)
{
	iText(x, y, (char*)s, font);
}

void drawTextCentered(double cx, double y, const char* s, void* font)
{
	drawText(cx - textWidth(s, font) / 2.0, y, s, font);
}

// GLUT has no bold font, so overdraw with a 1px offset to fake weight
void drawTextBold(double x, double y, const char* s, void* font)
{
	drawText(x,     y,     s, font);
	drawText(x + 1, y,     s, font);
	drawText(x,     y + 1, s, font);
	drawText(x + 1, y + 1, s, font);
}

void drawTextCenteredBold(double cx, double y, const char* s, void* font)
{
	drawTextBold(cx - textWidth(s, font) / 2.0, y, s, font);
}

// title text with a dark drop shadow behind it
void drawTitleText(double cx, double y, const char* s, Color face, void* font)
{
	setColor(COL_PANEL);
	drawTextCenteredBold(cx + 3, y - 3, s, font);
	setColor(face);
	drawTextCenteredBold(cx, y, s, font);
}

// ---------------- shapes ----------------

void drawVGradient(double x, double y, double w, double h, Color bottom, Color top)
{
	int steps = 44, i;
	double bandH = h / steps;
	for (i = 0; i < steps; i++) {
		setColorMix(bottom, top, (double)i / (steps - 1));
		iFilledRectangle(x, y + i * bandH, w, bandH + 1);
	}
}

void drawFrame(Rect r, Color c, int thickness)
{
	int i;
	setColor(c);
	for (i = 0; i < thickness; i++)
		iRectangle(r.x - i, r.y - i, r.w + 2 * i, r.h + 2 * i);
}

// small gold diamond, used as a decorative marker
void drawDiamond(double cx, double cy, double s, Color c)
{
	double px[4], py[4];
	px[0] = cx;     py[0] = cy + s;
	px[1] = cx + s; py[1] = cy;
	px[2] = cx;     py[2] = cy - s;
	px[3] = cx - s; py[3] = cy;
	setColor(c);
	iFilledPolygon(px, py, 4);
}

// small padlock, drawn from primitives because the GLUT bitmap fonts
// have no lock character
void drawPadlock(double cx, double cy, double s, Color c)
{
	setColor(c);

	// shackle: a ring whose lower half is hidden by the body
	iCircle(cx, cy + s * 0.55, s * 0.42, 18);
	iCircle(cx, cy + s * 0.55, s * 0.42 - 1.5, 18);

	// body
	iFilledRectangle(cx - s * 0.62, cy - s * 0.62, s * 1.24, s * 1.10);

	// keyhole
	setColor(COL_PANEL);
	iFilledRectangle(cx - s * 0.11, cy - s * 0.34, s * 0.22, s * 0.55);
}

// dark panel with a gold frame and corner ticks
void drawPanel(Rect r, double alpha)
{
	double c = 14;
	fillRectAlpha(r.x, r.y, r.w, r.h, COL_PANEL, alpha);
	drawFrame(r, COL_GOLD_DIM, 1);

	setColor(COL_GOLD);
	iLine(r.x, r.y + c, r.x, r.y);
	iLine(r.x, r.y, r.x + c, r.y);
	iLine(r.x + r.w - c, r.y, r.x + r.w, r.y);
	iLine(r.x + r.w, r.y, r.x + r.w, r.y + c);
	iLine(r.x, r.y + r.h - c, r.x, r.y + r.h);
	iLine(r.x, r.y + r.h, r.x + c, r.y + r.h);
	iLine(r.x + r.w - c, r.y + r.h, r.x + r.w, r.y + r.h);
	iLine(r.x + r.w, r.y + r.h, r.x + r.w, r.y + r.h - c);
}

// horizontal gold rule with a diamond in the middle
void drawDivider(double cx, double y, double halfWidth)
{
	setColor(COL_GOLD_DIM);
	iLine(cx - halfWidth, y, cx - 14, y);
	iLine(cx + 14, y, cx + halfWidth, y);
	drawDiamond(cx, y, 5, COL_GOLD);
}

// ---------------- menu button ----------------

void drawButton(Rect r, const char* label, int selected)
{
	double pulse = 0.5 + 0.5 * sin(uiTime * 0.09);

	if (selected) {
		fillRectAlpha(r.x - 4, r.y - 4, r.w + 8, r.h + 8, COL_GOLD, 0.16 + 0.10 * pulse);
		fillRectAlpha(r.x, r.y, r.w, r.h, COL_BLUE, 0.92);
		drawFrame(r, COL_GOLD, 2);
	} else {
		fillRectAlpha(r.x, r.y, r.w, r.h, COL_BLUE_DARK, 0.72);
		drawFrame(r, COL_GREY_DARK, 1);
	}

	setColor(selected ? COL_GOLD : COL_GREY);
	drawTextCenteredBold(r.x + r.w / 2, r.y + r.h / 2 - 7, label, FONT_MED);

	if (selected) {
		drawDiamond(r.x - 18, r.y + r.h / 2, 6, COL_GOLD);
		drawDiamond(r.x + r.w + 18, r.y + r.h / 2, 6, COL_GOLD);
	}
}

// ---------------- decorative banner ----------------
// tapered cloth that sways with uiTime; phase separates the two copies

void drawHangingBanner(double x, double topY, double w, double h, double phase)
{
	int rows = 10, i;
	double px[4], py[4];
	double segH = h / rows;

	setColor(COL_GREY_DARK);
	iFilledRectangle(x - 3, topY - 6, 6, 18);

	for (i = 0; i < rows; i++) {
		double t0 = (double)i / rows;
		double t1 = (double)(i + 1) / rows;
		double sway0 = sin(uiTime * 0.05 + phase + t0 * 2.2) * (5 + 12 * t0);
		double sway1 = sin(uiTime * 0.05 + phase + t1 * 2.2) * (5 + 12 * t1);
		double half0 = (w / 2) * (1.0 - 0.25 * t0);
		double half1 = (w / 2) * (1.0 - 0.25 * t1);

		px[0] = x - half0 + sway0; py[0] = topY - i * segH;
		px[1] = x + half0 + sway0; py[1] = topY - i * segH;
		px[2] = x + half1 + sway1; py[2] = topY - (i + 1) * segH;
		px[3] = x - half1 + sway1; py[3] = topY - (i + 1) * segH;

		setColorMix(COL_BLUE, COL_BLUE_DARK, t0);
		iFilledPolygon(px, py, 4);

		setColor(COL_GOLD_DIM);
		iLine(px[0], py[0], px[3], py[3]);
		iLine(px[1], py[1], px[2], py[2]);
	}

	{
		double eSway = sin(uiTime * 0.05 + phase + 0.9) * 9;
		drawDiamond(x + eSway, topY - h * 0.42, 13, COL_GOLD);
		drawDiamond(x + eSway, topY - h * 0.42, 6, COL_BLUE_DARK);
	}
}

// ---------------- backdrop scenery ----------------
// night battlefield: sky, stars, hills, castle, campfires, stakes

void drawBackdrop()
{
	int i;
	drawVGradient(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COL_SKY_BOTTOM, COL_SKY_TOP);

	// stars: fixed positions, gentle twinkle
	for (i = 0; i < 70; i++) {
		double sx = (double)((i * 197 + 53) % SCREEN_WIDTH);
		double sy = 380 + (double)((i * 89) % 280);
		double tw = 0.45 + 0.55 * sin(uiTime * 0.04 + i);
		fillRectAlpha(sx, sy, 2, 2, COL_WHITE, 0.25 + 0.45 * tw);
	}

	// moon with a soft halo
	beginBlend();
	for (i = 5; i >= 1; i--) {
		glColor4f(0.93f, 0.94f, 1.0f, (float)(0.05 - i * 0.006));
		iFilledCircle(760, 566, 26 + i * 9.0, 34);
	}
	endBlend();
	setColor(COL_WHITE);
	iFilledCircle(760, 566, 24, 40);

	// far ridge
	{
		double hx[8], hy[8];
		hx[0] = 0;    hy[0] = 150;
		hx[1] = 150;  hy[1] = 250;
		hx[2] = 330;  hy[2] = 190;
		hx[3] = 520;  hy[3] = 285;
		hx[4] = 760;  hy[4] = 205;
		hx[5] = 980;  hy[5] = 270;
		hx[6] = 1200; hy[6] = 200;
		hx[7] = 1200; hy[7] = 0;
		setColor(COL_HILL_FAR);
		iFilledPolygon(hx, hy, 8);
		iFilledRectangle(0, 0, 6, 150);
	}

	// castle silhouette on the right ridge
	{
		double bx = 900, by = 250;
		setColor(COL_HILL_NEAR);
		iFilledRectangle(bx, by, 190, 70);
		for (i = 0; i < 7; i++) iFilledRectangle(bx + i * 28, by + 70, 16, 12);
		iFilledRectangle(bx - 26, by, 34, 118);
		iFilledRectangle(bx + 182, by, 34, 100);
		for (i = 0; i < 3; i++) {
			iFilledRectangle(bx - 26 + i * 12, by + 118, 8, 10);
			iFilledRectangle(bx + 182 + i * 12, by + 100, 8, 10);
		}
		setColor(COL_FLAME);
		iFilledRectangle(bx + 40, by + 26, 7, 12);
		iFilledRectangle(bx + 96, by + 26, 7, 12);
		iFilledRectangle(bx - 16, by + 70, 7, 12);
	}

	// near hill band
	{
		double nx[6], ny[6];
		nx[0] = 0;    ny[0] = 120;
		nx[1] = 260;  ny[1] = 165;
		nx[2] = 600;  ny[2] = 118;
		nx[3] = 940;  ny[3] = 172;
		nx[4] = 1200; ny[4] = 126;
		nx[5] = 1200; ny[5] = 0;
		setColor(COL_HILL_NEAR);
		iFilledPolygon(nx, ny, 6);
	}

	setColor(COL_GROUND);
	iFilledRectangle(0, 0, SCREEN_WIDTH, 96);

	// campfires with flicker
	for (i = 0; i < 2; i++) {
		double fx = (i == 0) ? 190.0 : 1010.0;
		double fy = 108.0;
		double flick = 0.75 + 0.25 * sin(uiTime * 0.35 + i * 2.0);
		int g;
		beginBlend();
		for (g = 4; g >= 1; g--) {
			glColor4f(0.92f, 0.61f, 0.20f, (float)(0.05 * flick));
			iFilledCircle(fx, fy + 4, 14.0 * g * flick, 22);
		}
		endBlend();
		setColor(COL_GREY_DARK);
		iFilledRectangle(fx - 16, fy - 8, 32, 6);
		setColor(COL_FLAME);
		iFilledCircle(fx, fy + 6 * flick, 9 * flick, 20);
		setColor(COL_GOLD);
		iFilledCircle(fx, fy + 3, 4 * flick, 16);
	}

	// crossed wooden stakes along the foreground
	setColor(COL_HILL_NEAR);
	for (i = 0; i < 9; i++) {
		double sx = 40 + i * 140.0;
		iLine(sx, 60, sx + 22, 104);
		iLine(sx + 22, 60, sx, 104);
	}
}

// A painted screen background, drawn edge to edge.
//
// Both background paintings are 16:9, exactly like the window, so they
// scale uniformly and are never distorted. If the file is missing the
// screen falls back to the hand drawn night backdrop rather than to the
// magenta missing-asset box, which full screen would be unreadable.
//
//   dim - 0.0 leaves the painting alone, higher values darken it so
//         text and buttons on top stay legible
void drawSceneBackground(int spriteId, double dim)
{
	if (!spriteReady(spriteId)) {
		drawBackdrop();
		return;
	}

	drawSpriteEx(spriteId, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1.0);

	if (dim > 0.0)
		fillRectAlpha(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COL_PANEL, dim);
}

// fade-to-black overlay, strongest right after a state change
void drawFadeOverlay()
{
	if (fadeTimer <= 0) return;
	fillRectAlpha(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
	              COL_PANEL, (double)fadeTimer / FADE_TICKS);
}

// hint line pinned to the bottom of every screen
void drawHintBar(const char* hint)
{
	fillRectAlpha(0, 0, SCREEN_WIDTH, 34, COL_PANEL, 0.75);
	setColor(COL_GREY);
	drawTextCentered(SCREEN_WIDTH / 2, 12, hint, FONT_SMALL);
}

#endif
