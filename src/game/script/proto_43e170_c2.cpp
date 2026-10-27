// Prototype getters (static T s; return &s;).
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptsystem.h"
using namespace Script;

// GLOBAL: 0x5d3530 Script::ScriptSound::ExposeStreamCount::vftable
// 0x43e170
ScriptReference *TFBMember_Sound_StreamCount()
{
	static ScriptSound::ExposeStreamCount s;
	return &s;
}

// GLOBAL: 0x5d38d8 Script::ScriptSystem::ExposeNearClip::vftable
// 0x43eba0
ScriptReference *TFBMember_System_NearClip()
{
	static ScriptSystem::ExposeNearClip s;
	return &s;
}

// GLOBAL: 0x5d3928 Script::ScriptSystem::ExposeFarClip::vftable
// 0x43ebe0
ScriptReference *TFBMember_System_FarClip()
{
	static ScriptSystem::ExposeFarClip s;
	return &s;
}

// GLOBAL: 0x5d3978 Script::ScriptSystem::ExposeRadarControl::vftable
// 0x43ec20
ScriptReference *TFBMember_System_RadarControl()
{
	static ScriptSystem::ExposeRadarControl s;
	return &s;
}

// GLOBAL: 0x5d39d8 Script::ScriptSystem::ExposeCutSceneActive::vftable
// 0x43ec60
ScriptReference *TFBMember_System_CutSceneActive()
{
	static ScriptSystem::ExposeCutSceneActive s;
	return &s;
}

// GLOBAL: 0x5d3a30 Script::ScriptSystem::ExposeConsole::vftable
// 0x43eca0
ScriptReference *TFBMember_System_Console()
{
	static ScriptSystem::ExposeConsole s;
	return &s;
}

// GLOBAL: 0x5d3a88 Script::ScriptSystem::ExposeVSyncFlipCount::vftable
// 0x43ece0
ScriptReference *TFBMember_System_VSyncFlipCount()
{
	static ScriptSystem::ExposeVSyncFlipCount s;
	return &s;
}

// GLOBAL: 0x5d3ae0 Script::ScriptSystem::ExposeLevelZone::vftable
// 0x43ed20
ScriptReference *TFBMember_System_LevelZone()
{
	static ScriptSystem::ExposeLevelZone s;
	return &s;
}

// GLOBAL: 0x5d3b30 Script::ScriptSystem::ExposeCurrentLevel::vftable
// 0x43ed60
ScriptReference *TFBMember_System_CurrentLevel()
{
	static ScriptSystem::ExposeCurrentLevel s;
	return &s;
}

// GLOBAL: 0x5d3b88 Script::ScriptSystem::ExposeDesignBuild::vftable
// 0x43eda0
ScriptReference *TFBMember_System_DesignBuild()
{
	static ScriptSystem::ExposeDesignBuild s;
	return &s;
}

// GLOBAL: 0x5d3be0 Script::ScriptSystem::ExposePlayMode::vftable
// 0x43ede0
ScriptReference *TFBMember_System_PlayMode()
{
	static ScriptSystem::ExposePlayMode s;
	return &s;
}

// GLOBAL: 0x5d3c30 Script::ScriptSystem::ExposeRenderYonIgnore::vftable
// 0x43ee20
ScriptReference *TFBMember_System_RenderYonIgnore()
{
	static ScriptSystem::ExposeRenderYonIgnore s;
	return &s;
}

// GLOBAL: 0x5d3c88 Script::ScriptSystem::ExposeBeenoxQuitGame::vftable
// 0x43ee60
ScriptReference *TFBMember_System_BeenoxQuitGame()
{
	static ScriptSystem::ExposeBeenoxQuitGame s;
	return &s;
}
