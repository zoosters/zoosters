// Level table accessors (levelmanager.h).
#include "tfb/level/levelmanager.h"

// The current level, with the title counted as the new-game request (LEVEL_NEW_GAME).
// 0x460c30
int LevelRemapTitle()
{
	int level = g_currentLevel;
	if (level == LEVEL_TITLE)
		level = LEVEL_NEW_GAME;
	return level;
}

// The stream file of a level (the first entry of its row).
// 0x460c40
const char *LevelTable_GetFirst(int index)
{
	return g_LevelTable[index].m_stream;
}
