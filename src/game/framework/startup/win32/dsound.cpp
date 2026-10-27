// The DirectSound object the movie player (Bink) plays on, opened around every movie.
// CFLAGS: /GS
#include "zoosters/base.h"
#include "framework/startup/win32/platform.h"

// IDirectSound8: only the methods used are named, the rest keep the slots.
struct DirectSound {
	virtual long __stdcall QueryInterface() = 0;
	virtual unsigned long __stdcall AddRef() = 0;
	virtual unsigned long __stdcall Release() = 0;
	virtual long __stdcall CreateSoundBuffer() = 0;
	virtual long __stdcall GetCaps() = 0;
	virtual long __stdcall DuplicateSoundBuffer() = 0;
	virtual long __stdcall SetCooperativeLevel(void *hwnd, unsigned level) = 0;
};

static const unsigned DSSCL_PRIORITY = 2;

// GLOBAL: 0x62f1e0 g_dsoundObj

// 0x5724c0: DSOUND.DLL import, ordinal 11.
extern "C" {
long __stdcall FUN_005724c0(void *guid, DirectSound **out, void *outer);
}

// 0x51de10
void DSound_Init(void *hwnd)
{
	if (!g_dsoundObj)
		FUN_005724c0(0, &g_dsoundObj, 0);
	if (g_dsoundObj)
		g_dsoundObj->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
}

// 0x51de40
void DSound_Release()
{
	if (g_dsoundObj) {
		g_dsoundObj->Release();
		g_dsoundObj = 0;
	}
}

// Nothing reads these two parameters (initial values 20 and 1) and neither function has a caller.
extern int g_audioParamA;                  // GLOBAL: 0x60d9cc g_audioParamA
extern int g_audioParamB;                  // GLOBAL: 0x60d9d0 g_audioParamB
extern int g_audioValue214;                // GLOBAL: 0x62f214 g_audioValue214
// 0x51de60
void FUN_0051de60(int a, int b)
{
	g_audioParamA = a;
	g_audioParamB = b;
}

// 0x51de80
int FUN_0051de80()
{
	return g_audioValue214;
}
