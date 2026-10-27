// Key binding table of one input context. Every action has six binding slots; a binding names a device kind and a code on it, and
// carries the value InputManager::UpdateBindings last read. Filled by Controls_LoadSettings.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_CTLMAP_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_CTLMAP_H

#include "zoosters/base.h"

// Pad kinds count the DirectInput pad slot: kind 3 = pad 0 ... kind 12 = pad 9.
enum CtlBindType {
	CTL_BIND_NONE = 0,
	CTL_BIND_KEYBOARD = 1,    // code = DirectInput keyboard scan code
	CTL_BIND_MOUSE = 2,       // code = InputManager::GetMouseControl code
	CTL_BIND_PAD_FIRST = 3,   // code = InputManager::GetPadControl code
	CTL_BIND_PAD_LAST = 12
};

// Rows of the control name table g_ctlBindings, and the pad buttons the XInput emulation reads: analog buttons 4..11 in XDK order
// (A B X Y black white left trigger right trigger) and the digital button actions.
enum CtlAction {
	CTL_ACTION_JUMP = 4,               // = pad button A
	CTL_ACTION_ACTION = 5,             // B
	CTL_ACTION_FIRE = 6,               // X
	CTL_ACTION_ALTERNATE_ATTACK = 7,   // Y
	CTL_ACTION_TAKE_SCREENSHOT = 8,    // black
	CTL_ACTION_WHITE = 9,              // white (no row in g_ctlBindings)
	CTL_ACTION_RESET_CAMERA = 0xa,     // left trigger
	CTL_ACTION_SHOW_UI = 0xb,          // right trigger
	CTL_ACTION_PAUSE = 0xc,            // start
	CTL_ACTION_PAD_BACK = 0xd,
	CTL_ACTION_PAD_LEFT_THUMB = 0xe,
	CTL_ACTION_PAD_RIGHT_THUMB = 0xf,
	CTL_ACTION_FORWARD = 0x10,         // left stick y, up
	CTL_ACTION_BACKWARD = 0x11,        // left stick y, down
	CTL_ACTION_TURN_LEFT = 0x12,       // left stick x, left
	CTL_ACTION_TURN_RIGHT = 0x13,      // left stick x, right
	CTL_ACTION_CAMERA_UP = 0x14,       // right stick y, up
	CTL_ACTION_CAMERA_DOWN = 0x15,     // right stick y, down
	CTL_ACTION_CAMERA_LEFT = 0x16,     // right stick x, left
	CTL_ACTION_CAMERA_RIGHT = 0x17,    // right stick x, right
	CTL_ACTION_DPAD_UP = 0x18,
	CTL_ACTION_DPAD_DOWN = 0x19,
	CTL_ACTION_DPAD_LEFT = 0x1a,
	CTL_ACTION_DPAD_RIGHT = 0x1b
};

enum {
	CTL_MAX_ACTIONS = 50,     // actions per map (FindPressedInput scans 0x32)
	CTL_SLOTS = 6             // binding slots per action
};

struct CtlMapEntry {
	int m_type;               // CtlBindType; 0 = slot unused
	int m_code;
	float m_value;            // 0..1 for buttons, -1..1 for axes; refreshed by UpdateBindings
};
ZOOSTERS_ASSERT_SIZE(CtlMapEntry, 12);

struct CtlMap {
	unsigned m_count;         // highest bound action + 1
	CtlMapEntry m_entries[CTL_MAX_ACTIONS][CTL_SLOTS];

	CtlMap *Init();
	void Clear();                                          // GLOBAL: 0x51a630 ?Clear@CtlMap@@QAEXXZ
	CtlMap *Copy(CtlMap *src);                             // GLOBAL: 0x51a550 ?Copy@CtlMap@@QAEXPAU1@@Z
	void Unbind(int type, int code);                       // GLOBAL: 0x51a5e0 ?Unbind@CtlMap@@QAEXHH@Z
	void ClearSlot(int action, int slot);                  // GLOBAL: 0x51a5b0 ?ClearSlot@CtlMap@@QAEXHH@Z
	float GetValue(int action);                            // GLOBAL: 0x51a650 ?GetValue@CtlMap@@QAEMH@Z
	void GetBinding(int action, int slot, int *type, int *code);   // GLOBAL: 0x51a570 ?GetBinding@CtlMap@@QAEXHHPAH0@Z
	void Bind(int action, int slot, int type, int code);   // GLOBAL: 0x51a790 ?Bind@CtlMap@@QAEXHHHH@Z
};
ZOOSTERS_ASSERT_SIZE(CtlMap, 0xe14);
ZOOSTERS_ASSERT_OFFSET(CtlMap, m_entries, 4);

#endif
