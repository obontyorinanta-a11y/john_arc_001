#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

// -----------------------------------------------------------------
//  GameState.hpp - the state machine and the few globals that
//  survive across screens (level, score, lives).
//
//  RULE: never assign to gameState directly, always call changeState()
//  so the fade, the timers and later the music stay in sync.
// -----------------------------------------------------------------

#include "Config.hpp"

enum GameStateId {
	STATE_MENU = 0,
	STATE_INSTRUCTIONS,
	STATE_CREDITS,
	STATE_LEVEL_SELECT,
	STATE_STORY,
	STATE_PLAYING,
	STATE_PAUSED,
	STATE_PATH_CHOICE,
	STATE_LEVEL_COMPLETE,
	STATE_GAMEOVER,
	STATE_VICTORY,
	STATE_COUNT
};

int gameState     = STATE_MENU;
int previousState = STATE_MENU;

int fadeTimer  = 0;    // counts down after every state change
long uiTime    = 0;    // advances every logic tick, drives UI animation
long animFrame = 0;    // advances every animation tick

// ---- run progress (filled in properly in later phases) ----
int currentLevel = 1;
int score        = 0;
int highScore    = 0;
int lives        = START_LIVES;
int heroPath     = 0;   // 0 = not chosen yet, 1 = Blade, 2 = Bow

void changeState(int newState)
{
	previousState = gameState;
	gameState     = newState;
	fadeTimer     = FADE_TICKS;
}

// Reset everything that belongs to a single playthrough.
// Campaign unlocks live in Campaign.hpp and are deliberately NOT touched
// here, so returning to the menu and pressing NEW GAME keeps them.
void resetRun()
{
	currentLevel = 1;
	score        = 0;
	lives        = START_LIVES;
	heroPath     = 0;
}

const char* stateName(int s)
{
	switch (s) {
	case STATE_MENU:           return "MENU";
	case STATE_INSTRUCTIONS:   return "INSTRUCTIONS";
	case STATE_CREDITS:        return "CREDITS";
	case STATE_LEVEL_SELECT:   return "LEVEL SELECT";
	case STATE_STORY:          return "STORY";
	case STATE_PLAYING:        return "PLAYING";
	case STATE_PAUSED:         return "PAUSED";
	case STATE_PATH_CHOICE:    return "PATH CHOICE";
	case STATE_LEVEL_COMPLETE: return "LEVEL COMPLETE";
	case STATE_GAMEOVER:       return "GAME OVER";
	case STATE_VICTORY:        return "VICTORY";
	}
	return "UNKNOWN";
}

const char* levelName(int n)
{
	switch (n) {
	case 1: return "BRITTANY";
	case 2: return "THE MONTCLAIR";
	case 3: return "ORLEANS";
	}
	return "UNKNOWN FIELD";
}

#endif
