// ControlsManager::Poll: opens new ports, reads every open pad, merges one-shot presses injected by other code, watches pad activity.
#include "script/script.h"
#include "script/referenceset.h"
#include "script/ops_message.h"
#include "framework/startup/win32/input/controlsmanager.h"
#include "framework/startup/win32/input/inputmanager.h"
#include "framework/startup/win32/input/xinput.h"
#include "modules/tfb/sprites/gamesprite.h"

extern unsigned char g_wndRequestFlag;      // GLOBAL: 0x62a5b1 g_wndRequestFlag
extern int g_wndRequestA;               // GLOBAL: 0x62a5b4 g_wndRequestA
extern int g_wndRequestB;                 // GLOBAL: 0x62a5b8 g_wndRequestB
extern unsigned char g_padStickUp;      // GLOBAL: 0x62a5bc g_padStickUp
extern unsigned char g_padStickDown;    // GLOBAL: 0x62a5bd g_padStickDown
extern unsigned char g_padStickLeft;    // GLOBAL: 0x62a5be g_padStickLeft
extern unsigned char g_padStickRight;   // GLOBAL: 0x62a5bf g_padStickRight
extern unsigned char g_cfgQuitOnInput;       // GLOBAL: 0x62a5c1 g_cfgQuitOnInput
extern unsigned char g_cfgKeyReqA;       // GLOBAL: 0x62a5c2 g_cfgKeyReqA
extern unsigned char g_quitRequested;     // GLOBAL: 0x62f140 g_quitRequested
extern unsigned char g_padHeld;         // GLOBAL: 0x617bf4 g_padHeld
extern unsigned char g_shotRequest;      // GLOBAL: 0x62a490 g_shotRequest

struct SkipTargetView {
	char m_pad[0xf6];
	char m_flag;
};
// 0x61f668 is g_subsetTable[2], written by RegisterSubset under that name only.
#include "script/ops_set.h"
#define g_skipA ((Script::ReferenceSet *)Script::g_subsetTable[2])
extern Script::UserReference *g_userRefSlots[];   // GLOBAL: 0x61f6c4 g_userRefSlots
#ifdef ZOOSTERS_OPTIONS
extern "C" void port_options_title_open(void);    // port/options/pages/pause_options.cpp
#endif

// Throw-away owner for GetInt.
// GLOBAL: 0x5ca3ec SkipOwnerTemp::vftable
struct SkipOwnerTemp : Script::ScriptReference {
	SkipOwnerTemp() {}
	~SkipOwnerTemp() {}
};

// 0x405b10
void ControlsManager::Poll()
{
	unsigned removals;
	unsigned insertions;
	XGetDeviceChanges(0, &insertions, &removals);
	for (unsigned i = 0; i < 4; i++) {
		g_pads[i].m_justInserted = 0;
		if (insertions & (1 << i)) {
			g_pads[i].m_device = XInputOpen(0, i, 0, 0);
			XInputGetCapabilities(g_pads[i].m_device, &g_pads[i].m_caps);
			g_pads[i].m_justInserted = 1;
		}
		if (g_pads[i].m_device) {
			XInputGetState(g_pads[i].m_device, &g_pads[i]);
			// one-shot presses from the keyboard pad keys and the menu stick presses
			if (g_wndRequestFlag && g_wndRequestB == i) {
				int value = g_wndRequestA;
				if (value == XINPUT_GAMEPAD_START)
					g_pads[i].m_buttons = (unsigned short)value;
				else
					g_pads[i].m_analogButtons[value] = 0xff;
				g_wndRequestFlag = 0;
			}
			if (g_padStickDown) {
				g_pads[i].m_thumbLY = -32768;
				g_padStickDown = 0;
			}
			if (g_padStickUp) {
				g_pads[i].m_thumbLY = 0x7fff;
				g_padStickUp = 0;
			}
			if (g_padStickLeft) {
				g_pads[i].m_thumbLX = -32768;
				g_padStickLeft = 0;
			}
			if (g_padStickRight) {
				g_pads[i].m_thumbLX = 0x7fff;
				g_padStickRight = 0;
			}
			if (g_cfgQuitOnInput) {
				if (g_pads[i].m_analogButtons[0] || g_pads[i].m_analogButtons[1] || g_pads[i].m_analogButtons[2]
					|| g_pads[i].m_analogButtons[3])
					g_quitRequested = 1;
			}
			if (g_cfgKeyReqA) {
				if (g_pads[i].m_analogButtons[0] || g_pads[i].m_analogButtons[1] || g_pads[i].m_analogButtons[2]
					|| g_pads[i].m_analogButtons[3] || g_pads[i].m_analogButtons[4] || g_pads[i].m_analogButtons[5]
					|| g_pads[i].m_analogButtons[6] || g_pads[i].m_analogButtons[7] || g_pads[i].m_caps.m_data[4])
					g_pads[i].m_buttons = XINPUT_GAMEPAD_START;
			}
			// cut-scene skip: button A counts as activity and is swallowed when the skip set's first member allows it
			SkipOwnerTemp owner;
			if (g_skipA && g_userRefSlots[0] && g_pads[i].m_analogButtons[0] == 0xff) {
#ifdef ZOOSTERS_OPTIONS
				int sel = g_userRefSlots[0]->GetInt(&owner);   // 3 = Options, 4 = Quit (ReferenceSet::Insert spawns both)
				if (sel == 3 || sel == 4) {
#else
				if (g_userRefSlots[0]->GetInt(&owner) == 3) {
#endif
					Script::SetNode *node = g_skipA->m_head;
					if (node) {
						SkipTargetView *target = (SkipTargetView *)node->m_object;
						if (target) {
							// +0xf6 of the 32-bit layout is GameSprite::m_visible; on x64 the raw offset read another byte and swallowed button A
							if (((RWS::TFB::GameSprite *)node->m_object)->m_visible) {
#ifdef ZOOSTERS_OPTIONS
								if (sel == 3) port_options_title_open();
								else
#endif
								g_quitRequested = 1;
								g_pads[i].m_analogButtons[0] = 0;
							}
						}
					}
				}
			}
			// the screenshot action: held, newly pressed
			CtlMap *map = &g_inputManager->m_contexts[g_pads[i].m_device - 1]->m_map;   // m_device = port + 1
			if (0.0f != map->GetValue(CTL_ACTION_TAKE_SCREENSHOT)) {
				if (!g_padHeld)
					g_shotRequest = 1;
				g_padHeld = 1;
			} else {
				g_padHeld = 0;
			}
		}
	}
}
