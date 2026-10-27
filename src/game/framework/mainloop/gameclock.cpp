#include "framework/mainloop/gameclock.h"
#include "framework/startup/win32/systemhook.h"
#include "script/types/scriptsystem.h"

// 0x461c30
bool TFBClock_Resume()
{
	if (TFBClockStart) {
		TFBClockBase += *Script::ScriptSystem::GetScriptTimePtr() - TFBClockStart;
		TFBClockStart = 0;
		return true;
	}
	return false;
}

// 0x461c70
int TFBClock_Stop()
{
	if (TFBClockStart != 0) {
		TFBClockBase += *Script::ScriptSystem::GetScriptTimePtr() - TFBClockStart;
		TFBClockStart = 0;
	}
	return TFBClockStart = *Script::ScriptSystem::GetScriptTimePtr();
}

// 0x461cb0
int TFBClock_GetRawTime()
{
	int now = TFBClockStart;
	if (now == 0)
		now = *Script::ScriptSystem::GetScriptTimePtr();
	return now - TFBClockBase;
}

// 0x461cd0
float TFBClock_GetRawSeconds()
{
	return (float)(unsigned)TFBClock_GetRawTime() * 0.001f;
}

// 0x461d00
void TFBClock_SetTime(int v)
{
	int now = TFBClockStart;
	if (now == 0)
		now = *Script::ScriptSystem::GetScriptTimePtr();
	TFBClockSum = TFBClockBase - now + v;
}

// HACK: the game reads base and sum into registers before the arithmetic; cl does so only for volatile operands.
#define VOL(x) (*(volatile int *)&(x))

// 0x461d30
int TFBClock_GetTime()
{
	int ms = TFBClockStart;
	if (ms == 0)
		ms = *Script::ScriptSystem::GetScriptTimePtr();
	ms -= VOL(TFBClockBase);
	ms += VOL(TFBClockSum);
	return ms;
}

// 0x461d60
float TFBClock_GetSeconds()
{
	int ms = TFBClockStart;
	if (ms == 0)
		ms = *Script::ScriptSystem::GetScriptTimePtr();
	ms -= VOL(TFBClockBase);
	ms += VOL(TFBClockSum);
	return (float)(unsigned)ms * 0.001f;
}

// 0x461da0
int TFBClock_IsStopped()
{
	return TFBClockStart != 0;
}

// 0x461e00
void TFBHook_Pause()
{
	TFBHook.Pause();
}

// 0x461e10
void TFBHook_Resume()
{
	TFBHook.Resume();
}

// 0x461e20
float Script::ScriptSystem::ExposeGameTime::GetFloat(Script::ScriptReference *owner)
{
	int ms = TFBClockStart;
	if (ms == 0)
		ms = *Script::ScriptSystem::GetScriptTimePtr();
	ms -= VOL(TFBClockBase);
	ms += VOL(TFBClockSum);
	return (float)(unsigned)ms * 0.001f;
}

// GLOBAL: 0x62a380 TFBClockStart
// GLOBAL: 0x62a384 TFBClockSum
// GLOBAL: 0x62a388 TFBClockBase
