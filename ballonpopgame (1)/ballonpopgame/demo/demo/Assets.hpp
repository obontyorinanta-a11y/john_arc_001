#ifndef ASSETS_HPP
#define ASSETS_HPP

// -----------------------------------------------------------------
//  Assets.hpp - texture loading and sprite drawing.
//
//  The table of assets is NOT written by hand. tools/prepare_assets.py
//  exports the artwork and regenerates AssetList.hpp; this file just
//  walks that table.
//
//  Why drawSprite() exists instead of the framework's iShowImage():
//    - iShowImage uses GL_REPLACE with no alpha blending, so soft PNG
//      edges come out as hard halos
//    - it always samples the whole texture, but our art sits in the
//      top-left corner of a power-of-two canvas
//    - it cannot mirror a sprite, and a side scroller needs to face left
//  iGraphics.h itself is never modified.
// -----------------------------------------------------------------

#include <stdio.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "AssetList.hpp"

// ASSET_SPRITE_COUNT can legitimately be 0 before any art is exported,
// and a zero sized array is not valid C++, hence the max(1) below.
#define SPRITE_ARRAY_SIZE (ASSET_SPRITE_COUNT > 0 ? ASSET_SPRITE_COUNT : 1)

struct Sprite {
	unsigned int tex;    // OpenGL texture name from iLoadImage
	int w, h;            // real artwork size in pixels
	double u1, v1;       // fraction of the canvas the artwork occupies
	int ok;              // 1 = file found and uploaded
};

Sprite gSprites[SPRITE_ARRAY_SIZE];
int gAssetsLoaded  = 0;
int gAssetsMissing = 0;

// ---------------- loading ----------------

int fileExists(const char* path)
{
	FILE* f = fopen(path, "rb");
	if (!f) return 0;
	fclose(f);
	return 1;
}

// MUST be called after iInitialize(), because a texture cannot be
// created before the OpenGL context exists.
void loadAllAssets()
{
	int i;
	gAssetsLoaded  = 0;
	gAssetsMissing = 0;

	printf("\n--- loading assets (%d listed) ---\n", ASSET_SPRITE_COUNT);

	for (i = 0; i < ASSET_SPRITE_COUNT; i++) {
		const AssetEntry* e = &ASSET_TABLE[i];

		gSprites[i].w  = e->w;
		gSprites[i].h  = e->h;
		gSprites[i].u1 = (e->texW > 0) ? (double)e->w / e->texW : 1.0;
		gSprites[i].v1 = (e->texH > 0) ? (double)e->h / e->texH : 1.0;
		gSprites[i].ok = 0;
		gSprites[i].tex = 0;

		// stb_image fails silently inside iLoadImage, so check first
		if (!fileExists(e->path)) {
			printf("  MISSING  %s\n", e->path);
			gAssetsMissing++;
			continue;
		}

		gSprites[i].tex = iLoadImage((char*)e->path);
		gSprites[i].ok  = 1;
		gAssetsLoaded++;
		printf("  ok       %-40s %4dx%-4d\n", e->path, e->w, e->h);
	}

	printf("--- %d loaded, %d missing ---\n\n", gAssetsLoaded, gAssetsMissing);

	if (gAssetsMissing > 0) {
		printf("Missing files are drawn as magenta boxes.\n");
		printf("Check that the working directory is the project folder and\n");
		printf("that tools/prepare_assets.py has been run.\n\n");
	}
}

// ---------------- drawing ----------------

// did this sprite's file load? used by callers that need a fallback
// instead of the magenta missing-asset box
int spriteReady(int id)
{
	return (id >= 0 && id < ASSET_SPRITE_COUNT && gSprites[id].ok);
}

// width / height of the artwork, 1.0 if the asset is missing
double spriteAspect(int id)
{
	if (id < 0 || id >= ASSET_SPRITE_COUNT || gSprites[id].h == 0) return 1.0;
	return (double)gSprites[id].w / gSprites[id].h;
}

// magenta placeholder so a missing asset is impossible to overlook
void drawMissingBox(double x, double y, double w, double h)
{
	iSetColor(230, 0, 190);
	iFilledRectangle(x, y, w, h);
	iSetColor(0, 0, 0);
	iRectangle(x, y, w, h);
	iLine(x, y, x + w, y + h);
	iLine(x, y + h, x + w, y);
}

//  id     - a SpriteId from AssetList.hpp
//  x, y   - bottom-left corner, in game coordinates
//  w, h   - size to draw at
//  flipX  - 1 mirrors the sprite horizontally
//  alpha  - 0.0 transparent ... 1.0 solid
//  tint r,g,b in 0..1 multiply the texture, so the same artwork can be
//  reused for several enemy ranks and for day / dusk / night backgrounds
void drawSpriteTint(int id, double x, double y, double w, double h,
                    int flipX, double alpha, double tr, double tg, double tb)
{
	double uL, uR;

	if (id < 0 || id >= ASSET_SPRITE_COUNT || !gSprites[id].ok) {
		drawMissingBox(x, y, w, h);
		return;
	}

	uL = flipX ? gSprites[id].u1 : 0.0;
	uR = flipX ? 0.0 : gSprites[id].u1;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, gSprites[id].tex);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// MODULATE lets the vertex colour tint the sprite and carry alpha
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f((float)tr, (float)tg, (float)tb, (float)alpha);

	// stb_image gives top-down rows, so v = 0 is the TOP of the artwork
	glBegin(GL_QUADS);
		glTexCoord2f((float)uL, (float)gSprites[id].v1); glVertex2f((float)x,       (float)y);
		glTexCoord2f((float)uR, (float)gSprites[id].v1); glVertex2f((float)(x + w), (float)y);
		glTexCoord2f((float)uR, 0.0f);                   glVertex2f((float)(x + w), (float)(y + h));
		glTexCoord2f((float)uL, 0.0f);                   glVertex2f((float)x,       (float)(y + h));
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void drawSpriteEx(int id, double x, double y, double w, double h,
                  int flipX, double alpha)
{
	drawSpriteTint(id, x, y, w, h, flipX, alpha, 1.0, 1.0, 1.0);
}

// draw at the artwork's own pixel size
void drawSprite(int id, double x, double y)
{
	if (id < 0 || id >= ASSET_SPRITE_COUNT) { drawMissingBox(x, y, 48, 48); return; }
	drawSpriteEx(id, x, y, gSprites[id].w, gSprites[id].h, 0, 1.0);
}

// draw scaled to a height, keeping the aspect ratio
void drawSpriteH(int id, double x, double y, double height, int flipX)
{
	double aspect;
	if (id < 0 || id >= ASSET_SPRITE_COUNT || gSprites[id].h == 0) {
		drawMissingBox(x, y, height, height);
		return;
	}
	aspect = (double)gSprites[id].w / gSprites[id].h;
	drawSpriteEx(id, x, y, height * aspect, height, flipX, 1.0);
}

void drawSpriteRect(int id, Rect r, int flipX, double alpha)
{
	drawSpriteEx(id, r.x, r.y, r.w, r.h, flipX, alpha);
}

// ---------------- character registration ----------------
//
//  Poses are drawn from different source images with different framings:
//  the sword-up frame has a blade above the head, the sword-down frame has
//  one thrust out to the side. Scaling each one to the same box would make
//  the character grow and shrink as she swings, and anchoring each one to
//  the bottom of its own canvas would make her feet leave the ground.
//
//  So every character sprite carries three fractions, measured off the
//  exported alpha channel by tools/measure_art.py:
//
//     bodyFrac - height of the figure alone, sword excluded, over canvas height
//     feetFrac - where the soles sit, as a fraction down from the canvas top
//     cxFrac   - horizontal centre of the figure, as a fraction of the width
//
//  drawCharacter() then sizes the quad so the FIGURE is bodyH tall, drops it
//  so the soles land exactly on groundY, and shifts it so the figure - not
//  the canvas - is centred on x. Poses can then be swapped freely mid-swing
//  and nothing moves that should not.

struct CharArt {
	double bodyFrac;
	double feetFrac;
	double cxFrac;
};

CharArt makeCharArt(double bodyFrac, double feetFrac, double cxFrac)
{
	CharArt a;
	a.bodyFrac = bodyFrac;
	a.feetFrac = feetFrac;
	a.cxFrac   = cxFrac;
	return a;
}

void drawCharacterTint(int id, CharArt art, double x, double groundY, double bodyH,
                       int flipX, double alpha, double tr, double tg, double tb)
{
	double drawH, drawW, drawX, drawY;

	if (id < 0 || id >= ASSET_SPRITE_COUNT || !gSprites[id].ok) {
		drawMissingBox(x - bodyH * 0.25, groundY, bodyH * 0.5, bodyH);
		return;
	}
	if (art.bodyFrac <= 0.01) art.bodyFrac = 1.0;

	// scale so the FIGURE, not the canvas, is bodyH tall
	drawH = bodyH / art.bodyFrac;
	drawW = drawH * (double)gSprites[id].w / gSprites[id].h;

	// drop it so the soles sit on the ground line
	drawY = groundY - drawH * (1.0 - art.feetFrac);

	// centre the figure horizontally; mirroring flips which side it sits on
	drawX = flipX ? (x - drawW * (1.0 - art.cxFrac))
	              : (x - drawW * art.cxFrac);

	drawSpriteTint(id, drawX, drawY, drawW, drawH, flipX, alpha, tr, tg, tb);
}

void drawCharacter(int id, CharArt art, double x, double groundY, double bodyH,
                   int flipX, double alpha)
{
	drawCharacterTint(id, art, x, groundY, bodyH, flipX, alpha, 1.0, 1.0, 1.0);
}

// ---------------- animation ----------------

// picks the sprite id of one frame of an animation
//   animId - an AnimId from AssetList.hpp
//   step   - any counter that grows over time, e.g. animFrame
int animSprite(int animId, long step)
{
	int first, count;
	if (animId < 0 || animId >= ASSET_ANIM_COUNT) return -1;
	first = ANIM_TABLE[animId].first;
	count = ANIM_TABLE[animId].count;
	if (count <= 0) return -1;
	if (step < 0) step = -step;
	return first + (int)(step % count);
}

void drawAnimH(int animId, long step, double x, double y, double height, int flipX)
{
	drawSpriteH(animSprite(animId, step), x, y, height, flipX);
}

#endif
