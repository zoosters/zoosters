// The part of DirectInput 8 the input manager uses: the COM interfaces in vtable slot order (slots the game doesn't call are
// listed to keep the numbers right), the records it reads and writes, and the flag values it passes. The game loads dinput8.dll
// itself and calls DirectInput8Create through a pointer.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_DINPUT_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_DINPUT_H

struct DiGuid {
	unsigned m_data1;
	unsigned short m_data2;
	unsigned short m_data3;
	unsigned char m_data4[8];
};

// Header of the c_dfDI* tables in the game's data.
struct DIDATAFORMAT {
	unsigned dwSize;
	unsigned dwObjSize;
	unsigned dwFlags;
	unsigned dwDataSize;
	unsigned dwNumObjs;
	const void *rgodf;
};

// Bit 7 of a button byte = down.
struct DIMOUSESTATE2 {
	int lX;
	int lY;
	int lZ;
	unsigned char rgbButtons[8];
};

// rgdwPOV[0]: the hat in hundredths of a degree, 0xffffffff = centred. Axes and sliders are scaled to -1000..1000 by SetProperty(DIPROP_RANGE).
struct DIJOYSTATE2 {
	int lX, lY, lZ;
	int lRx, lRy, lRz;
	int rglSlider[2];
	unsigned rgdwPOV[4];
	unsigned char rgbButtons[128];
	int lVX, lVY, lVZ;
	int lVRx, lVRy, lVRz;
	int rglVSlider[2];
	int lAX, lAY, lAZ;
	int lARx, lARy, lARz;
	int rglASlider[2];
	int lFX, lFY, lFZ;
	int lFRx, lFRy, lFRz;
	int rglFSlider[2];
};

struct DIDEVICEINSTANCEA {
	unsigned dwSize;
	DiGuid guidInstance;        // +0x04
	DiGuid guidProduct;         // +0x14
	unsigned dwDevType;         // +0x24
	char tszInstanceName[260];  // +0x28
	char tszProductName[260];
	DiGuid guidFFDriver;
	unsigned short wUsagePage;
	unsigned short wUsage;
};

struct DIDEVICEOBJECTINSTANCEA {
	unsigned dwSize;
	DiGuid guidType;
	unsigned dwOfs;
	unsigned dwType;                 // +0x18
	unsigned dwFlags;                // +0x1c (DIDOI_FFACTUATOR)
	char tszName[260];
	unsigned dwFFMaxForce;
	unsigned dwFFForceResolution;    // +0x128
	unsigned short wCollectionNumber;
	unsigned short wDesignatorIndex;
	unsigned short wUsagePage;
	unsigned short wUsage;
	unsigned dwDimension;
	unsigned short wExponent;
	unsigned short wReserved;
};

// The property set on every axis of a pad.
struct DIPROPHEADER {
	unsigned dwSize;
	unsigned dwHeaderSize;
	unsigned dwObj;
	unsigned dwHow;
};
struct DIPROPRANGE {
	DIPROPHEADER diph;
	int lMin;
	int lMax;
};

// The constant-force effect of a pad.
struct DIEFFECT {
	unsigned dwSize;
	unsigned dwFlags;
	unsigned dwDuration;
	unsigned dwSamplePeriod;
	unsigned dwGain;
	unsigned dwTriggerButton;
	unsigned dwTriggerRepeatInterval;
	unsigned cAxes;
	unsigned *rgdwAxes;
	long *rglDirection;
	void *lpEnvelope;
	unsigned cbTypeSpecificParams;
	void *lpvTypeSpecificParams;
	unsigned dwStartDelay;
};

struct IDirectInputEffect {
	virtual long __stdcall QueryInterface(const void *iid, void **out) = 0;
	virtual unsigned long __stdcall AddRef() = 0;
	virtual unsigned long __stdcall Release() = 0;
	virtual long __stdcall Initialize(void *hinst, unsigned version, const DiGuid *guid) = 0;
	virtual long __stdcall GetEffectGuid(DiGuid *guid) = 0;
	virtual long __stdcall GetParameters(DIEFFECT *effect, unsigned flags) = 0;
	virtual long __stdcall SetParameters(const DIEFFECT *effect, unsigned flags) = 0;   // 6
	virtual long __stdcall Start(unsigned iterations, unsigned flags) = 0;
	virtual long __stdcall Stop() = 0;                                                  // 8
};

struct IDirectInputDevice8A {
	typedef int (__stdcall *EnumObjectsCallback)(const DIDEVICEOBJECTINSTANCEA *object, void *ref);
	virtual long __stdcall QueryInterface(const void *iid, void **out) = 0;
	virtual unsigned long __stdcall AddRef() = 0;
	virtual unsigned long __stdcall Release() = 0;                                      // 2
	virtual long __stdcall GetCapabilities(void *caps) = 0;
	virtual long __stdcall EnumObjects(EnumObjectsCallback callback, void *ref, unsigned flags) = 0;   // 4
	virtual long __stdcall GetProperty(const void *prop, DIPROPHEADER *header) = 0;
	virtual long __stdcall SetProperty(const void *prop, const DIPROPHEADER *header) = 0;   // 6
	virtual long __stdcall Acquire() = 0;                                               // 7
	virtual long __stdcall Unacquire() = 0;
	virtual long __stdcall GetDeviceState(unsigned size, void *data) = 0;               // 9
	virtual long __stdcall GetDeviceData(unsigned size, void *data, unsigned *count, unsigned flags) = 0;
	virtual long __stdcall SetDataFormat(const DIDATAFORMAT *format) = 0;               // 11
	virtual long __stdcall SetEventNotification(void *event) = 0;
	virtual long __stdcall SetCooperativeLevel(void *hwnd, unsigned flags) = 0;         // 13
	virtual long __stdcall GetObjectInfo(void *object, unsigned objId, unsigned how) = 0;
	virtual long __stdcall GetDeviceInfo(void *info) = 0;
	virtual long __stdcall RunControlPanel(void *owner, unsigned flags) = 0;
	virtual long __stdcall Initialize(void *hinst, unsigned version, const DiGuid *guid) = 0;
	virtual long __stdcall CreateEffect(const DiGuid *guid, const DIEFFECT *effect, IDirectInputEffect **out, void *outer) = 0;   // 18
	virtual long __stdcall EnumEffects(void *callback, void *ref, unsigned type) = 0;
	virtual long __stdcall GetEffectInfo(void *info, const DiGuid *guid) = 0;
	virtual long __stdcall GetForceFeedbackState(unsigned *state) = 0;
	virtual long __stdcall SendForceFeedbackCommand(unsigned command) = 0;
	virtual long __stdcall EnumCreatedEffectObjects(void *callback, void *ref, unsigned flags) = 0;
	virtual long __stdcall Escape(void *escape) = 0;
	virtual long __stdcall Poll() = 0;                                                  // 25
};

struct IDirectInput8A {
	typedef int (__stdcall *EnumDevicesCallback)(const DIDEVICEINSTANCEA *device, void *ref);
	virtual long __stdcall QueryInterface(const void *iid, void **out) = 0;
	virtual unsigned long __stdcall AddRef() = 0;
	virtual unsigned long __stdcall Release() = 0;
	virtual long __stdcall CreateDevice(const DiGuid *guid, IDirectInputDevice8A **out, void *outer) = 0;   // 3
	virtual long __stdcall EnumDevices(unsigned devType, EnumDevicesCallback callback, void *ref, unsigned flags) = 0;   // 4
};

typedef long (__stdcall *DirectInput8CreateFn)(void *hinst, unsigned version, const DiGuid *iid, IDirectInput8A **out, void *outer);

// Values the game passes.
enum {
	DIRECTINPUT_VERSION = 0x0800,
	DI8DEVCLASS_GAMECTRL = 4,
	DIEDFL_ATTACHEDONLY = 1,
	DISCL_EXCLUSIVE = 1,
	DISCL_NONEXCLUSIVE = 2,
	DISCL_FOREGROUND = 4,
	DIDFT_AXIS = 3,                    // DIDFT_RELAXIS | DIDFT_ABSAXIS
	DIPH_BYID = 2,
	DIDOI_FFACTUATOR = 1,
	DIEFF_OBJECTOFFSETS = 2,
	DIEFF_SPHERICAL = 0x10,
	DIEP_DIRECTION = 0x40,
	DIEP_TYPESPECIFICPARAMS = 0x100,
	DIEP_START = 0x20000000,
	DIEB_NOTRIGGER = 0xffffffff,
	DI_FFNOMINALMAX = 10000
};
// (long)(int) keeps the 32-bit negative HRESULTs negative where long is 64-bit.
const long DIERR_NOTACQUIRED = (long)(int)0x8007000C;
const long DIERR_INPUTLOST = (long)(int)0x8007001E;
#define DIPROP_RANGE ((const void *)4)
#define DIJOFS_X 0                         // offsets of the first two axes in DIJOYSTATE2
#define DIJOFS_Y 4
#define DI_INFINITE 0xffffffff

#endif
