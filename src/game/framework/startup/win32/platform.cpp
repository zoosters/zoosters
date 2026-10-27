// CFLAGS: /GS
#include <string.h>
#include "zoosters/base.h"
#include "framework/core/memoryhandler.h"
#include "framework/startup/win32/appstate.h"
#include "framework/startup/win32/platform.h"
#include "zoosters/os.h"

enum {
	SPI_GETSCREENSAVETIMEOUT = 0x0e,
	SPI_GETSTICKYKEYS = 0x3a,
	SPI_SETSTICKYKEYS = 0x3b,
	SPI_GETMOUSESPEED = 0x70
};
enum {
	SKF_AVAILABLE = 0x02,        // installed
	SKF_HOTKEYACTIVE = 0x04      // five Shift presses switch sticky keys on
};

struct StickyKeys {   // STICKYKEYS
	unsigned cbSize;
	unsigned dwFlags;
};
ZOOSTERS_ASSERT_SIZE(StickyKeys, 8);

// Entry of a table of 255 lists freed by Platform_Shutdown. Nothing in the game fills it.
struct PlatformNode {
	int m_unk0;
	void *m_data;
	PlatformNode *m_next;
};

extern void *g_hInstance;                    // GLOBAL: 0x62f13c g_hInstance
extern StickyKeys g_stickyOff;               // GLOBAL: 0x62ed28 g_stickyOff (the settings while the game runs: hotkey off)
extern StickyKeys g_stickySaved;             // GLOBAL: 0x62f12c g_stickySaved (the user's settings, restored on focus loss and exit)
extern PlatformNode *g_platformLists[255];   // GLOBAL: 0x62ed30 g_platformLists
extern int g_mouseSpeed;                     // GLOBAL: 0x62f154 g_mouseSpeed (SPI_GETMOUSESPEED at start-up; never restored)
// GLOBAL: 0x62f14c g_screenSaverTimeout (declared in platform.h)

// 0x51a070
void Platform_Init()
{
	memset(g_platformLists, 0, sizeof(g_platformLists));
	zoosters::os::StickyKeysInit();
	g_screenSaverTimeout = zoosters::os::ScreenSaverTimeout();
}

// 0x51a1a0
void Platform_Shutdown()
{
	for (int i = 0; i < 255; i++) {
		PlatformNode *node = g_platformLists[i];
		while (node) {
			PlatformNode *cur = node;
			node = node->m_next;
			operator delete(cur->m_data);
			operator delete(cur);
		}
	}
	zoosters::os::StickyKeysSet(false);
	if (g_mainWnd) {
		zoosters::os::DestroyWindow(g_mainWnd);
		g_mainWnd = 0;
	}
}

// 0x51a210
void App_SetInactive()
{
	g_frameReady = 0;
	zoosters::os::StickyKeysSet(false);
}

// 0x51a230
void App_SetActive()
{
	zoosters::os::StickyKeysSet(true);
	g_frameReady = 1;
}
