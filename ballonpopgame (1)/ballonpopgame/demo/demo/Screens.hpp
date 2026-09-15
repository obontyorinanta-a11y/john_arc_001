#ifndef SCREENS_HPP
#define SCREENS_HPP

// -----------------------------------------------------------------
//  Screens.hpp - one draw function and one input handler per state.
//
//  Phase 1 builds the menu family for real. STATE_PLAYING is still a
//  placeholder card; the gameplay modules replace it in later phases.
// -----------------------------------------------------------------

#include <stdio.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "GameState.hpp"
#include "Input.hpp"
#include "UI.hpp"
#include "Campaign.hpp"
#include "Game.hpp"

// ================= main menu =================

const int MENU_ITEM_COUNT = 4;

const char* menuLabels[MENU_ITEM_COUNT] = {
	"NEW GAME",
	"INSTRUCTIONS",
	"CREDITS",
	"EXIT"
};

int menuSelected = 0;

Rect menuItemRect(int i)
{
	double w = 320, h = 50, gap = 14;
	double x = SCREEN_WIDTH / 2 - w / 2;
	double top = 330;
	return makeRect(x, top - i * (h + gap), w, h);
}

// mouse hover selects, exactly like the keyboard does
void updateMenuHover()
{
	int i;
	for (i = 0; i < MENU_ITEM_COUNT; i++) {
		if (pointInRect(iMouseX, iMouseY, menuItemRect(i))) menuSelected = i;
	}
}

void activateMenuItem(int i)
{
	switch (i) {
	case 0: resetRun(); newRunPlayer(); changeState(STATE_LEVEL_SELECT); break;
	case 1: changeState(STATE_INSTRUCTIONS);             break;
	case 2: changeState(STATE_CREDITS);                  break;
	case 3: exit(0);                                     break;
	}
}

void drawMenuScreen()
{
	int i;
	char buf[64];

	// the intro poster, darkened so the title and the buttons stay readable
	drawSceneBackground(SPR_UI_INTRO_POSTER, 0.46);

	drawHangingBanner(150, 640, 108, 300, 0.0);
	drawHangingBanner(1050, 640, 108, 300, 1.6);

	// title block
	drawTitleText(SCREEN_WIDTH / 2, 560, "JOAN OF ARC", COL_GOLD, FONT_BIG);
	drawDivider(SCREEN_WIDTH / 2, 540, 210);
	setColor(COL_WHITE);
	drawTextCentered(SCREEN_WIDTH / 2, 505, "BANNER OF ORLEANS", FONT_MED);
	setColor(COL_GREY);
	drawTextCentered(SCREEN_WIDTH / 2, 480, "France, 1429", FONT_SMALL);

	for (i = 0; i < MENU_ITEM_COUNT; i++)
		drawButton(menuItemRect(i), menuLabels[i], i == menuSelected);

	// high score plate
	sprintf(buf, "BEST SCORE   %d", highScore);
	setColor(COL_GOLD_DIM);
	drawTextCentered(SCREEN_WIDTH / 2, 120, buf, FONT_SMALL);

	drawHintBar("ARROW KEYS or MOUSE to choose      ENTER or CLICK to confirm      ESC to quit");
	drawFadeOverlay();
}

void handleMenuKeys()
{
	if (specialTapped(GLUT_KEY_UP))   menuSelected = (menuSelected + MENU_ITEM_COUNT - 1) % MENU_ITEM_COUNT;
	if (specialTapped(GLUT_KEY_DOWN)) menuSelected = (menuSelected + 1) % MENU_ITEM_COUNT;
	if (keyTappedCh('w', 'W'))        menuSelected = (menuSelected + MENU_ITEM_COUNT - 1) % MENU_ITEM_COUNT;
	if (keyTappedCh('s', 'S'))        menuSelected = (menuSelected + 1) % MENU_ITEM_COUNT;

	if (keyTapped(KEY_ENTER))  activateMenuItem(menuSelected);
	if (keyTapped(KEY_ESCAPE)) exit(0);
}

// ================= instructions =================

void drawInstructionsScreen()
{
	Rect panel = makeRect(130, 70, 940, 520);
	double x = panel.x + 60;
	double y = 500;

	drawBackdrop();
	drawPanel(panel, 0.88);

	drawTitleText(SCREEN_WIDTH / 2, 540, "HOW TO FIGHT", COL_GOLD, FONT_BIG);
	drawDivider(SCREEN_WIDTH / 2, 520, 200);

	setColor(COL_WHITE);
	drawText(x, y, "MOVE", FONT_MED);
	setColor(COL_GREY);
	drawText(x + 220, y, "A / D   or   LEFT / RIGHT ARROW", FONT_MED);

	y -= 44;
	setColor(COL_WHITE);
	drawText(x, y, "JUMP", FONT_MED);
	setColor(COL_GREY);
	drawText(x + 220, y, "SPACE   or   W", FONT_MED);

	y -= 44;
	setColor(COL_WHITE);
	drawText(x, y, "SWORD", FONT_MED);
	setColor(COL_GREY);
	drawText(x + 220, y, "J   or   LEFT MOUSE BUTTON", FONT_MED);

	y -= 44;
	setColor(COL_WHITE);
	drawText(x, y, "BOW", FONT_MED);
	setColor(COL_GREY);
	drawText(x + 220, y, "K   or   RIGHT MOUSE BUTTON", FONT_MED);

	y -= 44;
	setColor(COL_WHITE);
	drawText(x, y, "SACRED MENDING", FONT_MED);
	setColor(COL_GREY);
	drawText(x + 220, y, "Q   -   heal yourself and nearby allies", FONT_MED);

	y -= 44;
	setColor(COL_WHITE);
	drawText(x, y, "ORIFLAMME CALL", FONT_MED);
	setColor(COL_GREY);
	drawText(x + 220, y, "E   -   summon companions to your side", FONT_MED);

	y -= 44;
	setColor(COL_WHITE);
	drawText(x, y, "BANNER OF WAR", FONT_MED);
	setColor(COL_GREY);
	drawText(x + 220, y, "R   -   stage 4 only, strikes every foe near you", FONT_MED);

	y -= 44;
	setColor(COL_WHITE);
	drawText(x, y, "PAUSE / MUSIC", FONT_MED);
	setColor(COL_GREY);
	drawText(x + 220, y, "P or ESC to pause      M toggles music", FONT_MED);

	y -= 48;
	setColor(COL_GOLD);
	drawText(x, y, "KNOW YOUR ENEMY", FONT_SMALL);
	y -= 24;
	setColor(COL_GREY);
	drawText(x, y, "PIKEMAN  heavy armour, but arrows punch through it", FONT_SMALL);
	y -= 22;
	drawText(x, y, "CROSSBOW  shoots from afar, falls quickly to the sword", FONT_SMALL);
	y -= 22;
	drawText(x, y, "BRUTE  no weakness, simply hit it until it stops", FONT_SMALL);

	drawHintBar("ESC or ENTER to go back");
	drawFadeOverlay();
}

void handleInstructionsKeys()
{
	if (keyTapped(KEY_ESCAPE) || keyTapped(KEY_ENTER)) changeState(STATE_MENU);
}

// ================= credits =================

void drawCreditsScreen()
{
	Rect panel = makeRect(300, 120, 600, 440);
	double y = 470;

	drawBackdrop();
	drawPanel(panel, 0.88);

	drawTitleText(SCREEN_WIDTH / 2, 510, "CREDITS", COL_GOLD, FONT_BIG);
	drawDivider(SCREEN_WIDTH / 2, 492, 150);

	setColor(COL_GREY);
	drawTextCentered(SCREEN_WIDTH / 2, y, "CSE-1200  SOFTWARE DEVELOPMENT I", FONT_SMALL);
	y -= 20;
	drawTextCentered(SCREEN_WIDTH / 2, y, "AHSANULLAH UNIVERSITY OF SCIENCE AND TECHNOLOGY", FONT_SMALL);

	y -= 50;
	setColor(COL_GOLD_DIM);
	drawTextCentered(SCREEN_WIDTH / 2, y, "TEAM", FONT_SMALL);
	y -= 34;
	setColor(COL_WHITE);
	drawTextCentered(SCREEN_WIDTH / 2, y, "Obonty Orin Anta", FONT_MED);      y -= 32;
	drawTextCentered(SCREEN_WIDTH / 2, y, "Sidana Tahrin", FONT_MED);         y -= 32;
	drawTextCentered(SCREEN_WIDTH / 2, y, "Tahsin Saba", FONT_MED);           y -= 32;
	drawTextCentered(SCREEN_WIDTH / 2, y, "Iffat Jahan", FONT_MED);

	y -= 52;
	setColor(COL_GOLD_DIM);
	drawTextCentered(SCREEN_WIDTH / 2, y, "SUPERVISED BY", FONT_SMALL);
	y -= 26;
	setColor(COL_GREY);
	drawTextCentered(SCREEN_WIDTH / 2, y, "Saha Reno   -   Md. Zahid Hossain", FONT_SMALL);

	y -= 40;
	setColor(COL_GREY_DARK);
	drawTextCentered(SCREEN_WIDTH / 2, y, "Built with iGraphics.  All art and code original.", FONT_SMALL);

	drawHintBar("ESC or ENTER to go back");
	drawFadeOverlay();
}

void handleCreditsKeys()
{
	if (keyTapped(KEY_ESCAPE) || keyTapped(KEY_ENTER)) changeState(STATE_MENU);
}

// ================= level select / campaign map =================
//
//  The painted map is the screen. Everything below is drawn on top of
//  it: one marker per location, a status plate over the painted one,
//  a back button and the "locked" message.
//
//  Progress itself lives in Campaign.hpp - this section only reads it.

int  levelSelected = 0;      // 0..TOTAL_LEVELS-1, the highlighted marker
int  backHovered   = 0;
long lockMsgUntil  = 0;      // uiTime at which the locked message expires

void showLockedMessage()
{
	lockMsgUntil = uiTime + LOCK_MSG_TICKS;
	sfx("hit");
}

// one location on the map. level is 1..TOTAL_LEVELS
void drawLevelMarker(int level)
{
	Rect   r         = levelMarkerRect(level);
	int    unlocked  = isLevelUnlocked(level);
	int    completed = isLevelCompleted(level);
	int    hot       = (levelSelected == level - 1);
	double pulse     = 0.5 + 0.5 * sin(uiTime * 0.08);
	double cx        = r.x + r.w / 2;
	char   buf[48];

	// the level you can actually march on breathes a gold glow; a
	// hovered locked one only gets a dull grey wash
	if (unlocked && !completed)
		fillRectAlpha(r.x - 8, r.y - 8, r.w + 16, r.h + 16,
		              COL_GOLD, (hot ? 0.20 : 0.09) + 0.10 * pulse);
	else if (hot)
		fillRectAlpha(r.x - 6, r.y - 6, r.w + 12, r.h + 12, COL_GREY_DARK, 0.30);

	// The map has its own place names painted on it and the marker sits
	// right on top of one, so the plate has to be all but opaque or the
	// painted text reads through our own second line.
	fillRectAlpha(r.x, r.y, r.w, r.h, COL_PANEL, unlocked ? 0.96 : 0.97);
	drawFrame(r, unlocked ? COL_GOLD : COL_GREY_DARK, (unlocked && hot) ? 3 : 1);

	// a pin above an open location, a padlock above a shut one
	if (unlocked) {
		drawDiamond(cx, r.y + r.h + 15, 8, COL_GOLD);
		drawDiamond(cx, r.y + r.h + 15, 4, COL_PANEL);
	} else {
		fillRectAlpha(cx - 14, r.y + r.h + 1, 28, 30, COL_PANEL, 0.97);
		drawFrame(makeRect(cx - 14, r.y + r.h + 1, 28, 30), COL_GREY_DARK, 1);
		drawPadlock(cx, r.y + r.h + 15, 11, COL_GREY);
	}

	// line 1 - the place
	setColor(completed ? COL_GREY : (unlocked ? COL_GOLD : COL_GREY_DARK));
	drawTextCenteredBold(cx, r.y + 36, levelName(level), FONT_MED);

	// line 2 - the mission number and its state
	if (completed)      sprintf(buf, "LEVEL %02d   COMPLETE", level);
	else if (!unlocked) sprintf(buf, "LEVEL %02d   LOCKED", level);
	else                sprintf(buf, "LEVEL %02d", level);

	setColor(completed ? COL_GREY : (unlocked ? COL_WHITE : COL_GREY_DARK));
	drawTextCentered(cx, r.y + 13, buf, FONT_SMALL);
}

// replaces the decorative status plate painted on the artwork with the
// real campaign figures, in the same place and the same size
void drawCampaignStatus()
{
	Rect r = campaignStatusRect();
	char buf[64];
	double y;
	int i;

	drawPanel(r, 0.97);

	setColor(COL_GOLD);
	drawTextCenteredBold(r.x + r.w / 2, r.y + r.h - 24, "CAMPAIGN STATUS", FONT_SMALL);
	drawDivider(r.x + r.w / 2, r.y + r.h - 34, r.w / 2 - 14);

	sprintf(buf, "COMPLETED   %d / %d", campaignCompletedCount(), TOTAL_LEVELS);
	setColor(COL_WHITE);
	drawTextCentered(r.x + r.w / 2, r.y + r.h - 58, buf, FONT_SMALL);

	y = r.y + r.h - 88;
	for (i = 1; i <= TOTAL_LEVELS; i++) {
		if (isLevelCompleted(i))     setColor(COL_GREY);
		else if (isLevelUnlocked(i)) setColor(COL_GOLD);
		else                         setColor(COL_GREY_DARK);

		sprintf(buf, "%02d  %s", i, levelName(i));
		drawText(r.x + 12, y, buf, FONT_SMALL);
		y -= 22;
	}
}

// fades out on its own, and any further click clears it early
void drawLockedMessage()
{
	double left = (double)(lockMsgUntil - uiTime);
	double a;
	Rect   r;

	if (left <= 0) return;
	a = (left < LOCK_MSG_FADE) ? left / LOCK_MSG_FADE : 1.0;

	r = makeRect(SCREEN_WIDTH / 2 - 250, 320, 500, 82);

	fillRectAlpha(r.x, r.y, r.w, r.h, COL_PANEL, 0.88 * a);
	fillRectAlpha(r.x, r.y, r.w, 2, COL_GOLD_DIM, a);
	fillRectAlpha(r.x, r.y + r.h - 2, r.w, 2, COL_GOLD_DIM, a);
	fillRectAlpha(r.x, r.y, 2, r.h, COL_GOLD_DIM, a);
	fillRectAlpha(r.x + r.w - 2, r.y, 2, r.h, COL_GOLD_DIM, a);

	// iText has no alpha, so the colour is mixed towards the panel instead
	setColorMix(COL_PANEL, COL_GOLD, a);
	drawTextCenteredBold(SCREEN_WIDTH / 2, r.y + 48, "LEVEL LOCKED", FONT_MED);
	setColorMix(COL_PANEL, COL_GREY, a);
	drawTextCentered(SCREEN_WIDTH / 2, r.y + 22,
	                 "COMPLETE THE PREVIOUS LEVEL TO UNLOCK", FONT_SMALL);
}

void drawLevelSelectScreen()
{
	int i;

	// The map is 1600x900 and the window is 1200x675 - the same 16:9 -
	// so it fills the screen at a uniform 0.75 and is never distorted.
	// It carries its own "KINGDOM OF FRANCE / CAMPAIGN MAP" title, so
	// none is drawn on top of it.
	drawSceneBackground(SPR_UI_CAMPAIGN_MAP, 0.0);

	for (i = 1; i <= TOTAL_LEVELS; i++) drawLevelMarker(i);

	drawCampaignStatus();
	drawButton(campaignBackRect(), "BACK", backHovered);
	drawLockedMessage();

	drawHintBar("CLICK a location to march      LEFT / RIGHT and ENTER also work      ESC for the main menu");
	drawFadeOverlay();
}

// the single gate into a level: a locked one can never get past here
void enterLevel(int level)
{
	if (!isLevelUnlocked(level)) {
		showLockedMessage();
		return;
	}
	lockMsgUntil = 0;
	currentLevel = level;
	changeState(STATE_STORY);
}

void handleLevelSelectKeys()
{
	if (specialTapped(GLUT_KEY_RIGHT) || keyTappedCh('d', 'D'))
		levelSelected = (levelSelected + 1) % TOTAL_LEVELS;
	if (specialTapped(GLUT_KEY_LEFT) || keyTappedCh('a', 'A'))
		levelSelected = (levelSelected + TOTAL_LEVELS - 1) % TOTAL_LEVELS;

	if (keyTapped(KEY_ENTER) || keyTapped(KEY_SPACE)) enterLevel(levelSelected + 1);
	if (keyTapped(KEY_ESCAPE)) changeState(STATE_MENU);
}

void updateLevelSelectHover()
{
	int i;
	for (i = 1; i <= TOTAL_LEVELS; i++) {
		if (pointInRect(iMouseX, iMouseY, levelMarkerRect(i))) levelSelected = i - 1;
	}
	backHovered = pointInRect(iMouseX, iMouseY, campaignBackRect());
}

void handleLevelSelectClick(int mx, int my)
{
	int i;

	lockMsgUntil = 0;            // clicking anywhere dismisses the message

	if (pointInRect(mx, my, campaignBackRect())) {
		changeState(STATE_MENU);
		return;
	}

	for (i = 1; i <= TOTAL_LEVELS; i++) {
		if (!pointInRect(mx, my, levelMarkerRect(i))) continue;
		levelSelected = i - 1;
		enterLevel(i);
		return;
	}
}

// ================= story card =================

int storyTimer = 0;   // auto-advance safety, counted in logic ticks

void drawStoryScreen()
{
	Rect panel = makeRect(180, 150, 840, 380);
	double y = 450;
	char buf[64];

	drawBackdrop();
	drawPanel(panel, 0.9);

	sprintf(buf, "MISSION %d", currentLevel);
	setColor(COL_GOLD_DIM);
	drawTextCentered(SCREEN_WIDTH / 2, 486, buf, FONT_SMALL);
	drawTitleText(SCREEN_WIDTH / 2, 500, levelName(currentLevel), COL_GOLD, FONT_BIG);

	y = 400;
	setColor(COL_WHITE);
	drawTextCentered(SCREEN_WIDTH / 2, y, "The siege has held for seven months.", FONT_MED);
	y -= 34;
	drawTextCentered(SCREEN_WIDTH / 2, y, "The garrison is starving and the walls are thin.", FONT_MED);
	y -= 34;
	drawTextCentered(SCREEN_WIDTH / 2, y, "A farm girl carries a banner to the gates,", FONT_MED);
	y -= 34;
	drawTextCentered(SCREEN_WIDTH / 2, y, "and the soldiers follow her.", FONT_MED);

	y -= 60;
	setColor(COL_GOLD);
	drawTextCentered(SCREEN_WIDTH / 2, y, "Break the siege.", FONT_MED);

	drawHintBar("ENTER or SPACE to begin");
	drawFadeOverlay();
}

void beginLevelFromStory()
{
	startLevel(currentLevel);
	changeState(STATE_PLAYING);
}

void handleStoryKeys()
{
	if (keyTapped(KEY_ENTER) || keyTapped(KEY_SPACE)) beginLevelFromStory();
	if (keyTapped(KEY_ESCAPE)) { musicStop(); changeState(STATE_LEVEL_SELECT); }
}

// ================= gameplay =================

void drawPlayingScreen()
{
	drawGameplay();
	drawFadeOverlay();
}

void handlePlayingKeys()
{
	readPlayerInput();
	if (keyTappedCh('p', 'P') || keyTapped(KEY_ESCAPE)) changeState(STATE_PAUSED);
	if (keyTappedCh('m', 'M')) musicToggle();
}

// ================= pause =================

const int PAUSE_ITEM_COUNT = 3;

const char* pauseLabels[PAUSE_ITEM_COUNT] = {
	"RESUME",
	"RESTART LEVEL",
	"ABANDON - CAMPAIGN MAP"
};

int pauseSelected = 0;

Rect pauseItemRect(int i)
{
	double w = 340, h = 46, gap = 12;
	return makeRect(SCREEN_WIDTH / 2 - w / 2, 330 - i * (h + gap), w, h);
}

void updatePauseHover()
{
	int i;
	for (i = 0; i < PAUSE_ITEM_COUNT; i++) {
		if (pointInRect(iMouseX, iMouseY, pauseItemRect(i))) pauseSelected = i;
	}
}

void activatePauseItem(int i)
{
	switch (i) {
	case 0: changeState(STATE_PLAYING); break;
	case 1: startLevel(currentLevel); changeState(STATE_PLAYING); break;
	case 2: musicStop(); changeState(STATE_LEVEL_SELECT); break;
	}
}

void drawPauseScreen()
{
	int i;

	// keep the frozen battlefield visible behind the overlay
	drawPlayingScreen();
	fillRectAlpha(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COL_PANEL, 0.72);

	drawPanel(makeRect(380, 250, 440, 300), 0.9);
	drawTitleText(SCREEN_WIDTH / 2, 500, "PAUSED", COL_GOLD, FONT_BIG);
	drawDivider(SCREEN_WIDTH / 2, 482, 120);

	for (i = 0; i < PAUSE_ITEM_COUNT; i++)
		drawButton(pauseItemRect(i), pauseLabels[i], i == pauseSelected);

	drawHintBar("P or ESC to resume");
}

void handlePauseKeys()
{
	if (specialTapped(GLUT_KEY_UP))   pauseSelected = (pauseSelected + PAUSE_ITEM_COUNT - 1) % PAUSE_ITEM_COUNT;
	if (specialTapped(GLUT_KEY_DOWN)) pauseSelected = (pauseSelected + 1) % PAUSE_ITEM_COUNT;
	if (keyTapped(KEY_ENTER))         activatePauseItem(pauseSelected);
	if (keyTappedCh('p', 'P') || keyTapped(KEY_ESCAPE)) changeState(STATE_PLAYING);
}

// ================= path choice (hero progression) =================

int pathSelected = 0;   // 0 = blade, 1 = bow

Rect pathCardRect(int i)
{
	double w = 300, h = 260;
	double cx = (i == 0) ? SCREEN_WIDTH / 2 - 180 : SCREEN_WIDTH / 2 + 180;
	return makeRect(cx - w / 2, 220, w, h);
}

void updatePathHover()
{
	int i;
	for (i = 0; i < 2; i++) {
		if (pointInRect(iMouseX, iMouseY, pathCardRect(i))) pathSelected = i;
	}
}

void drawPathChoiceScreen()
{
	int i;
	drawBackdrop();

	drawTitleText(SCREEN_WIDTH / 2, 560, "CHOOSE YOUR PATH", COL_GOLD, FONT_BIG);
	drawDivider(SCREEN_WIDTH / 2, 540, 220);
	setColor(COL_GREY);
	drawTextCentered(SCREEN_WIDTH / 2, 508, "This choice shapes the rest of the campaign.", FONT_SMALL);

	for (i = 0; i < 2; i++) {
		Rect r = pathCardRect(i);
		int sel = (i == pathSelected);

		drawPanel(r, sel ? 0.92 : 0.7);
		if (sel) drawFrame(r, COL_GOLD, 2);

		setColor(sel ? COL_GOLD : COL_GREY);
		drawTextCentered(r.x + r.w / 2, r.y + r.h - 50,
		                 (i == 0) ? "THE BLADE" : "THE BOW", FONT_BIG);

		setColor(COL_WHITE);
		if (i == 0) {
			drawTextCentered(r.x + r.w / 2, r.y + 150, "Heavier sword damage", FONT_SMALL);
			drawTextCentered(r.x + r.w / 2, r.y + 126, "Wider swing arc", FONT_SMALL);
			drawTextCentered(r.x + r.w / 2, r.y + 102, "More armour", FONT_SMALL);
		} else {
			drawTextCentered(r.x + r.w / 2, r.y + 150, "Fire arrows at range", FONT_SMALL);
			drawTextCentered(r.x + r.w / 2, r.y + 126, "Faster movement", FONT_SMALL);
			drawTextCentered(r.x + r.w / 2, r.y + 102, "Weaker in melee", FONT_SMALL);
		}

		if (sel) {
			drawDiamond(r.x + r.w / 2, r.y + 56, 7, COL_GOLD);
			setColor(COL_GOLD_DIM);
			drawTextCentered(r.x + r.w / 2, r.y + 24, "SELECTED", FONT_SMALL);
		}
	}

	drawHintBar("LEFT / RIGHT to compare      ENTER or CLICK to swear the oath");
	drawFadeOverlay();
}

void confirmPath()
{
	heroPath = pathSelected + 1;
	setHeroStage(2);
	sfx("levelup");
	// back to the map: the player picks the next mission, it never
	// starts on its own
	changeState(STATE_LEVEL_SELECT);
}

void handlePathChoiceKeys()
{
	if (specialTapped(GLUT_KEY_LEFT))  pathSelected = 0;
	if (specialTapped(GLUT_KEY_RIGHT)) pathSelected = 1;
	if (keyTappedCh('a', 'A'))         pathSelected = 0;
	if (keyTappedCh('d', 'D'))         pathSelected = 1;
	if (keyTapped(KEY_ENTER))          confirmPath();
}

// ================= level complete =================

// Where the run goes after a finished level.
//
// This is the ONLY place a level is marked complete, and it is reached
// only from the level-complete screen, so dying and restarting can
// never unlock anything.
void advanceAfterLevel()
{
	markLevelCompleted(currentLevel);

	if (currentLevel == 1 && heroPath == 0) {
		changeState(STATE_PATH_CHOICE);   // which then returns to the map
		return;
	}
	if (currentLevel >= TOTAL_LEVELS) {
		if (score > highScore) highScore = score;
		musicStop();
		changeState(STATE_VICTORY);
		return;
	}
	// the next level is open now, but the player still has to choose it
	changeState(STATE_LEVEL_SELECT);
}

void drawLevelCompleteScreen()
{
	char buf[80];
	Rect panel = makeRect(320, 210, 560, 300);

	drawBackdrop();
	drawPanel(panel, 0.9);

	drawTitleText(SCREEN_WIDTH / 2, 470, "MISSION COMPLETE", COL_GOLD, FONT_BIG);
	drawDivider(SCREEN_WIDTH / 2, 452, 190);

	setColor(COL_WHITE);
	sprintf(buf, "%s  -  %s", levelName(currentLevel),
	        currentLevel == 2 ? "HYDRA DEFEATED" : "COMMANDER DEFEATED");
	drawTextCentered(SCREEN_WIDTH / 2, 412, buf, FONT_SMALL);

	setColor(COL_GREY);
	sprintf(buf, "COINS COLLECTED       %d", player.coins);
	drawTextCentered(SCREEN_WIDTH / 2, 372, buf, FONT_MED);
	sprintf(buf, "REMAINING HP       %d / %d", player.hp, player.maxHp);
	drawTextCentered(SCREEN_WIDTH / 2, 342, buf, FONT_MED);
	sprintf(buf, "ENEMIES SLAIN         %d", player.kills);
	drawTextCentered(SCREEN_WIDTH / 2, 312, buf, FONT_MED);
	sprintf(buf, "SCORE THIS RUN     %d", score);
	drawTextCentered(SCREEN_WIDTH / 2, 282, buf, FONT_SMALL);

	setColor(COL_GOLD_DIM);
	if (currentLevel == 1 && heroPath == 0)
		drawTextCentered(SCREEN_WIDTH / 2, 244, "Your path awaits.", FONT_SMALL);
	else if (currentLevel >= TOTAL_LEVELS)
		drawTextCentered(SCREEN_WIDTH / 2, 244, "The commander has fallen.", FONT_SMALL);
	else {
		sprintf(buf, "%s IS OPEN ON THE CAMPAIGN MAP", levelName(currentLevel + 1));
		drawTextCentered(SCREEN_WIDTH / 2, 244, buf, FONT_SMALL);
	}

	drawHintBar("ENTER to continue");
	drawFadeOverlay();
}

void handleLevelCompleteKeys()
{
	if (keyTapped(KEY_ENTER)) advanceAfterLevel();
}

// ================= game over =================

void drawGameOverScreen()
{
	char buf[80];

	drawBackdrop();
	fillRectAlpha(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COL_RED, 0.16);
	drawPanel(makeRect(340, 200, 520, 300), 0.94);
	drawFrame(makeRect(340, 200, 520, 300), COL_RED, 2);

	drawTitleText(SCREEN_WIDTH / 2, 450, "THE BANNER FALLS", COL_RED_LIGHT, FONT_BIG);
	drawDivider(SCREEN_WIDTH / 2, 430, 180);

	setColor(COL_WHITE);
	drawTextCentered(SCREEN_WIDTH / 2, 390, "The line is broken, but the war is not over.", FONT_SMALL);

	setColor(COL_WHITE);
	sprintf(buf, "FINAL SCORE   %d", score);
	drawTextCentered(SCREEN_WIDTH / 2, 330, buf, FONT_MED);
	sprintf(buf, "REACHED       %s", levelName(currentLevel));
	setColor(COL_GOLD_DIM);
	drawTextCentered(SCREEN_WIDTH / 2, 296, buf, FONT_SMALL);

	setColor(COL_GOLD);
	drawTextCentered(SCREEN_WIDTH / 2, 245, "R  -  take the field again", FONT_MED);

	drawHintBar("R to retry      ENTER for the main menu");
	drawFadeOverlay();
}

void handleGameOverKeys()
{
	if (keyTappedCh('r', 'R')) {
		if (score > highScore) highScore = score;
		musicStop();
		score = 0;
		lives = START_LIVES;
		newRunPlayer();
		setHeroStage(currentLevel);
		startLevel(currentLevel);
		changeState(STATE_PLAYING);
	}
	if (keyTapped(KEY_ENTER) || keyTapped(KEY_ESCAPE)) {
		if (score > highScore) highScore = score;
		musicStop();
		changeState(STATE_MENU);
	}
}

// ================= victory =================

void drawVictoryScreen()
{
	char buf[80];
	int i;

	drawBackdrop();

	// rays of light behind the panel
	for (i = 0; i < 12; i++) {
		double a = 0.05 + 0.03 * sin(uiTime * 0.05 + i);
		fillRectAlpha(SCREEN_WIDTH / 2 - 300 + i * 50, 120, 18, 460, COL_GOLD, a);
	}

	drawHangingBanner(210, 640, 100, 280, 0.4);
	drawHangingBanner(990, 640, 100, 280, 2.1);
	drawPanel(makeRect(360, 210, 480, 290), 0.88);

	drawTitleText(SCREEN_WIDTH / 2, 460, "FRANCE ENDURES", COL_GOLD, FONT_BIG);
	drawDivider(SCREEN_WIDTH / 2, 440, 180);

	setColor(COL_WHITE);
	drawTextCentered(SCREEN_WIDTH / 2, 400, "The siege is broken and the road to Reims is open.", FONT_SMALL);

	sprintf(buf, "FINAL SCORE   %d", score);
	setColor(COL_GOLD);
	drawTextCentered(SCREEN_WIDTH / 2, 340, buf, FONT_MED);

	setColor(COL_GREY);
	sprintf(buf, "PATH TAKEN    %s", (heroPath == 2) ? "THE BOW" : "THE BLADE");
	drawTextCentered(SCREEN_WIDTH / 2, 300, buf, FONT_SMALL);
	sprintf(buf, "BEST SCORE    %d", highScore);
	drawTextCentered(SCREEN_WIDTH / 2, 272, buf, FONT_SMALL);

	drawHintBar("ENTER for the credits");
	drawFadeOverlay();
}

void handleVictoryKeys()
{
	if (keyTapped(KEY_ENTER)) changeState(STATE_CREDITS);
	if (keyTapped(KEY_ESCAPE)) changeState(STATE_MENU);
}

// ================= dispatchers =================

void drawCurrentScreen()
{
	switch (gameState) {
	case STATE_MENU:           drawMenuScreen();          break;
	case STATE_INSTRUCTIONS:   drawInstructionsScreen();  break;
	case STATE_CREDITS:        drawCreditsScreen();       break;
	case STATE_LEVEL_SELECT:   drawLevelSelectScreen();   break;
	case STATE_STORY:          drawStoryScreen();         break;
	case STATE_PLAYING:        drawPlayingScreen();       break;
	case STATE_PAUSED:         drawPauseScreen();         break;
	case STATE_PATH_CHOICE:    drawPathChoiceScreen();    break;
	case STATE_LEVEL_COMPLETE: drawLevelCompleteScreen(); break;
	case STATE_GAMEOVER:       drawGameOverScreen();      break;
	case STATE_VICTORY:        drawVictoryScreen();       break;
	}
}

void handleCurrentKeys()
{
	switch (gameState) {
	case STATE_MENU:           handleMenuKeys();          break;
	case STATE_INSTRUCTIONS:   handleInstructionsKeys();  break;
	case STATE_CREDITS:        handleCreditsKeys();       break;
	case STATE_LEVEL_SELECT:   handleLevelSelectKeys();   break;
	case STATE_STORY:          handleStoryKeys();         break;
	case STATE_PLAYING:        handlePlayingKeys();       break;
	case STATE_PAUSED:         handlePauseKeys();         break;
	case STATE_PATH_CHOICE:    handlePathChoiceKeys();    break;
	case STATE_LEVEL_COMPLETE: handleLevelCompleteKeys(); break;
	case STATE_GAMEOVER:       handleGameOverKeys();      break;
	case STATE_VICTORY:        handleVictoryKeys();       break;
	}
}

// hover tracking, called once per frame from iDraw
void updateCurrentHover()
{
	switch (gameState) {
	case STATE_MENU:         updateMenuHover();        break;
	case STATE_LEVEL_SELECT: updateLevelSelectHover(); break;
	case STATE_PAUSED:       updatePauseHover();       break;
	case STATE_PATH_CHOICE:  updatePathHover();        break;
	}
}

// left click on the current screen
void handleCurrentClick(int mx, int my)
{
	int i;
	switch (gameState) {
	case STATE_MENU:
		for (i = 0; i < MENU_ITEM_COUNT; i++)
			if (pointInRect(mx, my, menuItemRect(i))) activateMenuItem(i);
		break;
	case STATE_LEVEL_SELECT:
		handleLevelSelectClick(mx, my);
		break;
	case STATE_PAUSED:
		for (i = 0; i < PAUSE_ITEM_COUNT; i++)
			if (pointInRect(mx, my, pauseItemRect(i))) activatePauseItem(i);
		break;
	case STATE_PATH_CHOICE:
		for (i = 0; i < 2; i++)
			if (pointInRect(mx, my, pathCardRect(i))) { pathSelected = i; confirmPath(); }
		break;
	case STATE_INSTRUCTIONS:
	case STATE_CREDITS:
		changeState(STATE_MENU);
		break;
	case STATE_STORY:
		beginLevelFromStory();
		break;
	case STATE_LEVEL_COMPLETE:
		advanceAfterLevel();
		break;
	}
}

#endif
