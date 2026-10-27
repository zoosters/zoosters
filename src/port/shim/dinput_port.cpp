// DirectInput 8 for the game's InputManager (instead of dinput8.dll), fed by the port's platform input layer:
//   keyboard  port_keyboard_frame (real keyboard while focused, PORT_KEYS / PORT_INPUT / PORT_LEVEL)
//   mouse     port_mouse_frame    (raw deltas while focused, or PORT_MOUSE)
//   pad       port_gamepad_state  (first SDL gamepad; none without SDL)
// The pad's instance GUID is all zero, the value of the unset Controls\Gamepads\PadID1, so the game files it in slot 0. Two
// force-feedback actuators (resolution 10000) make the game create its constant-force effect; SetParameters' direction values are the
// motor levels. port_dinput_install runs before WinMain and sets the two globals the InputManager constructor checks, so it skips
// loading dinput8.dll. Deviation: DISCL_FOREGROUND is modelled by the sources; Acquire, Poll and GetDeviceState always succeed.
#include <stdio.h>
#include <string.h>
#include "framework/startup/win32/input/dinput.h"

extern "C" FILE *port_log;
extern "C" void port_keyboard_frame(unsigned char *keys);   // 256 DIK-indexed bytes, 0x80 = down
extern "C" void port_mouse_frame(DIMOUSESTATE2 *state);     // relative movement since the previous call, wheel, buttons
extern "C" int port_gamepad_init(void);                     // 1 = gamepads can be read (port/provisional/gamepad_*.cpp)
extern "C" int port_gamepad_state(DIJOYSTATE2 *state);      // the pad, -1000..1000 axes (1 = one is connected)
extern "C" void port_gamepad_rumble(float low, float high); // motor levels 0..1

extern DirectInput8CreateFn g_pDirectInput8Create;   // GLOBAL: 0x62f158 g_pDirectInput8Create
extern unsigned char g_dinputLoaded;                 // GLOBAL: 0x62f164 g_dinputLoaded

namespace {

const unsigned GUID_SYSMOUSE_DATA1 = 0x6F1D2B60;      // GUID_SysMouse / GUID_SysKeyboard differ only in Data1
const unsigned GUID_SYSKEYBOARD_DATA1 = 0x6F1D2B61;
// (long)(int): negative also where long is 64-bit (macOS LP64), so the game's `< 0` failure tests hold
const long E_NOTIMPL_ = (long)(int)0x80004001;
const long E_NOINTERFACE_ = (long)(int)0x80004002;
const long DIERR_DEVICENOTREG_ = (long)(int)0x80040154;
const long DIERR_INVALIDPARAM_ = (long)(int)0x80070057;

enum DeviceKind { KIND_KEYBOARD, KIND_MOUSE, KIND_PAD };
const unsigned kFFResolution = 10000;   // dwFFForceResolution of the two actuators (the game divides by it)

// Constant-force effect: the direction values are the two motors (x = scripts' actuator 2, y = actuator 1). Actuator 1 drives the heavy
// motor: the game varies it and leaves actuator 2 at 0, which is barely felt on a DualSense's light motor.
struct PortEffect : IDirectInputEffect {
	unsigned long m_refs;
	PortEffect() : m_refs(1) {}
	long __stdcall QueryInterface(const void *, void **out) override { if (out) *out = 0; return E_NOINTERFACE_; }
	unsigned long __stdcall AddRef() override { return ++m_refs; }
	unsigned long __stdcall Release() override
	{
		unsigned long n = --m_refs;
		if (!n) { port_gamepad_rumble(0.0f, 0.0f); delete this; }
		return n;
	}
	long __stdcall Initialize(void *, unsigned, const DiGuid *) override { return 0; }
	long __stdcall GetEffectGuid(DiGuid *) override { return E_NOTIMPL_; }
	long __stdcall GetParameters(DIEFFECT *, unsigned) override { return E_NOTIMPL_; }
	long __stdcall SetParameters(const DIEFFECT *e, unsigned flags) override
	{
		if (!e) return DIERR_INVALIDPARAM_;
		static int logged;
		if (port_log && logged++ < 16)
			fprintf(port_log, "dinput_port: effect SetParameters(flags %x, axes %u, direction %ld %ld)\n", flags, e->cAxes,
			        e->rglDirection && e->cAxes >= 1 ? (long)e->rglDirection[0] : 0L, e->rglDirection && e->cAxes >= 2 ? (long)e->rglDirection[1] : 0L);
		if ((flags & DIEP_DIRECTION) && e->rglDirection && e->cAxes >= 2)
			port_gamepad_rumble((float)e->rglDirection[1] / kFFResolution / 100.0f, (float)e->rglDirection[0] / kFFResolution / 100.0f);
		return 0;
	}
	long __stdcall Start(unsigned, unsigned) override { return 0; }
	long __stdcall Stop() override { port_gamepad_rumble(0.0f, 0.0f); return 0; }
};

struct PortDevice : IDirectInputDevice8A {
	DeviceKind m_kind;
	unsigned long m_refs;
	explicit PortDevice(DeviceKind kind) : m_kind(kind), m_refs(1) {}
	long __stdcall QueryInterface(const void *, void **out) override { if (out) *out = 0; return E_NOINTERFACE_; }
	unsigned long __stdcall AddRef() override { return ++m_refs; }
	unsigned long __stdcall Release() override { unsigned long n = --m_refs; if (!n) delete this; return n; }
	long __stdcall GetCapabilities(void *) override { return E_NOTIMPL_; }
	long __stdcall EnumObjects(EnumObjectsCallback callback, void *ref, unsigned flags) override
	{
		if (m_kind != KIND_PAD || !callback || (flags && !(flags & DIDFT_AXIS)))
			return 0;
		static const char *const kAxes[6] = { "X Axis", "Y Axis", "Z Axis", "X Rotation", "Y Rotation", "Z Rotation" };
		for (unsigned i = 0; i < 6; i++) {   // the absolute axes; the first two carry the force feedback
			DIDEVICEOBJECTINSTANCEA o;
			memset(&o, 0, sizeof(o));
			o.dwSize = sizeof(o);
			o.dwOfs = i * 4;
			o.dwType = (i << 8) | 2;    // DIDFT_MAKEINSTANCE(i) | DIDFT_ABSAXIS
			o.dwFlags = i < 2 ? DIDOI_FFACTUATOR : 0;
			o.dwFFMaxForce = i < 2 ? kFFResolution : 0;
			o.dwFFForceResolution = i < 2 ? kFFResolution : 0;
			snprintf(o.tszName, sizeof(o.tszName), "%s", kAxes[i]);
			if (!callback(&o, ref))
				break;
		}
		return 0;
	}
	long __stdcall GetProperty(const void *, DIPROPHEADER *) override { return E_NOTIMPL_; }
	long __stdcall SetProperty(const void *, const DIPROPHEADER *) override { return 0; }
	long __stdcall Acquire() override { return 0; }
	long __stdcall Unacquire() override { return 0; }
	long __stdcall GetDeviceState(unsigned size, void *data) override
	{
		if (!data) return DIERR_INVALIDPARAM_;
		if (m_kind == KIND_PAD) {
			DIJOYSTATE2 st;
			port_gamepad_state(&st);
			memset(data, 0, size);
			memcpy(data, &st, size < sizeof(st) ? size : sizeof(st));
			return 0;
		}
		if (m_kind == KIND_KEYBOARD) {
			if (size != 256) return DIERR_INVALIDPARAM_;
			port_keyboard_frame((unsigned char *)data);
			return 0;
		}
		DIMOUSESTATE2 st;
		memset(&st, 0, sizeof(st));
		port_mouse_frame(&st);
		memset(data, 0, size);
		memcpy(data, &st, size < sizeof(st) ? size : sizeof(st));   // DIMOUSESTATE (16 bytes) is the prefix of DIMOUSESTATE2
		return 0;
	}
	long __stdcall GetDeviceData(unsigned, void *, unsigned *count, unsigned) override { if (count) *count = 0; return 0; }
	long __stdcall SetDataFormat(const DIDATAFORMAT *) override { return 0; }
	long __stdcall SetEventNotification(void *) override { return 0; }
	long __stdcall SetCooperativeLevel(void *, unsigned) override { return 0; }
	long __stdcall GetObjectInfo(void *, unsigned, unsigned) override { return E_NOTIMPL_; }
	long __stdcall GetDeviceInfo(void *) override { return E_NOTIMPL_; }
	long __stdcall RunControlPanel(void *, unsigned) override { return 0; }
	long __stdcall Initialize(void *, unsigned, const DiGuid *) override { return 0; }
	long __stdcall CreateEffect(const DiGuid *, const DIEFFECT *, IDirectInputEffect **out, void *) override
	{
		if (!out) return DIERR_INVALIDPARAM_;
		*out = m_kind == KIND_PAD ? new PortEffect : 0;
		if (port_log) fprintf(port_log, "dinput_port: CreateEffect -> %s\n", *out ? "the pad's rumble" : "none");
		return *out ? 0 : E_NOTIMPL_;
	}
	long __stdcall EnumEffects(void *, void *, unsigned) override { return 0; }
	long __stdcall GetEffectInfo(void *, const DiGuid *) override { return E_NOTIMPL_; }
	long __stdcall GetForceFeedbackState(unsigned *) override { return E_NOTIMPL_; }
	long __stdcall SendForceFeedbackCommand(unsigned) override { return E_NOTIMPL_; }
	long __stdcall EnumCreatedEffectObjects(void *, void *, unsigned) override { return 0; }
	long __stdcall Escape(void *) override { return E_NOTIMPL_; }
	long __stdcall Poll() override { return 0; }
};

struct PortDirectInput : IDirectInput8A {
	unsigned long m_refs;
	PortDirectInput() : m_refs(1) {}
	long __stdcall QueryInterface(const void *, void **out) override { if (out) *out = 0; return E_NOINTERFACE_; }
	unsigned long __stdcall AddRef() override { return ++m_refs; }
	unsigned long __stdcall Release() override { unsigned long n = --m_refs; if (!n) delete this; return n; }
	long __stdcall CreateDevice(const DiGuid *guid, IDirectInputDevice8A **out, void *) override
	{
		if (!out) return DIERR_INVALIDPARAM_;
		*out = 0;
		if (!guid) return DIERR_INVALIDPARAM_;
		static const DiGuid zero = {};
		if (guid->m_data1 == GUID_SYSKEYBOARD_DATA1) *out = new PortDevice(KIND_KEYBOARD);
		else if (guid->m_data1 == GUID_SYSMOUSE_DATA1) *out = new PortDevice(KIND_MOUSE);
		else if (!memcmp(guid, &zero, sizeof(zero)) && port_gamepad_init()) *out = new PortDevice(KIND_PAD);
		if (port_log) fprintf(port_log, "dinput_port: CreateDevice %08x -> %s\n", guid->m_data1, *out ? "ok" : "not available");
		return *out ? 0 : DIERR_DEVICENOTREG_;
	}
	long __stdcall EnumDevices(unsigned devType, EnumDevicesCallback callback, void *ref, unsigned) override
	{
		if (devType != DI8DEVCLASS_GAMECTRL || !callback || !port_gamepad_init())
			return 0;
		DIDEVICEINSTANCEA d;   // the one pad: GUID all zero (= the unset PadID1: pad slot 0), whatever SDL gamepad is connected
		memset(&d, 0, sizeof(d));
		d.dwSize = sizeof(d);
		d.dwDevType = 0x15;    // DI8DEVTYPE_GAMEPAD
		snprintf(d.tszInstanceName, sizeof(d.tszInstanceName), "Gamepad");
		snprintf(d.tszProductName, sizeof(d.tszProductName), "Gamepad");
		if (port_log) fprintf(port_log, "dinput_port: EnumDevices(game controllers): the port pad\n");
		callback(&d, ref);
		return 0;
	}
};

long __stdcall PortDirectInput8Create(void *, unsigned version, const DiGuid *, IDirectInput8A **out, void *)
{
	if (!out) return DIERR_INVALIDPARAM_;
	*out = new PortDirectInput;
	if (port_log) fprintf(port_log, "dinput_port: DirectInput8Create(version %04x): port keyboard + mouse\n", version);
	return 0;
}

}   // namespace

extern "C" void port_dinput_install(void)
{
	g_pDirectInput8Create = PortDirectInput8Create;
	g_dinputLoaded = 1;
}
