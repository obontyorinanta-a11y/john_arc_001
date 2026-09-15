// ---------------------------------------------------------------
//  AssetList.hpp   ***  AUTO-GENERATED FILE - DO NOT EDIT BY HAND ***
//
//  Produced by tools/prepare_assets.py
//  Re-run that script after adding or changing any artwork.
// ---------------------------------------------------------------

#ifndef ASSETLIST_HPP
#define ASSETLIST_HPP

#define ASSET_SPRITE_COUNT 38
#define ASSET_ANIM_COUNT   20

// every sprite the pipeline exported, in table order
enum SpriteId {
	SPR_PLAYER_JOAN_SIDE_00,
	SPR_UI_JOAN_PORTRAIT,
	SPR_ENEMY_BRUTE_IDLE_00,
	SPR_BACKGROUND_FIELD_ORLEANS,
	SPR_UI_INTRO_POSTER,
	SPR_UI_CAMPAIGN_MAP,
	SPR_PLAYER_JOAN_RUN_00,
	SPR_PLAYER_JOAN_RUN_01,
	SPR_PLAYER_JOAN_RUN_02,
	SPR_PLAYER_JOAN_RUN_03,
	SPR_PLAYER_JOAN_RUN_04,
	SPR_PLAYER_JOAN_RUN_05,
	SPR_PLAYER_JOAN_RUN_06,
	SPR_PLAYER_JOAN_RUN_07,
	SPR_PLAYER_JOAN_RUN_08,
	SPR_PLAYER_JOAN_RUN_09,
	SPR_PLAYER_JOAN_RUN_10,
	SPR_PLAYER_JOAN_RUN_11,
	SPR_PLAYER_JOAN_SWORDUP,
	SPR_PLAYER_JOAN_SWORDDOWN,
	SPR_ENEMY_SOLDIER_SWORDUP,
	SPR_ENEMY_SOLDIER_SWORDDOWN,
	SPR_BOSS_COMMANDER_IDLE,
	SPR_PLAYER_ARCHER_RUN_00,
	SPR_PLAYER_ARCHER_RUN_01,
	SPR_PLAYER_ARCHER_RUN_02,
	SPR_PLAYER_ARCHER_RUN_03,
	SPR_PLAYER_ARCHER_RUN_04,
	SPR_PLAYER_ARCHER_SHOOT_00,
	SPR_PLAYER_ARCHER_SHOOT_01,
	SPR_PLAYER_ARCHER_SHOOT_02,
	SPR_PLAYER_ARCHER_SHOOT_03,
	SPR_ITEMS_ARROW_BOLT,
	SPR_BOSS_HYDRA,
	SPR_EFFECTS_FIREBALL,
	SPR_BACKGROUND_MONTCLAIR,
	SPR_ENEMY_HEAVY_SWORDUP,
	SPR_ENEMY_HEAVY_SWORDDOWN,
	SPR_COUNT
};

struct AssetEntry {
	const char* path;   // relative to the working directory (the project folder)
	int w, h;           // real pixel size of the artwork
	int texW, texH;     // power-of-two canvas it sits in
};

static const AssetEntry ASSET_TABLE[38] = {
	{ "Assets/player/joan_side_00.png", 287, 512, 512, 512 },
	{ "Assets/ui/joan_portrait.png", 235, 512, 256, 512 },
	{ "Assets/enemy/brute_idle_00.png", 256, 187, 256, 256 },
	{ "Assets/background/field_orleans.png", 1600, 800, 2048, 1024 },
	{ "Assets/ui/intro_poster.png", 1019, 573, 1024, 1024 },
	{ "Assets/ui/campaign_map.png", 1600, 900, 2048, 1024 },
	{ "Assets/player/joan_run_00.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_01.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_02.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_03.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_04.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_05.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_06.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_07.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_08.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_09.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_10.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_run_11.png", 247, 256, 256, 256 },
	{ "Assets/player/joan_swordup.png", 275, 384, 512, 512 },
	{ "Assets/player/joan_sworddown.png", 384, 286, 512, 512 },
	{ "Assets/enemy/soldier_swordup.png", 186, 320, 256, 512 },
	{ "Assets/enemy/soldier_sworddown.png", 320, 238, 512, 256 },
	{ "Assets/boss/commander_idle.png", 328, 512, 512, 512 },
	{ "Assets/player/archer_run_00.png", 236, 256, 256, 256 },
	{ "Assets/player/archer_run_01.png", 236, 256, 256, 256 },
	{ "Assets/player/archer_run_02.png", 236, 256, 256, 256 },
	{ "Assets/player/archer_run_03.png", 236, 256, 256, 256 },
	{ "Assets/player/archer_run_04.png", 236, 256, 256, 256 },
	{ "Assets/player/archer_shoot_00.png", 236, 256, 256, 256 },
	{ "Assets/player/archer_shoot_01.png", 236, 256, 256, 256 },
	{ "Assets/player/archer_shoot_02.png", 236, 256, 256, 256 },
	{ "Assets/player/archer_shoot_03.png", 236, 256, 256, 256 },
	{ "Assets/items/arrow_bolt.png", 57, 12, 64, 16 },
	{ "Assets/boss/hydra.png", 512, 298, 512, 512 },
	{ "Assets/effects/fireball.png", 256, 132, 256, 256 },
	{ "Assets/background/montclair.png", 1280, 904, 2048, 1024 },
	{ "Assets/enemy/heavy_swordup.png", 214, 320, 256, 512 },
	{ "Assets/enemy/heavy_sworddown.png", 216, 320, 256, 512 },
};

// frame sequences, grouped from _00 _01 _02 style file names
enum AnimId {
	ANIM_PLAYER_JOAN_SIDE,
	ANIM_UI_JOAN_PORTRAIT,
	ANIM_ENEMY_BRUTE_IDLE,
	ANIM_BACKGROUND_FIELD_ORLEANS,
	ANIM_UI_INTRO_POSTER,
	ANIM_UI_CAMPAIGN_MAP,
	ANIM_PLAYER_JOAN_RUN,
	ANIM_PLAYER_JOAN_SWORDUP,
	ANIM_PLAYER_JOAN_SWORDDOWN,
	ANIM_ENEMY_SOLDIER_SWORDUP,
	ANIM_ENEMY_SOLDIER_SWORDDOWN,
	ANIM_BOSS_COMMANDER_IDLE,
	ANIM_PLAYER_ARCHER_RUN,
	ANIM_PLAYER_ARCHER_SHOOT,
	ANIM_ITEMS_ARROW_BOLT,
	ANIM_BOSS_HYDRA,
	ANIM_EFFECTS_FIREBALL,
	ANIM_BACKGROUND_MONTCLAIR,
	ANIM_ENEMY_HEAVY_SWORDUP,
	ANIM_ENEMY_HEAVY_SWORDDOWN,
	ANIM_COUNT
};

struct AnimEntry {
	int first;   // index of frame 0 inside ASSET_TABLE
	int count;   // number of frames
};

static const AnimEntry ANIM_TABLE[20] = {
	{ 0, 1 },
	{ 1, 1 },
	{ 2, 1 },
	{ 3, 1 },
	{ 4, 1 },
	{ 5, 1 },
	{ 6, 12 },
	{ 18, 1 },
	{ 19, 1 },
	{ 20, 1 },
	{ 21, 1 },
	{ 22, 1 },
	{ 23, 5 },
	{ 28, 4 },
	{ 32, 1 },
	{ 33, 1 },
	{ 34, 1 },
	{ 35, 1 },
	{ 36, 1 },
	{ 37, 1 },
};

#endif
