// InputManager part 1: construction, DirectInput set-up, pad GUIDs and force feedback. Polling and the binding values are in inputpoll.cpp.
// CFLAGS: /GS
#include "framework/startup/win32/input/inputmanager.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "zoosters/os.h"

static inline int GetClientRect(void *wnd, long *rect)
{
	int w = 0, h = 0;
	int ok = zoosters::os::ClientSize(wnd, &w, &h);
	rect[0] = rect[1] = 0;
	rect[2] = w;
	rect[3] = h;
	return ok;
}
static inline int ClientToScreen(void *wnd, long *point)
{
	int x = (int)point[0], y = (int)point[1];
	int ok = zoosters::os::ClientToScreen(wnd, &x, &y);
	point[0] = x;
	point[1] = y;
	return ok;
}
static inline void SetCursorPos(int x, int y) { zoosters::os::SetCursorPos(x, y); }

extern const DiGuid g_guidSysKeyboard;     // GLOBAL: 0x5d85bc g_guidSysKeyboard (GUID_SysKeyboard, 6F1D2B61-D5A0-11CF-BFC7-444553540000: verified)
extern const DiGuid g_guidSysMouse;        // GLOBAL: 0x5d85cc g_guidSysMouse (GUID_SysMouse, 6F1D2B60-...: verified)
extern const DiGuid g_guidIDirectInput8A;  // GLOBAL: 0x5d866c g_guidIDirectInput8A (BF798030-483A-4DA2-AA99-5D64ED369700: verified)
extern const DiGuid g_guidNullPad;         // GLOBAL: 0x5dc010 g_guidNullPad (16 zero bytes in the retail exe: verified)
extern const DiGuid g_guidConstantForce;   // GLOBAL: 0x5d85ac g_guidConstantForce (GUID_ConstantForce)
extern const DIDATAFORMAT c_dfDIKeyboard;  // GLOBAL: 0x5fbfac c_dfDIKeyboard (dwSize 0x18, dwObjSize 0x10: verified)
extern const DIDATAFORMAT c_dfDIMouse;     // GLOBAL: 0x5fbda4 c_dfDIMouse (same header bytes)
extern const DIDATAFORMAT g_dfJoystick2;   // GLOBAL: 0x5fbb9c g_dfJoystick2 (c_dfDIJoystick2: the format of the 0x110 byte pad state)

// Looked up in dinput8.dll by the constructor; the game doesn't link dinput8.lib.
extern DirectInput8CreateFn g_pDirectInput8Create;   // GLOBAL: 0x62f158 g_pDirectInput8Create
extern void *g_hDInput8;                   // GLOBAL: 0x62f160 g_hDInput8
extern unsigned char g_dinputLoaded;       // GLOBAL: 0x62f164 g_dinputLoaded

enum {
	MAPVK_VSC_TO_VK = 1,
	MAPVK_VK_TO_CHAR = 2,
	GUID_TEXT_LENGTH = 36      // "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"
};

// 0x51a7d0
InputManager::InputManager()
{
	m_smoothMouse = 0;
	m_flushHeld = 0;
	m_initialised = 0;
	m_mouseSens = 0.01f;
	m_numContexts = INPUT_MAX_PLAYERS;
	m_keyboard = 0;
	m_mouse = 0;
	m_dinput = 0;
	memset(&m_mouseState, 0, sizeof(m_mouseState));
	memset(&m_mousePrev, 0, sizeof(m_mousePrev));
	memset(&m_mouseSnap, 0, sizeof(m_mouseSnap));
	memset(m_padState, 0, sizeof(m_padState));
	memset(m_padPrev, 0, sizeof(m_padPrev));
	memset(m_padSnap, 0, sizeof(m_padSnap));
	for (int i = 0; i < INPUT_MAX_PADS; i++)
		m_padDevices[i] = 0;
	for (int i = 0; i < INPUT_MAX_PADS; i++)
		m_padEffects[i] = 0;
	for (int i = 0; i < INPUT_MAX_PADS; i++)
		m_padFFAxes[i] = 0;
	memset(m_padFFResolution, 0, sizeof(m_padFFResolution));
	for (int i = 0; i < INPUT_MAX_PLAYERS; i++)
		m_contexts[i] = 0;
	m_padPolledMask = 0;
}

// 0x51b440
InputManager::~InputManager()
{
	ReleaseDevices();
}

// As in the game only the first four pad slots are released, and the mouse device not at all.
// 0x51ab70
ZOOSTERS_NOINLINE void InputManager::ReleaseDevices()
{
	const int releasedPads = 4;
	for (int i = 0; i < releasedPads; i++) {
		if (m_padDevices[i]) {
			m_padDevices[i]->Release();
			m_padDevices[i] = 0;
		}
		if (m_padEffects[i]) {
			m_padEffects[i]->Release();
			m_padEffects[i] = 0;
		}
	}
	if (m_keyboard) {
		m_keyboard->Release();
		m_keyboard = 0;
	}
	if (m_dinput) {
		m_dinput->Release();
		m_dinput = 0;
	}
	m_initialised = 0;
}

// 0x51a9c0
void InputManager::SetSmoothMouse(char on)
{
	m_smoothMouse = on;
}

// 0x51b150
DIJOYSTATE2 *InputManager::GetPadState(int index)
{
	return &m_padState[index];
}

// 0x51b170
bool InputManager::HasForceFeedback(int index)
{
	if (m_padDevices[index])
		return m_padFFAxes[index] > 0;
	return false;
}

// 0x51b340
char InputManager::CreateContext(CtlBuf **out)
{
	*out = new CtlBuf;
	return 1;
}

struct WinPoint {
	long x, y;
};
struct WinRect {
	long left, top, right, bottom;
};

// A context with m_centreCursor also recentres the mouse. Everything held down is hidden until released (m_flushHeld, see Poll): a key that opened the screen must not act in it.
// 0x403f70
void InputManager::SetPlayerContext(int index, CtlBuf *context)
{
	if (m_contexts[index] == context)
		return;
	m_contexts[index] = context;
	if (context->m_centreCursor) {
		WinRect client;
		GetClientRect(m_hwnd, (long *)&client);
		WinPoint centre;
		centre.x = (client.right - client.left) / 2;
		centre.y = (client.bottom - client.top) / 2;
		ClientToScreen(m_hwnd, (long *)&centre);
		SetCursorPos(centre.x, centre.y);
	}
	memcpy(m_keysSnap, m_keys, sizeof(m_keys));
	memcpy(m_padSnap, m_padState, sizeof(m_padState));
	m_mouseSnap = m_mouseState;
	m_flushHeld = 1;
}

// Upper-case hex digits only.
// 0x51a9d0
unsigned ParseHex(const char **pp)
{
	unsigned value = 0;
	while ((**pp >= '0' && **pp <= '9') || (**pp <= 'F' && **pp >= 'A')) {
		value <<= 4;
		if (**pp >= '0' && **pp <= '9')
			value += *(*pp)++ - '0';
		else
			value += *(*pp)++ - 55;
	}
	return value;
}

// Text of another length gives the all-zero GUID, which matches no device.
// 0x51aa30
void InputManager::SetPadGuid(int index, const char *text)
{
	DiGuid guid;
	memset(&guid, 0, sizeof(guid));
	const char *p = text;
	if (strlen(text) == GUID_TEXT_LENGTH) {
		guid.m_data1 = ParseHex(&p);
		p++;
		guid.m_data2 = (unsigned short)ParseHex(&p);
		p++;
		guid.m_data3 = (unsigned short)ParseHex(&p);
		p++;
		unsigned v = ParseHex(&p);
		guid.m_data4[0] = (unsigned char)(v >> 8);
		guid.m_data4[1] = (unsigned char)v;
		p++;
		// cut the next four digits out so ParseHex stops after them
		char group[8];
		memcpy(group, p, 4);
		group[4] = 0;
		const char *q = group;
		v = ParseHex(&q);
		guid.m_data4[2] = (unsigned char)(v >> 8);
		guid.m_data4[3] = (unsigned char)v;
		p += 4;
		v = ParseHex(&p);
		guid.m_data4[4] = (unsigned char)(v >> 24);
		guid.m_data4[5] = (unsigned char)(v >> 16);
		guid.m_data4[6] = (unsigned char)(v >> 8);
		guid.m_data4[7] = (unsigned char)v;
	}
	m_padGuids[index] = guid;
}

// Labels for keys whose scan code names no printable character.
struct KeyLabel {
	int m_scanCode;
	const char *m_label;
};
static const KeyLabel kKeyLabels[] = {
	{ 1, "ESC" }, { 14, "BACK" }, { 15, "TAB" }, { 28, "ENTER" }, { 157, "RCTRL" }, { 29, "LCTRL" }, { 42, "LSHIFT" },
	{ 54, "RSHIFT" }, { 57, "SPACE" }, { 58, "CAPSL" }, { 70, "SCROLL" }, { 56, "LALT" }, { 184, "RALT" }, { 197, "PAUSE" },
	{ 199, "HOME" }, { 201, "PGUP" }, { 200, "UP" }, { 203, "LEFT" }, { 208, "DOWN" }, { 205, "RIGHT" }, { 211, "DEL" },
	{ 210, "INS" }, { 207, "END" }, { 209, "PGDWN+" }, { 179, "KP." }, { 156, "KPEnter" }, { 181, "/" }, { 74, "-" },
	{ 69, "NumL" }, { 78, "+" }, { 55, "*" }, { 82, "KP0" }, { 79, "KP1" }, { 80, "KP2" }, { 81, "KP3" }, { 75, "KP4" },
	{ 76, "KP5" }, { 77, "KP6" }, { 71, "KP7" }, { 72, "KP8" }, { 73, "KP9" }, { 59, "F1" }, { 60, "F2" }, { 61, "F3" },
	{ 62, "F4" }, { 63, "F5" }, { 64, "F6" }, { 65, "F7" }, { 66, "F8" }, { 67, "F9" }, { 68, "F10" }, { 87, "F11" },
	{ 88, "F12" }, { 100, "F13" }, { 101, "F14" }, { 102, "F15" }
};

// Builds the key-name table: the character MapVirtualKeyA gives for the scan code (0xB4 shown as an apostrophe), then the fixed labels.
// Returns 1 when set up, also when it already was.
// 0x51bf30
char InputManager::Init(void *hwnd)
{
	if (m_initialised)
		return 1;
	if (!hwnd)
		return 0;
	if (!m_dinput) {
		m_hwnd = hwnd;
		g_pDirectInput8Create(0, DIRECTINPUT_VERSION, &g_guidIDirectInput8A, &m_dinput, 0);
		ScanDevices(0);
		for (unsigned scanCode = 0; scanCode < 0x100; scanCode++) {
			unsigned char ch = (unsigned char)zoosters::os::ScanCodeToChar(scanCode);
			if (ch == 0xb4)
				ch = '\'';
			m_keyNames[scanCode][0] = ch;
			m_keyNames[scanCode][1] = 0;
		}
		for (unsigned i = 0; i < sizeof(kKeyLabels) / sizeof(kKeyLabels[0]); i++)
			strcpy(m_keyNames[kKeyLabels[i].m_scanCode], kKeyLabels[i].m_label);
		m_initialised = 1;
	}
	return 1;
}

// DIEnumDevices callback; the reference is the manager.
// 0x51bbb0
int __stdcall PlatEnumDevicesCallback(const DIDEVICEINSTANCEA *info, void *manager)
{
	return ((InputManager *)manager)->OnEnumDevice(info);
}

static void MovePadSlot(InputManager *im, int to, int from)
{
	im->m_padPresent[to] = 1;
	im->m_padPresent[from] = 0;
	memset(&im->m_padState[to], 0, sizeof(DIJOYSTATE2));
	memset(&im->m_padPrev[to], 0, sizeof(DIJOYSTATE2));
	strcpy(im->m_padNames[to], im->m_padNames[from]);
	memset(im->m_padNames[from], 0, sizeof(im->m_padNames[from]));
	im->m_padGuids[to] = im->m_padGuids[from];
	im->m_padGuids[from] = g_guidNullPad;
	im->m_padDevices[to] = im->m_padDevices[from];
	im->m_padEffects[to] = im->m_padEffects[from];
	im->m_padDevices[from] = 0;
	im->m_padFFAxes[to] = im->m_padFFAxes[from];
	im->m_padEffects[from] = 0;
	im->m_padFFResolution[to][0] = im->m_padFFResolution[from][0];
	im->m_padFFAxes[from] = 0;
	im->m_padFFResolution[to][1] = im->m_padFFResolution[from][1];
	im->m_padFFResolution[from][0] = 0;
	im->m_padFFResolution[from][1] = 0;
}

// OnEnumDevice marks the pads still present in m_padPresent; gone ones are dropped and the slots compacted. adoptGuid: a controller
// with an unknown GUID takes a free slot and the GUID is remembered (m_scanAdoptGuid, read by OnEnumDevice).
// 0x51bbd0
void InputManager::ScanDevices(char adoptGuid)
{
	m_scanAdoptGuid = adoptGuid;
	memset(m_padPresent, 0, sizeof(m_padPresent));
	m_dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, PlatEnumDevicesCallback, this, DIEDFL_ATTACHEDONLY);
	if (!m_keyboard) {
		IDirectInputDevice8A *device;
		if (m_dinput->CreateDevice(&g_guidSysKeyboard, &device, 0) >= 0) {
			m_keyboard = device;
			device->SetDataFormat(&c_dfDIKeyboard);
			device->SetCooperativeLevel(m_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
			device->Acquire();
		}
	}
	if (!m_mouse) {
		IDirectInputDevice8A *device;
		if (m_dinput->CreateDevice(&g_guidSysMouse, &device, 0) >= 0) {
			m_mouse = device;
			device->SetDataFormat(&c_dfDIMouse);
			device->SetCooperativeLevel(m_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
			device->Acquire();
		}
	}
	for (int i = 0; i < INPUT_MAX_PADS; i++) {
		if (!m_padPresent[i] && m_padDevices[i]) {
			memset(&m_padState[i], 0, sizeof(DIJOYSTATE2));
			memset(&m_padPrev[i], 0, sizeof(DIJOYSTATE2));
			m_padDevices[i]->Release();
			m_padDevices[i] = 0;
			if (m_padEffects[i])
				m_padEffects[i]->Release();
			m_padFFResolution[i][0] = 0;
			m_padFFResolution[i][1] = 0;
			m_padEffects[i] = 0;
			m_padFFAxes[i] = 0;
		}
	}
	// every empty slot takes the next present pad behind it
	for (int slot = 0; slot < INPUT_MAX_PADS; slot++) {
		if (m_padPresent[slot])
			continue;
		for (int next = slot + 1; next < INPUT_MAX_PADS; next++) {
			if (m_padPresent[next]) {
				MovePadSlot(this, slot, next);
				break;
			}
		}
	}
	m_scanAdoptGuid = 0;
}

// Sets the axis range (-1000..1000) and records the force resolution of force-feedback actuators per device.
// 0x51b3b0
int InputManager::OnEnumObject(const DIDEVICEOBJECTINSTANCEA *object, IDirectInputDevice8A *device)
{
	DIPROPRANGE range;
	range.diph.dwSize = sizeof(DIPROPRANGE);
	range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	range.diph.dwHow = DIPH_BYID;
	range.diph.dwObj = object->dwType;
	range.lMin = -1000;
	range.lMax = 1000;
	device->SetProperty(DIPROP_RANGE, &range.diph);
	// only slots 0 and 1 are told apart; the objects of every other pad are counted for slot 0
	int slot = 0;
	if (m_padDevices[1] == device)
		slot = 1;
	if (object->dwFlags & DIDOI_FFACTUATOR) {
		unsigned axes = m_padFFAxes[slot];
		if (axes < 2) {
			m_padFFResolution[slot][axes] = object->dwFFForceResolution;
			m_padFFAxes[slot]++;
		}
	}
	return 1;
}

// The reference holds the manager and the device being enumerated.
struct EnumObjectsRef {
	InputManager *m_manager;
	IDirectInputDevice8A *m_device;
};

// 0x51b8e0
int __stdcall PlatEnumObjectsCallback(const DIDEVICEOBJECTINSTANCEA *object, void *ref)
{
	EnumObjectsRef *r = (EnumObjectsRef *)ref;
	return r->m_manager->OnEnumObject(object, r->m_device);
}

// A known, already created device is left alone.
// 0x51b900
int InputManager::OnEnumDevice(const DIDEVICEINSTANCEA *info)
{
	const DiGuid *guid = &info->guidInstance;
	int slot = -1;
	for (int i = 0; i < INPUT_MAX_PADS; i++) {
		if (memcmp(&m_padGuids[i], guid, sizeof(DiGuid)) == 0) {
			slot = i;
			break;
		}
	}
	if (slot < 0) {
		// an unknown device takes the first free slot, which adopts its GUID only if the scan asks to
		for (int i = 0; i < INPUT_MAX_PADS; i++) {
			if (m_padDevices[i] == 0) {
				slot = i;
				if (m_scanAdoptGuid)
					m_padGuids[i] = *guid;
				break;
			}
		}
		if (slot < 0)
			return 1;
	}
	IDirectInputDevice8A *device = (IDirectInputDevice8A *)-1;
	bool reuse = false;
	if (memcmp(&m_padGuids[slot], guid, sizeof(DiGuid)) == 0 && m_padDevices[slot]) {
		device = m_padDevices[slot];
		reuse = true;
	} else {
		if (m_dinput->CreateDevice(guid, &device, 0) < 0)
			return 1;
	}
	// when the GUID was not adopted no slot matches; the device just created is dropped without a Release
	int k = 0;
	while (k < INPUT_MAX_PADS) {
		if (memcmp(&m_padGuids[k], guid, sizeof(DiGuid)) == 0)
			break;
		k++;
	}
	if (k >= INPUT_MAX_PADS)
		return 1;
	m_padPresent[k] = 1;
	IDirectInputDevice8A *old = m_padDevices[k];
	if (old && !reuse)
		old->Release();
	m_padDevices[k] = device;
	strncpy(m_padNames[k], info->tszInstanceName, sizeof(m_padNames[k]));
	if (reuse)
		return 1;
	device->SetDataFormat(&g_dfJoystick2);
	device->SetCooperativeLevel(m_hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	EnumObjectsRef ref;
	ref.m_manager = this;
	ref.m_device = device;
	device->EnumObjects(PlatEnumObjectsCallback, &ref, DIDFT_AXIS);
	if (m_padFFAxes[k] > 0) {
		// started later by SetPadForce
		DIEFFECT effect;
		memset(&effect, 0, sizeof(effect));
		long direction[2];
		unsigned axes[2];
		long magnitude;
		axes[0] = DIJOFS_X;
		axes[1] = DIJOFS_Y;
		direction[0] = 0;
		direction[1] = 0;
		magnitude = 0;
		effect.dwSize = sizeof(DIEFFECT);
		effect.dwFlags = DIEFF_OBJECTOFFSETS | DIEFF_SPHERICAL;
		effect.dwDuration = DI_INFINITE;
		effect.dwSamplePeriod = 0;
		effect.dwGain = DI_FFNOMINALMAX;
		effect.dwTriggerButton = DIEB_NOTRIGGER;
		effect.dwTriggerRepeatInterval = 0;
		effect.cAxes = 2;
		effect.rgdwAxes = axes;
		effect.rglDirection = direction;
		effect.lpEnvelope = 0;
		effect.cbTypeSpecificParams = 4;
		effect.lpvTypeSpecificParams = &magnitude;
		effect.dwStartDelay = 0;
		IDirectInputEffect **slotEffect = &m_padEffects[k];
		if (*slotEffect)
			(*slotEffect)->Release();
		device->CreateEffect(&g_guidConstantForce, &effect, slotEffect, 0);
	}
	device->Acquire();
	return 1;
}

// Both strengths 0 stops the effect. Otherwise they are scaled by the per-axis force limits (m_padFFResolution, /65536) and the
// effect starts with a magnitude of 0..10000 (one force axis: x/limit0; else the length of (x,y) / limit0), direction 0,0.
// 0x51b1a0
void InputManager::SetPadForce(int index, int strength0, int strength1)
{
	IDirectInputEffect *effect = m_padEffects[index];
	if (!effect)
		return;
	if (strength0 == 0 && strength1 == 0) {
		effect->Stop();
		return;
	}
	unsigned limit0 = m_padFFResolution[index][0];
	unsigned limit1 = m_padFFResolution[index][1];
	long direction[2];
	direction[1] = 0;
	unsigned x = (unsigned)(int)((float)limit0 * (float)(unsigned)strength0 * 0.0000152587890625f);
	unsigned y = (unsigned)(int)((float)limit1 * (float)(unsigned)strength1 * 0.0000152587890625f);
	unsigned magnitude;
	if (m_padFFAxes[index] == 1) {
		magnitude = (unsigned)(int)((float)x / (float)limit0 * 10000.0f);
		direction[0] = 0;
	} else {
		direction[0] = x;
		direction[1] = y;
		magnitude = (unsigned)(int)(sqrt((float)x * (float)x + (float)y * (float)y) / (float)limit0 * 10000.0f);
	}
	DIEFFECT params;
	memset(&params, 0, sizeof(params));
	params.rglDirection = direction;
	params.dwSize = sizeof(DIEFFECT);
	params.dwFlags = DIEFF_OBJECTOFFSETS | DIEFF_SPHERICAL;
	params.cAxes = 2;
	params.lpEnvelope = 0;
	params.cbTypeSpecificParams = 4;
	params.lpvTypeSpecificParams = &magnitude;
	params.dwStartDelay = 0;
	effect->SetParameters(&params, DIEP_START | DIEP_TYPESPECIFICPARAMS | DIEP_DIRECTION);
}
