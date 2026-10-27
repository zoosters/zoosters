// Script::ScriptSystem's Expose members that reach into a behaviour: "radar enable display" switches the radar on and off.
#include "script/types/scriptsystem.h"
#include "modules/tfb/radarrender.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptcamera.h"
#include "tfb/level/level.h"
#include "modules/tfb/ctfbworld.h"
#include "script/types/twodmeasurement.h"
#include "script/runtime.h"
#include "script/bearings.h"
#include "tfb/level/levelmanager.h"
int *CutScene_StackSlot(int delta);              // 0x436c00
#include <math.h>

// GLOBAL: 0x6223e8 g_radarRender
// GLOBAL: 0x629710 g_nearClip
// GLOBAL: 0x62970c g_farClip
// GLOBAL: 0x62973c g_yonObject
// GLOBAL: 0x5ca5a8 g_floatZero

extern int g_nextLevel;       // GLOBAL: 0x62a2f8 g_nextLevel
extern int g_currentLevel;    // GLOBAL: 0x606a38 g_currentLevel
extern int TFBClockStart;     // GLOBAL: 0x62a380 TFBClockStart
extern int TFBClockSum;       // GLOBAL: 0x62a384 TFBClockSum
extern int TFBClockBase;      // GLOBAL: 0x62a388 TFBClockBase

namespace Script {

// 0x442690
int ScriptSystem::ExposeRadarControl::GetInt(ScriptReference *owner)
{
	if (g_radarRender)
		return g_radarRender->m_control;
	return 0;
}

// 0x4426b0
void ScriptSystem::ExposeRadarControl::SetValue(int value, ScriptReference *owner)
{
	if (g_radarRender)
		g_radarRender->m_control = value;
}

} // namespace Script

// the no-op Measurement::SetValue(int, owner) override shared by the read-only Expose* members (0x4025a0)
using namespace Script;

// 0x4025a0
void Script::ScriptSystem::ExposeVSyncFlipCount::SetValue(int value, Script::ScriptReference *owner)
{
}

// 0x41a5a0
float ScriptSystem::ExposeNearClip::GetFloat(ScriptReference *owner)
{
	return g_nearClip * 0.083333336f;
}

// 0x41a5b0
void ScriptSystem::ExposeNearClip::SetValue(float value, ScriptReference *owner)
{
	g_nearClip = value * 12.0f;
}

// 0x41a5d0
float ScriptSystem::ExposeFarClip::GetFloat(ScriptReference *owner)
{
	return g_farClip * 0.083333336f;
}

// 0x41a5e0
void ScriptSystem::ExposeFarClip::SetValue(float value, ScriptReference *owner)
{
	SetFarClip(value * 12.0f);
}

// 0x41a600
int ScriptSystem::ExposeRenderYonIgnore::GetInt(ScriptReference *owner)
{
	if (g_yonObject)
		return g_yonObject->m_yonIgnore;
	return 1;
}

// 0x41a620
void ScriptSystem::ExposeRenderYonIgnore::SetValue(int value, ScriptReference *owner)
{
	if (g_yonObject)
		g_yonObject->m_yonIgnore = value != 0;
}

// 0x41a640
int ScriptSystem::ExposeConsole::GetInt(ScriptReference *owner)
{
	return 1;
}

// 0x461080
int Script::ScriptSystem::ExposeNextLevel::GetInt(ScriptReference *owner)
{
	return g_nextLevel;
}

// 0x461090
void Script::ScriptSystem::ExposeNextLevel::SetValue(int value, ScriptReference *owner)
{
	g_nextLevel = value;
}

// 0x4610a0
int Script::ScriptSystem::ExposeCurrentLevel::GetInt(ScriptReference *owner)
{
	return g_currentLevel;
}

// 0x461e60
void Script::ScriptSystem::ExposeGameTime::SetValue(float value, ScriptReference *owner)
{
	TFBClockSum = (int)(value * 1000.0f);
}

Script::ScriptReference *TFBType_System();
// GLOBAL: 0x5cc9c8 g_millisToSeconds
extern const float g_millisToSeconds;
// GLOBAL: 0x6221b0 g_levelZone
// GLOBAL: 0x6221b4 g_playMode
// GLOBAL: 0x6221b8 g_vsyncFlipCount

namespace Script {

// 0x43c170
const char *ScriptSystem::ExposeNextLevel::GetName()
{
	return "Next Level Auto Load";
}

// 0x43c180
const char *ScriptSystem::ExposeGameTime::GetName()
{
	return "Game Running Time";
}

// 0x43e730
int ScriptSystem::ExposeLevelZone::GetInt(ScriptReference *owner)
{
	return g_levelZone;
}

// 0x43e740
void ScriptSystem::ExposeLevelZone::SetValue(int value, ScriptReference *owner)
{
	g_levelZone = value;
}

// 0x43e710
int ScriptSystem::ExposeVSyncFlipCount::GetInt(ScriptReference *owner)
{
	return g_vsyncFlipCount;
}

// 0x43e620
int ScriptSystem::ExposePlayMode::GetInt(ScriptReference *owner)
{
	return g_playMode;
}

// 0x43e600
int ScriptSystem::ExposeCutSceneActive::GetInt(ScriptReference *owner)
{
	return *CutScene_StackSlot(0) != 2;
}

// 0x43faf0
float ScriptSystem::ExposeScriptTime::GetFloat(ScriptReference *owner)
{
	if (g_curScript)
		return (float)(unsigned)(m_timeMs - g_curScript->m_startTime) * g_millisToSeconds;
	return g_floatZero;
}

// 0x43fb30
ScriptReference *ScriptSystem::GetType()
{
	return TFBType_System();
}

// 0x43e630
const char *ScriptSystem::GetName()
{
	return "system";
}

// 0x43e6a0
const char *ScriptSystem::ExposeNearClip::GetName()
{
	return "Near Clip";
}

// 0x43e6b0
const char *ScriptSystem::ExposeFarClip::GetName()
{
	return "Far Clip";
}

// 0x43e6c0
const char *ScriptSystem::ExposeRadarControl::GetName()
{
	return "radar enable display";
}

// 0x43e6d0
const char *ScriptSystem::ExposeCutSceneActive::GetName()
{
	return "Cut-scene Active";
}

// 0x43e6e0
const char *ScriptSystem::ExposeConsole::GetName()
{
	return "Hardware Target";
}

// 0x43e700
const char *ScriptSystem::ExposeVSyncFlipCount::GetName()
{
	return "VSync Flip Count";
}

// 0x43e720
const char *ScriptSystem::ExposeLevelZone::GetName()
{
	return "Level Zone";
}

// 0x43e750
const char *ScriptSystem::ExposeCurrentLevel::GetName()
{
	return "Current Level";
}

// 0x43e760
const char *ScriptSystem::ExposeDesignBuild::GetName()
{
	return "Design Build";
}

// 0x43e770
const char *ScriptSystem::ExposePlayMode::GetName()
{
	return "Play Mode";
}

// 0x43e780
const char *ScriptSystem::ExposeRenderYonIgnore::GetName()
{
	return "Render Yon Ignore";
}

// 0x43e7f0
const char *ScriptSystem::ExposeHardwareMetrics::GetName()
{
	return "Hardware Metrics";
}

// 0x43fae0
const char *ScriptSystem::ExposeScriptTime::GetName()
{
	return "Script Time";
}

}  // namespace Script


// 0x43e790
const char *ExposeBeenoxQuitGame_GetName()
{
	return "Beenox-Quit Game";
}

// 0x463620: the game binds ExposeBeenoxQuitGame::SetValue (slot 16) to the free function WindowCloseCallback(code, int) (config.cpp), whose
// arguments are the stack arguments of the x86 thiscall; as an x64 virtual call `this` comes first and the arguments shift by one, so the member is
// defined here.
void __stdcall WindowCloseCallback(int code, int);   // framework/startup/win32/config.cpp
// FOLDED: 0x463620 Script::ScriptSystem::ExposeBeenoxQuitGame::SetValue
void Script::ScriptSystem::ExposeBeenoxQuitGame::SetValue(int value, ScriptReference *owner)
{
	WindowCloseCallback(value, 0);
}
