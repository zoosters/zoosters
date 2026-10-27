// The DirectInput layer: owns the keyboard, mouse and up to ten game controllers, polls them once per frame and turns their
// states into the values of the players' key binding maps (UpdateBindings). One instance, g_inputManager.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_INPUTMANAGER_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_INPUTMANAGER_H

#include "framework/startup/win32/input/ctlbuf.h"
#include "framework/startup/win32/input/dinput.h"

// CTL_BIND_PAD_* codes.
enum PadControl {
	PADCTL_AXIS_FIRST = 1,   // 1..16: positive / negative half of axis (code - 1) / 2 of the DIJOYSTATE2 (lX lY lZ lRx lRy lRz slider0 slider1)
	PADCTL_HAT_RIGHT = 17,   // 17..20: the four hat (POV) directions, 135 degrees each
	PADCTL_HAT_LEFT = 18,
	PADCTL_HAT_DOWN = 19,
	PADCTL_HAT_UP = 20,
	PADCTL_AXIS_LAST = 20,
	PADCTL_BUTTON_FIRST = 21,    // 21..49: rgbButtons[0..28]
	PADCTL_BUTTON_LAST = 49
};

// CTL_BIND_MOUSE codes.
enum MouseControl {
	MOUSE_WHEEL_FAST = 0,   // wheel moved more than one notch: +1 forward, -1 back
	MOUSE_WHEEL_UP = 1,
	MOUSE_WHEEL_DOWN = 2,
	MOUSE_AXIS_Y = 3,          // signed deltas (accumulated in mouse look contexts)
	MOUSE_AXIS_X = 4,
	MOUSE_Y_NEGATIVE = 5,      // magnitude of the negative / positive half of the axis
	MOUSE_Y_POSITIVE = 6,
	MOUSE_X_NEGATIVE = 7,
	MOUSE_X_POSITIVE = 8,
	MOUSE_BUTTON_0 = 9,
	MOUSE_BUTTON_1 = 10,
	MOUSE_BUTTON_2 = 11
};

enum {
	INPUT_MAX_PADS = 10,         // DirectInput pad slots
	INPUT_MAX_PLAYERS = 4        // player contexts
};

class InputManager {
public:
	InputManager();
	~InputManager();
	void SetSmoothMouse(char on);
	char CreateContext(CtlBuf **out);   // allocates a 0xe2c byte context
	void SetPadGuid(int index, const char *guid);
	char Init(void *hwnd);   // 0 without a window
	void ScanDevices(char adoptGuid);
	DIJOYSTATE2 *GetPadState(int index);
	void ReleaseDevices();
	float GetPadControl(int pad, int code);   // 0..1 / -1..1
	void UpdateBindings(CtlMap *map, int player);
	int GetPadAxisControl(const DIJOYSTATE2 *pad, int control);   // -1000..1000 or 0 / 1000
	float GetMouseControl(int code, int player);
	int OnEnumDevice(const DIDEVICEINSTANCEA *info);   // always returns 1
	int OnEnumObject(const DIDEVICEOBJECTINSTANCEA *object, IDirectInputDevice8A *device);
	bool HasForceFeedback(int index);
	void SetPadForce(int index, int strength0, int strength1);
	void Poll();                                 // poll all devices  // GLOBAL: 0x51b450 ?Poll@InputManager@@QAEXXZ
	void SetPlayerContext(int index, CtlBuf *context);
	static void *operator new(size_t size) { return MemAlloc(size); }

	char m_smoothMouse;   // +0x0: a zero mouse delta takes half the last one (Poll, g_mouseSmoothX/Y)
	char m_flushHeld;   // +0x1: input equal to the m_*Snap snapshots is hidden until it changes (SetPlayerContext)
	char m_scanAdoptGuid;   // +0x2: ScanDevices' argument; OnEnumDevice then copies the device GUID into the slot
	char m_pad03;
	IDirectInputDevice8A *m_keyboard;    // +0x4: GUID_SysKeyboard
	IDirectInputDevice8A *m_mouse;       // +0x8: GUID_SysMouse
	IDirectInputDevice8A *m_padDevices[INPUT_MAX_PADS];   // +0xc
	IDirectInputEffect *m_padEffects[INPUT_MAX_PADS];   // +0x34: GUID_ConstantForce effect per pad
	unsigned m_padFFAxes[INPUT_MAX_PADS];   // +0x5c: force-feedback actuator axes per pad (max 2)
	unsigned m_padFFResolution[INPUT_MAX_PADS][2];   // +0x84: dwFFForceResolution per pad and axis
	char m_padNames[INPUT_MAX_PADS][100];                 // +0xd4: tszInstanceName
	DIMOUSESTATE2 m_mouseState;          // +0x4bc
	DIMOUSESTATE2 m_mousePrev;           // +0x4d0
	char m_padPresent[INPUT_MAX_PADS];   // +0x4e4: set by OnEnumDevice, compacted by ScanDevices
	char m_pad4ee[2];
	DIJOYSTATE2 m_padState[INPUT_MAX_PADS];   // +0x4f0
	DIJOYSTATE2 m_padPrev[INPUT_MAX_PADS];    // +0xf90
	DIMOUSESTATE2 m_mouseSnap;   // +0x1a30: snapshots for m_flushHeld
	DIJOYSTATE2 m_padSnap[INPUT_MAX_PADS];    // +0x1a44
	unsigned char m_keysSnap[0x100];     // +0x24e4
	unsigned char m_keys[0x100];         // +0x25e4: keyboard state (DIK codes, bit 7 = down)
	unsigned char m_keysPrev[0x100];     // +0x26e4
	IDirectInput8A *m_dinput;            // +0x27e4
	DiGuid m_padGuids[INPUT_MAX_PADS];   // +0x27e8: "PadID%d" settings
	char m_initialised;                  // +0x2888
	char m_pad2889[3];
	void *m_hwnd;                        // +0x288c
	float m_mouseSens;   // +0x2890
	char m_pad2894[8];
	char m_keyNames[256][0x100];   // +0x289c: key name per scan code
	char m_pad1289c[0x12c];
	unsigned m_numContexts;   // +0x129c8: player contexts polled
	unsigned m_padPolledMask;   // +0x129cc: bit i = pad i delivered a state in this Poll
	CtlBuf *m_contexts[INPUT_MAX_PLAYERS];   // +0x129d0: the XInput handle (port + 1) indexes it
};
ZOOSTERS_ASSERT_SIZE(InputManager, 0x129e0);
ZOOSTERS_ASSERT_OFFSET(InputManager, m_padDevices, 0xc);
ZOOSTERS_ASSERT_OFFSET(InputManager, m_mouseState, 0x4bc);
ZOOSTERS_ASSERT_OFFSET(InputManager, m_padState, 0x4f0);
ZOOSTERS_ASSERT_OFFSET(InputManager, m_keys, 0x25e4);
ZOOSTERS_ASSERT_OFFSET(InputManager, m_dinput, 0x27e4);
ZOOSTERS_ASSERT_OFFSET(InputManager, m_keyNames, 0x289c);
ZOOSTERS_ASSERT_OFFSET(InputManager, m_numContexts, 0x129c8);
ZOOSTERS_ASSERT_OFFSET(InputManager, m_contexts, 0x129d0);

extern InputManager *g_inputManager;       // GLOBAL: 0x62f15c g_inputManager

#endif
