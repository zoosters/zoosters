// Controls_LoadSettings, plus the small helpers of the controller module that share its translation unit in the game (CtlIsBusy, SendMsgId).
#define ZOOSTERS_RWS_SENDMSG

#include "framework/startup/win32/input/controls.h"
#include "framework/startup/win32/appstate.h"
#include "framework/startup/win32/input/controlsmanager.h"
#include "framework/startup/win32/input/inputmanager.h"
#include "framework/startup/win32/input/xinput.h"
#include "framework/startup/win32/cfgobj.h"
#include "zoosters/os.h"
#include <stdio.h>
#include <string.h>

static inline void Sleep(unsigned long ms) { zoosters::os::SleepMs((unsigned)ms); }

extern CtlBuf *g_ctlBuffers[9];            // GLOBAL: 0x617ba0 g_ctlBuffers
extern CtlBinding g_ctlBindings[16];       // GLOBAL: 0x602448 g_ctlBindings
extern unsigned char g_ctlLoaded;          // GLOBAL: 0x617be8 g_ctlLoaded
// GLOBAL: 0x617bcc g_camDirectTurn (Settings\Display\MouseLook)
extern float g_mouseSens;                  // GLOBAL: 0x617bc8 g_mouseSens
extern int g_ctlSelB;                // GLOBAL: 0x617b9c g_ctlSelB
extern void *g_mainWnd;                    // GLOBAL: 0x62f138 g_mainWnd
extern int g_ctlCounter;                   // GLOBAL: 0x617bfc g_ctlCounter
extern unsigned char g_audioMixerFlag;     // GLOBAL: 0x617bf8 g_audioMixerFlag
// GLOBAL: 0x617c04 RWS::ERROR_NOT_A_VALID_MESSAGE_ID

// DirectInput scan codes used by the default bindings.
enum {
	DIK_ESCAPE = 0x01,
	DIK_Q = 0x10,
	DIK_E = 0x12,
	DIK_RETURN = 0x1c,
	DIK_A = 0x1e,
	DIK_S = 0x1f,
	DIK_D = 0x20,
	DIK_W = 0x11,
	DIK_SPACE = 0x39,
	DIK_F11 = 0x57,
	DIK_NUMPADENTER = 0x9c,
	DIK_LEFT = 0xcb,
	DIK_RIGHT = 0xcd,
	DIK_UP = 0xc8,
	DIK_DOWN = 0xd0
};

enum {
	CTL_CONTEXTS = 9,              // 0 = the game, 1 = the menus (cursor shown), 2..8 other screens
	CTL_PAD_PORTS = 4,
	CTL_PAD_PROBE_TRIES = 5,
	CTL_PAD_PROBE_WAIT_MS = 100,
	CTL_RUMBLE_CAPABLE = 1000      // m_rumbleLeftCaps / m_rumbleRightCaps of a pad with force feedback
};

// 0x404050
void CreateInputManager()
{
	g_inputManager = new InputManager;
}

// The first pad binding type in action and slot order; 0 when there is none.
// 0x4040c0
int FindPressedInput(CtlMap *map)
{
	int type;
	for (int action = 0; action < CTL_MAX_ACTIONS; action++) {
		for (int slot = 0; slot < CTL_SLOTS; slot++) {
			map->GetBinding(action, slot, &type, 0);
			if (type >= CTL_BIND_PAD_FIRST && type <= CTL_BIND_PAD_LAST)
				return type;
		}
	}
	return 0;
}

// "Settings\Controls\Gamepads\PadID<n>"; all zero = none.
static void LoadPadGuids()
{
	char key[260];
	for (int slot = 0; slot < INPUT_MAX_PADS; slot++) {
		char guid[0x25];
		sprintf(key, "Settings\\Controls\\Gamepads\\PadID%d", slot + 1);
		g_audioDevice->GetString(key, "00000000-0000-0000-0000-000000000000", guid, 0x25);
		g_inputManager->SetPadGuid(slot, guid);
	}
}

// Slots 1..3 of each action of g_ctlBindings come from "Settings\Controls\Player1\<name><1..3>" or the table default; a "fixed" default
// is never read. Slot 3 always reads the registry.
static void LoadPlayerBindings(CtlMap &map)
{
	static const int kRowActions[16] = {
		CTL_ACTION_FORWARD, CTL_ACTION_BACKWARD, CTL_ACTION_TURN_LEFT, CTL_ACTION_TURN_RIGHT, CTL_ACTION_JUMP, CTL_ACTION_ACTION,
		CTL_ACTION_FIRE, CTL_ACTION_ALTERNATE_ATTACK, CTL_ACTION_SHOW_UI, CTL_ACTION_CAMERA_UP, CTL_ACTION_CAMERA_DOWN,
		CTL_ACTION_CAMERA_LEFT, CTL_ACTION_CAMERA_RIGHT, CTL_ACTION_PAUSE, CTL_ACTION_RESET_CAMERA, CTL_ACTION_TAKE_SCREENSHOT
	};
	char key[260];
	for (int row = 0; row < 16; row++) {
		CtlBinding binding = g_ctlBindings[row];
		sprintf(key, "Settings\\Controls\\Player%d\\%s1", 1, binding.m_name);
		unsigned value1 = !binding.m_fixed1 ? g_audioDevice->GetInt(key, binding.m_def1) : binding.m_def1;
		sprintf(key, "Settings\\Controls\\Player%d\\%s2", 1, binding.m_name);
		unsigned value2 = !binding.m_fixed2 ? g_audioDevice->GetInt(key, binding.m_def2) : binding.m_def2;
		sprintf(key, "Settings\\Controls\\Player%d\\%s3", 1, binding.m_name);
		// port: the default of the third slot is the launcher's pad preset 0 (Logitech RumblePad 2 / Dual Action layout, as in
		// port/provisional/gamepad_sdl.cpp): left stick moves, right stick camera, south button jumps, Start pauses. Without the
		// launcher the original leaves a pad doing nothing. A saved value still wins.
		static const unsigned kPortPadPreset[16] = { 0x30004, 0x30003, 0x30002, 0x30001, 0x30016, 0x30017, 0x30015, 0x30018, 0x3001a, 0x3000c,
		                                             0x3000b, 0x30006, 0x30005, 0x3001d, 0x30019, 0x3ffff };
		unsigned value3 = g_audioDevice->GetInt(key, kPortPadPreset[row]);
		// device kind << 16 | code
		map.Bind(kRowActions[row], 0, value1 >> 16, (short)value1);
		map.Bind(kRowActions[row], 1, value2 >> 16, (short)value2);
		map.Bind(kRowActions[row], 2, value3 >> 16, (short)value3);
	}
}

// Context 3: W S A D and the arrow keys on pad buttons (Y A X B and the second diamond), the first bound pad on the face buttons too.
static void BuildContext3(CtlMap &map0, CtlMap &map3)
{
	map3.Copy(&map0);
	map3.Unbind(CTL_BIND_KEYBOARD, DIK_W);
	map3.Unbind(CTL_BIND_KEYBOARD, DIK_S);
	map3.Unbind(CTL_BIND_KEYBOARD, DIK_A);
	map3.Unbind(CTL_BIND_KEYBOARD, DIK_D);
	map3.Bind(CTL_ACTION_ALTERNATE_ATTACK, 3, CTL_BIND_KEYBOARD, DIK_W);
	map3.Bind(CTL_ACTION_JUMP, 3, CTL_BIND_KEYBOARD, DIK_S);
	map3.Bind(CTL_ACTION_FIRE, 3, CTL_BIND_KEYBOARD, DIK_A);
	map3.Bind(CTL_ACTION_ACTION, 3, CTL_BIND_KEYBOARD, DIK_D);
	int padKind = FindPressedInput(&map3); 
	if (padKind >= CTL_BIND_PAD_FIRST && padKind <= CTL_BIND_PAD_LAST) {
		map3.Unbind(padKind, 1);
		map3.Unbind(padKind, 2);
		map3.Unbind(padKind, 3);
		map3.Unbind(padKind, 4);
		map3.Bind(CTL_ACTION_ALTERNATE_ATTACK, 4, padKind, 4);
		map3.Bind(CTL_ACTION_JUMP, 4, padKind, 3);
		map3.Bind(CTL_ACTION_FIRE, 4, padKind, 2);
		map3.Bind(CTL_ACTION_ACTION, 4, padKind, 1);
	}
	map3.Unbind(CTL_BIND_KEYBOARD, DIK_UP);
	map3.Unbind(CTL_BIND_KEYBOARD, DIK_DOWN);
	map3.Unbind(CTL_BIND_KEYBOARD, DIK_LEFT);
	map3.Unbind(CTL_BIND_KEYBOARD, DIK_RIGHT);
	map3.Bind(CTL_ACTION_PAD_BACK, 3, CTL_BIND_KEYBOARD, DIK_UP);
	map3.Bind(CTL_ACTION_SHOW_UI, 3, CTL_BIND_KEYBOARD, DIK_DOWN);
	map3.Bind(CTL_ACTION_WHITE, 3, CTL_BIND_KEYBOARD, DIK_LEFT);
	map3.Bind(CTL_ACTION_RESET_CAMERA, 3, CTL_BIND_KEYBOARD, DIK_RIGHT);
}

// Context 1, the menus: arrows, Enter and Space (A), E (B), Q (X), Esc (Y) and F11; the action numbers act as pad buttons.
static void BuildMenuContext(CtlMap &map0, CtlMap &map1)
{
	map1.Copy(&map0);
	map1.Unbind(CTL_BIND_KEYBOARD, DIK_UP);
	map1.Unbind(CTL_BIND_KEYBOARD, DIK_DOWN);
	map1.Unbind(CTL_BIND_KEYBOARD, DIK_LEFT);
	map1.Unbind(CTL_BIND_KEYBOARD, DIK_RIGHT);
	map1.Unbind(CTL_BIND_KEYBOARD, DIK_RETURN);
	map1.Unbind(CTL_BIND_KEYBOARD, DIK_SPACE);
	map1.Unbind(CTL_BIND_KEYBOARD, DIK_E);
	map1.Unbind(CTL_BIND_KEYBOARD, DIK_Q);
	map1.Unbind(CTL_BIND_KEYBOARD, DIK_ESCAPE);
	map1.Unbind(CTL_BIND_KEYBOARD, DIK_F11);
	map1.Unbind(CTL_BIND_MOUSE, MOUSE_Y_POSITIVE);
	map1.Unbind(CTL_BIND_MOUSE, MOUSE_Y_NEGATIVE);
	map1.Unbind(CTL_BIND_MOUSE, MOUSE_X_POSITIVE);
	map1.Unbind(CTL_BIND_MOUSE, MOUSE_X_NEGATIVE);
	map1.Unbind(CTL_BIND_MOUSE, MOUSE_BUTTON_0);
	map1.Bind(CTL_ACTION_FORWARD, 3, CTL_BIND_KEYBOARD, DIK_UP);
	map1.Bind(CTL_ACTION_BACKWARD, 3, CTL_BIND_KEYBOARD, DIK_DOWN);
	map1.Bind(CTL_ACTION_TURN_LEFT, 3, CTL_BIND_KEYBOARD, DIK_LEFT);
	map1.Bind(CTL_ACTION_TURN_RIGHT, 3, CTL_BIND_KEYBOARD, DIK_RIGHT);
	map1.Bind(CTL_ACTION_JUMP, 3, CTL_BIND_KEYBOARD, DIK_RETURN);
	map1.Bind(CTL_ACTION_JUMP, 4, CTL_BIND_KEYBOARD, DIK_SPACE);
	map1.Bind(CTL_ACTION_ACTION, 3, CTL_BIND_KEYBOARD, DIK_E);
	map1.Bind(CTL_ACTION_FIRE, 3, CTL_BIND_KEYBOARD, DIK_Q);
	map1.Bind(CTL_ACTION_ALTERNATE_ATTACK, 3, CTL_BIND_KEYBOARD, DIK_ESCAPE);
	map1.Bind(CTL_ACTION_TAKE_SCREENSHOT, 3, CTL_BIND_KEYBOARD, DIK_F11);
}

// Context 2: Enter and keypad Enter on button A, Esc on the pause action.
static void BuildContext2(CtlMap &map0, CtlMap &map2)
{
	map2.Copy(&map0);
	map2.Unbind(CTL_BIND_KEYBOARD, DIK_RETURN);
	map2.Unbind(CTL_BIND_KEYBOARD, DIK_NUMPADENTER);
	map2.Bind(CTL_ACTION_JUMP, 4, CTL_BIND_KEYBOARD, DIK_RETURN);
	map2.Bind(CTL_ACTION_JUMP, 5, CTL_BIND_KEYBOARD, DIK_NUMPADENTER);
	map2.ClearSlot(CTL_ACTION_ALTERNATE_ATTACK, 3);
	map2.Bind(CTL_ACTION_PAUSE, 3, CTL_BIND_KEYBOARD, DIK_ESCAPE);
}

// Context 7: like context 2, with Space on button X.
static void BuildContext7(CtlMap &map0, CtlMap &map7)
{
	map7.Copy(&map0);
	map7.Unbind(CTL_BIND_KEYBOARD, DIK_RETURN);
	map7.Unbind(CTL_BIND_KEYBOARD, DIK_NUMPADENTER);
	map7.Unbind(CTL_BIND_KEYBOARD, DIK_SPACE);
	map7.Bind(CTL_ACTION_JUMP, 4, CTL_BIND_KEYBOARD, DIK_RETURN);
	map7.Bind(CTL_ACTION_JUMP, 5, CTL_BIND_KEYBOARD, DIK_NUMPADENTER);
	map7.Bind(CTL_ACTION_FIRE, 4, CTL_BIND_KEYBOARD, DIK_SPACE);
}

// The mouse axes also drive the camera (context 0) or the right stick (contexts 4 to 6). Settings\Display\InvMouse swaps the vertical direction.
static void ApplyMouseLook(CtlMap &map0, CtlMap &map4, CtlMap &map5, CtlMap &map6)
{
	map0.Bind(CTL_ACTION_CAMERA_LEFT, 3, CTL_BIND_MOUSE, MOUSE_X_POSITIVE);
	map0.Bind(CTL_ACTION_CAMERA_RIGHT, 3, CTL_BIND_MOUSE, MOUSE_X_NEGATIVE);
	if (g_audioDevice->GetInt("Settings\\Display\\InvMouse", 0)) {
		map0.Bind(CTL_ACTION_CAMERA_UP, 3, CTL_BIND_MOUSE, MOUSE_Y_NEGATIVE);
		map0.Bind(CTL_ACTION_CAMERA_DOWN, 3, CTL_BIND_MOUSE, MOUSE_Y_POSITIVE);
	} else {
		map0.Bind(CTL_ACTION_CAMERA_UP, 3, CTL_BIND_MOUSE, MOUSE_Y_POSITIVE);
		map0.Bind(CTL_ACTION_CAMERA_DOWN, 3, CTL_BIND_MOUSE, MOUSE_Y_NEGATIVE);
	}
	map6.Bind(CTL_ACTION_TURN_RIGHT, 4, CTL_BIND_MOUSE, MOUSE_X_POSITIVE);
	map6.Bind(CTL_ACTION_TURN_LEFT, 4, CTL_BIND_MOUSE, MOUSE_X_NEGATIVE);
	if (g_audioDevice->GetInt("Settings\\Display\\InvMouse", 0)) {
		map6.Bind(CTL_ACTION_BACKWARD, 4, CTL_BIND_MOUSE, MOUSE_Y_POSITIVE);
		map6.Bind(CTL_ACTION_FORWARD, 4, CTL_BIND_MOUSE, MOUSE_Y_NEGATIVE);
	} else {
		map6.Bind(CTL_ACTION_BACKWARD, 4, CTL_BIND_MOUSE, MOUSE_Y_NEGATIVE);
		map6.Bind(CTL_ACTION_FORWARD, 4, CTL_BIND_MOUSE, MOUSE_Y_POSITIVE);
	}
	map4.Unbind(CTL_BIND_MOUSE, MOUSE_X_POSITIVE);
	map4.Unbind(CTL_BIND_MOUSE, MOUSE_X_NEGATIVE);
	map4.Unbind(CTL_BIND_MOUSE, MOUSE_Y_NEGATIVE);
	map4.Unbind(CTL_BIND_MOUSE, MOUSE_Y_POSITIVE);
	map4.Bind(CTL_ACTION_TURN_RIGHT, 3, CTL_BIND_MOUSE, MOUSE_X_POSITIVE);
	map4.Bind(CTL_ACTION_TURN_LEFT, 3, CTL_BIND_MOUSE, MOUSE_X_NEGATIVE);
	map4.Bind(CTL_ACTION_FORWARD, 3, CTL_BIND_MOUSE, MOUSE_Y_NEGATIVE);
	map4.Bind(CTL_ACTION_BACKWARD, 3, CTL_BIND_MOUSE, MOUSE_Y_POSITIVE);
	map4.Unbind(CTL_BIND_KEYBOARD, DIK_RIGHT);
	map4.Unbind(CTL_BIND_KEYBOARD, DIK_LEFT);
	map4.Unbind(CTL_BIND_KEYBOARD, DIK_UP);
	map4.Unbind(CTL_BIND_KEYBOARD, DIK_DOWN);
	map4.Bind(CTL_ACTION_CAMERA_RIGHT, 3, CTL_BIND_KEYBOARD, DIK_RIGHT);
	map4.Bind(CTL_ACTION_CAMERA_LEFT, 3, CTL_BIND_KEYBOARD, DIK_LEFT);
	map4.Bind(CTL_ACTION_CAMERA_UP, 3, CTL_BIND_KEYBOARD, DIK_UP);
	map4.Bind(CTL_ACTION_CAMERA_DOWN, 3, CTL_BIND_KEYBOARD, DIK_DOWN);
	map5.Unbind(CTL_BIND_MOUSE, MOUSE_X_POSITIVE);
	map5.Unbind(CTL_BIND_MOUSE, MOUSE_X_NEGATIVE);
	map5.Bind(CTL_ACTION_CAMERA_RIGHT, 3, CTL_BIND_MOUSE, MOUSE_X_POSITIVE);
	map5.Bind(CTL_ACTION_CAMERA_LEFT, 3, CTL_BIND_MOUSE, MOUSE_X_NEGATIVE);
}

// 0x404110
int Controls_LoadSettings()
{
	if (g_ctlLoaded)
		return 0;
	CreateInputManager();
	g_inputManager->SetSmoothMouse(1);
	g_inputManager->m_numContexts = 1;
	for (int i = 0; i < CTL_CONTEXTS; i++)
		g_inputManager->CreateContext(&g_ctlBuffers[i]);
	CtlMap &map0 = g_ctlBuffers[0]->m_map;
	CtlMap &map1 = g_ctlBuffers[1]->m_map;
	CtlMap &map2 = g_ctlBuffers[2]->m_map;
	CtlMap &map3 = g_ctlBuffers[3]->m_map;
	CtlMap &map4 = g_ctlBuffers[4]->m_map;
	CtlMap &map5 = g_ctlBuffers[5]->m_map;
	CtlMap &map6 = g_ctlBuffers[6]->m_map;
	CtlMap &map7 = g_ctlBuffers[7]->m_map;
	CtlMap &map8 = g_ctlBuffers[8]->m_map;
	map0.Clear();
	LoadPadGuids();
	LoadPlayerBindings(map0);
	// contexts 4, 5, 6 and 8 start as copies of the game bindings
	map4.Copy(&map0);
	map5.Copy(&map0);
	map6.Copy(&map0);
	map8.Copy(&map0);
	BuildContext3(map0, map3);
	BuildMenuContext(map0, map1);
	BuildContext2(map0, map2);
	BuildContext7(map0, map7);
	// only the menu context shows the cursor; only context 3 keeps the constructor default (recentre the mouse)
	g_ctlBuffers[0]->m_showCursor = 0;
	g_ctlBuffers[1]->m_showCursor = 1;
	g_ctlBuffers[2]->m_showCursor = 0;
	g_ctlBuffers[7]->m_showCursor = 0;
	g_ctlBuffers[6]->m_showCursor = 0;
	g_ctlBuffers[8]->m_showCursor = 0;
	g_ctlBuffers[4]->m_showCursor = 0;
	g_ctlBuffers[5]->m_showCursor = 0;
	g_ctlBuffers[0]->m_centreCursor = 0;
	g_ctlBuffers[1]->m_centreCursor = 0;
	g_ctlBuffers[2]->m_centreCursor = 0;
	g_ctlBuffers[7]->m_centreCursor = 0;
	g_ctlBuffers[6]->m_centreCursor = 0;
	g_ctlBuffers[8]->m_centreCursor = 0;
	g_ctlBuffers[4]->m_centreCursor = 0;
	g_ctlBuffers[5]->m_centreCursor = 0;

#ifdef ZOOSTERS_MOUSE_LOOK   // qol: mouse look unless the player turned it off
	g_camDirectTurn = g_audioDevice->GetInt("Settings\\Display\\MouseLook", 1) != 0;
#else
	g_camDirectTurn = g_audioDevice->GetInt("Settings\\Display\\MouseLook", 0) != 0;
#endif
	if (g_camDirectTurn)
		ApplyMouseLook(map0, map4, map5, map6);
	g_inputManager->Init(g_mainWnd);
	int sensitivity = g_audioDevice->GetInt("Settings\\Display\\Sensitivity", 0x32);
	g_inputManager->m_mouseSens = g_mouseSens = (float)sensitivity * 0.0033333334f + 0.01f;
	g_ctlSelB = 0;
	g_inputManager->SetPlayerContext(0, g_ctlBuffers[0]);
	XInitDevices(0, 0);
	// wait up to half a second for a pad
	int mask = 0;
	for (unsigned tries = 0; tries < CTL_PAD_PROBE_TRIES; tries++) {
		Sleep(CTL_PAD_PROBE_WAIT_MS);
		mask = XGetDevices(0);
		if (mask)
			break;
	}
	for (unsigned port = 0; port < CTL_PAD_PORTS; port++) {
		memset(&g_pads[port], 0, sizeof(PadState));
		if (mask & (1 << port)) {
			int handle = XInputOpen(0, port, 0, 0);
			g_pads[port].m_device = handle;
			XInputGetCapabilities(handle, &g_pads[port].m_caps);
			if (g_inputManager->HasForceFeedback(port)) {
				g_pads[port].m_rumbleRightCaps = CTL_RUMBLE_CAPABLE;
				g_pads[port].m_rumbleLeftCaps = CTL_RUMBLE_CAPABLE;
			}
		}
	}
	g_ctlLoaded = 1;
	return 0;
}

// True while the audio mixer is starting a stream or a stopped slot waits to be freed.
// 0x4064b0
int CtlIsBusy()
{
	if (!g_audioMixerFlag && !g_ctlCounter)
		return 0;
	return 1;
}

namespace RWS {
// Without data.
// 0x406470
int SendMsgId(CEventId *id)
{
	if (*id != ERROR_NOT_A_VALID_MESSAGE_ID && (*id)->Erm.p_next) {
		CMsg msg(*id);
		return _SendMsg(msg);
	}
	return 0;
}
}
