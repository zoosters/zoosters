// One input context: the key binding map plus the options that differ per screen (cursor shown, mouse recentred, mouse deltas
// accumulated into stick-like axes). The game keeps nine (g_ctlBuffers).
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_CTLBUF_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_CTLBUF_H

#include "framework/startup/win32/input/ctlmap.h"
#include <stddef.h>

void *MemAlloc(size_t size);

struct CtlBuf {
	CtlBuf();                                  // GLOBAL: 0x51c400 ??0CtlBuf@@QAE@XZ
	static void *operator new(size_t size) { return MemAlloc(size); }

	char m_unk00;                // +0x00: never used
	char m_showCursor;           // +0x01: cursor visible (Pad_SelectControlBuffer -> Cursor_Show)
	char m_centreCursor;         // +0x02: SetPlayerContext moves the mouse to the window centre
	char m_accumulateMouse;      // +0x03: GetMouseControl accumulates the mouse deltas (mouse look)
	float m_accumDecay;          // +0x04: applied to the accumulated position each time
	float m_accumLimit;          // +0x08: the accumulated position is clamped to +-this
	float m_accumX;              // +0x0c
	float m_accumY;              // +0x10
	float m_mouseGain;           // +0x14: scale of the new delta before it is added
	CtlMap m_map;                // +0x18
};
ZOOSTERS_ASSERT_SIZE(CtlBuf, 0xe2c);
ZOOSTERS_ASSERT_OFFSET(CtlBuf, m_accumulateMouse, 3);
ZOOSTERS_ASSERT_OFFSET(CtlBuf, m_map, 0x18);

#endif
