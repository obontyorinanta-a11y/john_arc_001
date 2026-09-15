#ifndef HUD_HPP
#define HUD_HPP

// -----------------------------------------------------------------
//  HUD.hpp - everything drawn on top of the battlefield.
//  Portrait, health, abilities with cooldown sweeps, score, lives,
//  the objective tracker and the boss bar.
// -----------------------------------------------------------------

#include <stdio.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "GameState.hpp"
#include "UI.hpp"
#include "Assets.hpp"
#include "Player.hpp"
#include "Units.hpp"
#include "World.hpp"

// one ability button with its cooldown sweep
void drawAbilityIcon(double x, double y, const char* key, const char* name,
                     int cooldown, int maxCooldown, int unlocked)
{
	double s = 46;
	Rect r = makeRect(x, y, s, s);
	double ready = (cooldown <= 0);

	fillRectAlpha(x, y, s, s, COL_PANEL, 0.85);

	if (!unlocked) {
		drawFrame(r, COL_GREY_DARK, 1);
		setColor(COL_GREY_DARK);
		drawTextCentered(x + s / 2, y + s / 2 - 6, "-", FONT_MED);
		setColor(COL_GREY_DARK);
		drawTextCentered(x + s / 2, y - 15, name, FONT_SMALL);
		return;
	}

	// the unavailable part drains from the bottom
	if (!ready && maxCooldown > 0) {
		double f = (double)cooldown / maxCooldown;
		fillRectAlpha(x, y, s, s * f, COL_BLUE_DARK, 0.85);
	}

	drawFrame(r, ready ? COL_GOLD : COL_GREY_DARK, ready ? 2 : 1);
	setColor(ready ? COL_GOLD : COL_GREY);
	drawTextCenteredBold(x + s / 2, y + s / 2 - 7, key, FONT_MED);
	setColor(ready ? COL_GREY : COL_GREY_DARK);
	drawTextCentered(x + s / 2, y - 15, name, FONT_SMALL);
}

// a gold coin, drawn with the same primitive the world pickup uses so the
// HUD icon and the thing on the road are recognisably the same object
void drawCoinIcon(double cx, double cy, double r)
{
	setColor(COL_GOLD_DIM);
	iFilledEllipse(cx, cy, r, r, 16);
	setColor(COL_GOLD);
	iFilledEllipse(cx, cy, r * 0.74, r * 0.74, 16);
	setColor(COL_GOLD_DIM);
	iFilledRectangle(cx - 1.5, cy - r * 0.42, 3, r * 0.84);
}

void drawHUD()
{
	char buf[96];
	int i;
	double barW = 240;
	double hpF  = (double)player.hp / player.maxHp;
	double hpS  = clampd(player.hpBarShown / player.maxHp, 0, 1);

	// ---- top strip ----
	fillRectAlpha(0, SCREEN_HEIGHT - 96, SCREEN_WIDTH, 96, COL_PANEL, 0.72);
	setColor(COL_GOLD_DIM);
	iLine(0, SCREEN_HEIGHT - 96, SCREEN_WIDTH, SCREEN_HEIGHT - 96);

	// ---- portrait ----
	{
		Rect pf = makeRect(16, SCREEN_HEIGHT - 86, 62, 76);
		fillRectAlpha(pf.x, pf.y, pf.w, pf.h, COL_BLUE_DARK, 0.9);
		drawSpriteEx(SPR_UI_JOAN_PORTRAIT, pf.x + 2, pf.y - 26, 58, 128, 0, 1.0);
		drawFrame(pf, COL_GOLD_DIM, 1);
	}

	// ---- health ----
	setColor(COL_WHITE);
	drawText(92, SCREEN_HEIGHT - 32, "JEANNE", FONT_SMALL);

	fillRectAlpha(92, SCREEN_HEIGHT - 56, barW, 16, COL_PANEL, 0.9);

	// The chasing bar is drawn UNDER the real one. When Joan is healed the
	// pale band lags behind and fills in; when she is hit it hangs out past
	// the red and drains away. Either way the change reads as motion.
	if (hpS > hpF) {
		setColor(COL_GREY_DARK);
		iFilledRectangle(92, SCREEN_HEIGHT - 56, barW * hpS, 16);
	} else if (hpS < hpF) {
		setColor(COL_GOLD);
		iFilledRectangle(92, SCREEN_HEIGHT - 56, barW * hpF, 16);
	}

	setColor(hpF > 0.35 ? COL_RED : COL_FLAME);
	iFilledRectangle(92, SCREEN_HEIGHT - 56, barW * (hpF < hpS ? hpF : hpS), 16);
	drawFrame(makeRect(92, SCREEN_HEIGHT - 56, barW, 16), COL_GOLD_DIM, 1);

	// the numbers live INSIDE the bar - outside, they ran into the road
	// progress bar in the middle of the screen
	sprintf(buf, "%d / %d", player.hp, player.maxHp);
	setColor(COL_WHITE);
	drawText(92 + barW - textWidth(buf, FONT_SMALL) - 7, SCREEN_HEIGHT - 54,
	         buf, FONT_SMALL);

	// ---- stage and path: the sword campaign only ----
	if (currentLevel == 3) {
		sprintf(buf, "STAGE %d   %s", player.stage,
		        heroPath == PATH_BLADE ? "BLADE" : heroPath == PATH_BOW ? "BOW" : "UNSWORN");
		setColor(COL_GOLD);
		drawText(92, SCREEN_HEIGHT - 80, buf, FONT_SMALL);
	}

	// ---- lives ----
	for (i = 0; i < lives; i++)
		drawDiamond(SCREEN_WIDTH - 40 - i * 26, SCREEN_HEIGHT - 26, 8, COL_RED);

	// ---- coins ----
	{
		double cx = SCREEN_WIDTH - 232;
		drawCoinIcon(cx, SCREEN_HEIGHT - 50, 11);
		sprintf(buf, "COINS  %d", player.coins);
		setColor(COL_GOLD);
		drawText(cx + 20, SCREEN_HEIGHT - 56, buf, FONT_MED);
	}

	sprintf(buf, "SCORE %06d", score);
	setColor(COL_GREY);
	drawText(SCREEN_WIDTH - 232, SCREEN_HEIGHT - 82, buf, FONT_SMALL);

	// Level 02 carries a bow. The quiver is not finite - a level built on
	// dodging should never also be a level you can run out of answers in -
	// so the HUD reports shots taken rather than shots left.
	if (currentLevel == 2) {
		sprintf(buf, "ARROWS  %d", player.arrowsFired);
		setColor(COL_GREY);
		drawText(SCREEN_WIDTH - 400, SCREEN_HEIGHT - 82, buf, FONT_SMALL);
	}

	// ---- level name, road progress, objective ----
	{
		double ox = SCREEN_WIDTH / 2 - 150;
		double f  = clampd(player.x / levelLength, 0, 1);

		sprintf(buf, "LEVEL %02d   %s", currentLevel, levelName(currentLevel));
		setColor(COL_GOLD_DIM);
		drawTextCentered(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30, buf, FONT_SMALL);

		fillRectAlpha(ox, SCREEN_HEIGHT - 52, 300, 10, COL_PANEL, 0.9);
		setColor(COL_BLUE);
		iFilledRectangle(ox, SCREEN_HEIGHT - 52, 300 * f, 10);

		// where the banner sits on the road
		setColor(COL_RED);
		iFilledRectangle(ox + 300 * clampd(gateX / levelLength, 0, 1) - 1,
		                 SCREEN_HEIGHT - 54, 2, 14);

		drawFrame(makeRect(ox, SCREEN_HEIGHT - 52, 300, 10), COL_GOLD_DIM, 1);
		drawDiamond(ox + 300 * f, SCREEN_HEIGHT - 47, 6, COL_GOLD);

		// the only line on screen that changes, so the player notices it
		if (levelPhase == PHASE_BOSS)
			sprintf(buf, currentLevel == 2 ? "DEFEAT THE HYDRA"
			                               : "DEFEAT THE ENEMY COMMANDER");
		else if (currentLevel == 2)
			sprintf(buf, "REACH THE HYDRA      SLAIN %d", levelKills);
		else if (currentLevel == 1)
			sprintf(buf, "CUT THE ROAD OPEN      SLAIN %d", levelKills);
		else
			sprintf(buf, "SLAIN %d      REACH THE BANNER", levelKills);

		setColor(levelPhase == PHASE_BOSS ? COL_RED_LIGHT : COL_GREY);
		drawTextCentered(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 76, buf, FONT_SMALL);
	}

	// ---- ability bar ----
	// One attack per level, so the other slots would be noise.
	{
		double bx = 20, by = 26;
		if (currentLevel == 2)
			drawAbilityIcon(bx, by, "F", "BOW",
			                player.shootCooldown, SHOOT_TICKS + ARROW_COOLDOWN, 1);
		else
			drawAbilityIcon(bx, by, "J", "SWORD", player.swingCooldown, SWING_COOLDOWN, 1);
		if (player.stage >= 2) {
			drawAbilityIcon(bx +  58, by, "K", "BOW",    player.bowCooldown,    BOW_COOLDOWN,    1);
			drawAbilityIcon(bx + 116, by, "Q", "MEND",   player.healCooldown,   HEAL_COOLDOWN,   1);
			drawAbilityIcon(bx + 174, by, "E", "CALL",   player.summonCooldown, SUMMON_COOLDOWN, player.stage >= 3);
			drawAbilityIcon(bx + 232, by, "R", "BANNER", player.ultCooldown,    ULT_COOLDOWN,    player.stage >= 4);
		}
	}

	// ---- boss bar ----
	if (currentLevel == 2) { drawHydraHPBar(); }

	for (i = 0; i < MAX_UNITS; i++) {
		if (!units[i].active || units[i].type != U_COMMANDER || units[i].state == US_DEAD) continue;
		if (units[i].state == US_SLEEP) continue;

		{
			double bw = 560;
			double bx = SCREEN_WIDTH / 2 - bw / 2;
			double by = SCREEN_HEIGHT - 130;
			double f  = (double)units[i].hp / units[i].maxHp;

			fillRectAlpha(bx - 4, by - 4, bw + 8, 30, COL_PANEL, 0.85);
			setColor(COL_RED);
			iFilledRectangle(bx, by, bw * f, 22);
			drawFrame(makeRect(bx, by, bw, 22), COL_GOLD, 2);

			// the two phase thresholds, marked on the bar
			setColor(COL_GOLD_DIM);
			iFilledRectangle(bx + bw * 0.333 - 1, by, 2, 22);
			iFilledRectangle(bx + bw * 0.666 - 1, by, 2, 22);

			setColor(COL_WHITE);
			drawTextCenteredBold(SCREEN_WIDTH / 2, by + 4, unitName(U_COMMANDER), FONT_SMALL);

			sprintf(buf, "%d", units[i].hp);
			setColor(COL_GREY);
			drawText(bx - 58, by + 4, buf, FONT_SMALL);

			sprintf(buf, "PHASE %d", units[i].phase + 1);
			setColor(COL_GOLD);
			drawText(bx + bw + 14, by + 4, buf, FONT_SMALL);
		}
		break;
	}

	// ---- low health vignette ----
	if (hpF < 0.3 && player.state != PS_DEAD) {
		double pulse = 0.10 + 0.06 * sin(uiTime * 0.2);
		fillRectAlpha(0, 0, SCREEN_WIDTH, 26, COL_RED, pulse);
		fillRectAlpha(0, SCREEN_HEIGHT - 26, SCREEN_WIDTH, 26, COL_RED, pulse);
		fillRectAlpha(0, 0, 26, SCREEN_HEIGHT, COL_RED, pulse);
		fillRectAlpha(SCREEN_WIDTH - 26, 0, 26, SCREEN_HEIGHT, COL_RED, pulse);
	}
}

#endif
