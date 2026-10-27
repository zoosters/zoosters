// script system symbol registration (0x43f890).
#include "script/script.h"
#include "script/types/scriptsystem.h"
using namespace Script;

namespace Script {
ScriptReference *ScriptSystem_RegisterMembers();   // 0x43f890
}

ScriptReference *TFBType_System();       // 0x43fa20
ScriptReference *TFBUser_LevelPlacementRemoved();       // 0x42f3f0
bool TFBMember_System_HardwareMetrics();                   // 0x43e800
ScriptReference *TFBMember_System_NearClip();   // 0x43eba0
ScriptReference *TFBMember_System_FarClip();   // 0x43ebe0
ScriptReference *TFBMember_System_RadarControl();   // 0x43ec20
ScriptReference *TFBMember_System_CutSceneActive();   // 0x43ec60
ScriptReference *TFBMember_System_Console();   // 0x43eca0
ScriptReference *TFBMember_System_VSyncFlipCount();   // 0x43ece0
ScriptReference *TFBMember_System_LevelZone();   // 0x43ed20
ScriptReference *TFBMember_System_CurrentLevel();   // 0x43ed60
ScriptReference *TFBMember_System_DesignBuild();   // 0x43eda0
ScriptReference *TFBMember_System_PlayMode();   // 0x43ede0
ScriptReference *TFBMember_System_RenderYonIgnore();   // 0x43ee20
ScriptReference *TFBMember_System_BeenoxQuitGame();   // 0x43ee60
ScriptReference *TFBType_MathOps();   // 0x43f610
ScriptReference *TFBType_Shadow();   // 0x43f680
ScriptReference *TFBType_Fog();   // 0x43f6f0
ScriptReference *TFBType_MotionBlur();   // 0x43f760
ScriptReference *TFBType_MemCard();   // 0x43f7d0

#define W3B_REG(f) (((ScriptReference *)(f)())->ResolveSymbol(TFBType_System(), 0, 0))

// 0x43f890
ScriptReference *Script::ScriptSystem_RegisterMembers()
{
	W3B_REG(TFBMember_System_NearClip);
	W3B_REG(TFBMember_System_FarClip);
	W3B_REG(TFBMember_System_RadarControl);
	W3B_REG(TFBMember_System_CutSceneActive);
	W3B_REG(TFBMember_System_Console);
	W3B_REG(TFBMember_System_VSyncFlipCount);
	W3B_REG(TFBMember_System_LevelZone);
	W3B_REG(TFBMember_System_CurrentLevel);
	W3B_REG(TFBMember_System_DesignBuild);
	W3B_REG(TFBMember_System_PlayMode);
	W3B_REG(TFBMember_System_RenderYonIgnore);
	W3B_REG(TFBMember_System_BeenoxQuitGame);
	ScriptSystem::GetScriptTimePtr();
	TFBMember_System_HardwareMetrics();
	W3B_REG(TFBType_MathOps);
	W3B_REG(TFBType_Shadow);
	W3B_REG(TFBType_Fog);
	W3B_REG(TFBType_MotionBlur);
	W3B_REG(TFBType_MemCard);
	return TFBUser_LevelPlacementRemoved();
}
