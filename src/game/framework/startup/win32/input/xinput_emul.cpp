// The XDK pad API on top of the InputManager: a "pad" is a player's input context (handle = port + 1), its state built from the context's binding map.
// CFLAGS: /GS
#include "framework/startup/win32/input/xinput.h"
#include "framework/startup/win32/input/inputmanager.h"
#include <stdlib.h>
#include <string.h>

extern void *g_mainWnd;                    // GLOBAL: 0x62f138 g_mainWnd
extern unsigned g_padPrev;                 // GLOBAL: 0x62f170 g_padPrev

// The XDK arguments are unused.
// 0x51c440
void XInitDevices(int preallocCount, int preallocTypes)
{
	g_inputManager->Init(g_mainWnd);
}

// 0x51c460
int XInputOpen(int deviceType, int port, int slot, int pollingParams)
{
	if (deviceType == 0) {
		if ((unsigned)port < g_inputManager->m_numContexts)
			return port + 1;
	}
	return 0;
}

// 0x51c4f0
void XInputGetCapabilities(int device, XInputCapabilities *caps)
{
	if (g_inputManager->GetPadState(device - 1))
		caps->m_subType = 1;
}

// One bit per player input context.
// 0x51c510
int XGetDevices(int deviceType)
{
	int mask = 0;
	if (deviceType == 0) {
		unsigned count = g_inputManager->m_numContexts;
		if (count > 0)
			mask = 1;
		if (count > 1)
			mask |= 2;
		if (count > 2)
			mask |= 4;
		if (count > 3)
			mask |= 8;
	}
	return mask;
}

// The status is always set to 0 (done); callers skip the update while it is 0x3e5 (ERROR_IO_PENDING).
// 0x51c480
int XInputSetState(int device, int *feedback)
{
	int port = device - 1;
	if (g_inputManager->GetPadState(port)) {
		if (g_inputManager->HasForceFeedback(port)) {
			g_inputManager->SetPadForce(port, feedback[3], feedback[4]);
			*feedback = 0;
			return 0;
		}
		*feedback = 0;
		return 0;
	}
	*feedback = 0;
	return 0;
}

// 0x51c550
int XGetDeviceChanges(int deviceType, unsigned *insertions, unsigned *removals)
{
	unsigned previous = g_padPrev;
	unsigned current = g_inputManager->m_padPolledMask;
	*insertions = current & ~previous;
	*removals = previous & ~current;
	g_padPrev = current;
	return 0;
}

// The action with the larger magnitude wins; equal magnitudes give 0. scaleA / scaleB carry the sign.
static int StickAxis(CtlMap *map, int actionA, float scaleA, int actionB, float scaleB)
{
	int a = (int)(map->GetValue(actionA) * scaleA);
	int b = (int)(map->GetValue(actionB) * scaleB);
	if (abs(a) > abs(b))
		return a;
	if (abs(a) != abs(b))
		return b;
	return 0;
}

struct PadButtonAction {
	int m_action;
	unsigned short m_button;
};

// Polls, then builds the XINPUT_STATE from the context's binding map: stick axes +-32767, analog buttons 0..255, digital bits.
// 0x51c590
int XInputGetState(int device, PadState *out)
{
	static const PadButtonAction kButtons[] = {
		{ CTL_ACTION_PAUSE, XINPUT_GAMEPAD_START },
		{ CTL_ACTION_PAD_BACK, XINPUT_GAMEPAD_BACK },
		{ CTL_ACTION_PAD_LEFT_THUMB, XINPUT_GAMEPAD_LEFT_THUMB },
		{ CTL_ACTION_PAD_RIGHT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB },
		{ CTL_ACTION_DPAD_UP, XINPUT_GAMEPAD_DPAD_UP },
		{ CTL_ACTION_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_DOWN },
		{ CTL_ACTION_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_LEFT },
		{ CTL_ACTION_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT }
	};
	g_inputManager->Poll();
	CtlMap *map = &g_inputManager->m_contexts[device - 1]->m_map;   // device = port + 1
	memset(&out->m_buttons, 0, offsetof(PadState, m_caps) - offsetof(PadState, m_buttons));
	out->m_thumbLX = StickAxis(map, CTL_ACTION_TURN_LEFT, -32767.0f, CTL_ACTION_TURN_RIGHT, 32767.0f);
	out->m_thumbLY = StickAxis(map, CTL_ACTION_FORWARD, 32767.0f, CTL_ACTION_BACKWARD, -32767.0f);
	out->m_thumbRX = StickAxis(map, CTL_ACTION_CAMERA_LEFT, -32767.0f, CTL_ACTION_CAMERA_RIGHT, 32767.0f);
	out->m_thumbRY = StickAxis(map, CTL_ACTION_CAMERA_UP, 32767.0f, CTL_ACTION_CAMERA_DOWN, -32767.0f);
	for (int i = 0; i < 8; i++)
		out->m_analogButtons[i] = (unsigned char)(int)(map->GetValue(CTL_ACTION_JUMP + i) * 255.0f);
	for (int i = 0; i < 8; i++) {
		if (map->GetValue(kButtons[i].m_action) != 0.0f)
			out->m_buttons |= kButtons[i].m_button;
	}
	return 0;
}
