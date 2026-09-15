#ifndef CAMPAIGN_HPP
#define CAMPAIGN_HPP

// -----------------------------------------------------------------
//  Campaign.hpp - campaign progress and the geometry of the map.
//
//  Holds WHICH levels are unlocked and completed, and WHERE each one
//  sits on the campaign map artwork. It holds no drawing code and no
//  input code: the Level Select screen itself lives in Screens.hpp
//  with every other screen.
//
//  RULE: campaign progress belongs to the whole session, not to one
//  run. resetRun() must never touch it, or "main menu -> new game"
//  would silently relock levels the player already earned.
// -----------------------------------------------------------------

#include "Config.hpp"
#include "Utils.hpp"

// ---------------- progress ----------------

struct CampaignProgress {
	bool level1Unlocked;
	bool level2Unlocked;
	bool level3Unlocked;

	bool level1Completed;
	bool level2Completed;
	bool level3Completed;
};

// Defined once, at program start, and never re-initialised.
//
// Brittany and The Montclair are both open from the first launch, so
// level 02 can be reached without replaying level 01 first. Orleans still
// has to be earned: markLevelCompleted(2) is the only thing that opens it,
// and that only runs when the Hydra actually falls.
CampaignProgress campaign = {
	true,  true,  false,      // unlocked
	false, false, false       // completed
};

bool isLevelUnlocked(int level)
{
	switch (level) {
	case 1: return campaign.level1Unlocked;
	case 2: return campaign.level2Unlocked;
	case 3: return campaign.level3Unlocked;
	}
	return false;
}

bool isLevelCompleted(int level)
{
	switch (level) {
	case 1: return campaign.level1Completed;
	case 2: return campaign.level2Completed;
	case 3: return campaign.level3Completed;
	}
	return false;
}

// Sequential unlocking: finishing a level opens the next one and
// nothing else. There is no path that can skip a level.
void markLevelCompleted(int level)
{
	switch (level) {
	case 1:
		campaign.level1Completed = true;
		campaign.level2Unlocked  = true;
		break;
	case 2:
		campaign.level2Completed = true;
		campaign.level3Unlocked  = true;
		break;
	case 3:
		campaign.level3Completed = true;
		break;
	}
}

int campaignCompletedCount()
{
	int n = 0;
	if (campaign.level1Completed) n++;
	if (campaign.level2Completed) n++;
	if (campaign.level3Completed) n++;
	return n;
}

// ---------------- map geometry ----------------
//
// The campaign map art is 1600x900 and the window is 1200x675, so the
// artwork is drawn full screen at exactly 0.75 scale and is never
// distorted. These centres are the painted place-name plates of the
// artwork converted into game coordinates:
//
//     screen x = art x * 0.75
//     screen y = (900 - art y) * 0.75      (iGraphics origin is bottom-left)
//
// A marker is drawn a little larger than the painted plate so it
// covers it completely and the level number replaces the painted text.

const double MARKER_W = 196;
const double MARKER_H = 66;

struct MapPoint { double cx, cy; };

static const MapPoint LEVEL_MAP_POINT[TOTAL_LEVELS] = {
	{ 283, 272 },   // BRITTANY       - left, above the coast
	{ 594, 468 },   // THE MONTCLAIR  - upper middle, below the mountains
	{ 849, 178 }    // ORLEANS        - lower right, on the dry plain
};

// level is 1..TOTAL_LEVELS
Rect levelMarkerRect(int level)
{
	int i = clampi(level, 1, TOTAL_LEVELS) - 1;
	return makeRect(LEVEL_MAP_POINT[i].cx - MARKER_W / 2,
	                LEVEL_MAP_POINT[i].cy - MARKER_H / 2,
	                MARKER_W, MARKER_H);
}

// bottom centre, over open water in the artwork
Rect campaignBackRect()
{
	return makeRect(500, 44, 200, 42);   // clear of the hint bar at y 0..34
}

// sits exactly on the painted "CAMPAIGN STATUS" plate of the artwork
Rect campaignStatusRect()
{
	return makeRect(1006, 506, 186, 156);
}

#endif
