// The level sequence: current and next level, the level table, the level switch step and the level load.
#ifndef ZOOSTERS_TFB_LEVEL_LEVELMANAGER_H
#define ZOOSTERS_TFB_LEVEL_LEVELMANAGER_H

#include "zoosters/base.h"

// In table order; the last two are requests beyond the table.
enum GameLevel {
	LEVEL_NONE = -1, // before the first level (boot)
	LEVEL_KINGOFNY = 0,
	LEVEL_ESCAPE = 1,
	LEVEL_STREET = 2,
	LEVEL_MUTINY = 3,
	LEVEL_JUNGLE = 4,
	LEVEL_RAVE = 5,
	LEVEL_LEMURS = 6,
	LEVEL_BANQUET = 7,
	LEVEL_COMING = 8,
	LEVEL_BEACH = 9,
	LEVEL_RESCUE = 10,
	LEVEL_BATTLE = 11,
	LEVEL_SHUFFLE = 12,
	LEVEL_GOLF = 13,
	LEVEL_MAP = 14,
	LEVEL_TITLE = 15,
	LEVEL_COUNT = 16,
	LEVEL_NEW_GAME = 16,    // requested by the title: the first level, after the opening video
	LEVEL_AFTER_BATTLE = 18 // requested at the end of battle: the video bgm\01.krn, then the map
};

enum PlayMode {
	PLAYMODE_NORMAL = 0,   // intro videos, then the title
	PLAYMODE_NO_TITLE = 1, // set when LEVELS\Title.stream is missing: the game starts at the first level
	PLAYMODE_DEMO = 2      // kiosk demo: a "PRESS START" screen, then beach; no level changes after that
};

struct LevelTableRow {
	const char *m_stream;     // under LEVELS\ (LEVEL_TITLE: "Title.stream")
	const char *m_loadScreen; // loading screen image (Images\<language>\...)
	float m_loadTime;         // how long the loading progress runs, in quarter seconds
	float m_progressX;        // where the loading progress is drawn, in 640 x 480 screen units
	float m_progressY;
};
ZOOSTERS_ASSERT_SIZE(LevelTableRow, 0x14);

extern LevelTableRow g_LevelTable[];  // LEVEL_COUNT rows
extern int g_currentLevel;
extern int g_nextLevel;  // GameLevel requested (by the scripts and the menus)
extern int g_playMode;  // PlayMode

// 1 when a new level is to be loaded (LevelLoad).
int LevelManager();
// Loads the requested level behind its loading screen; false when its stream file did not load.
bool LevelLoad();

#endif
