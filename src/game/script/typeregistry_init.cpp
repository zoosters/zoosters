// Script type registry init (0x43c870) and reset (0x43c9b0).
#include "script/script.h"
#include "script/ops_message.h"
#include "script/typeinit.h"
using namespace Script;

ScriptReference *TFBType_Value();       // 0x42f900
ScriptReference *TFBType_Angle();       // 0x4302c0
ScriptReference *TFBType_Color();       // 0x42fdf0
ScriptReference *TFBType_TwoD();        // 0x42fe30
ScriptReference *TFBType_Set();         // 0x43d2d0
ScriptReference *TFBType_User();        // 0x43c530
ScriptReference *TFBType_OpCode();      // 0x4310c0
ScriptReference *TFBType_Behavior();    // 0x4318f0
ScriptReference *TFBType_Actor();       // 0x436e30
ScriptReference *TFBType_Message();     // 0x43ad50
ScriptReference *TFBType_Sound();        // 0x43db70
ScriptReference *TFBType_Camera();      // 0x4389d0
ScriptReference *TFBType_Controller();  // 0x4393d0
ScriptReference *TFBType_Sprite();        // 0x43e330
ScriptReference *TFBType_Light();        // 0x43c490
ScriptReference *TFBType_Particle();        // 0x43b4f0
ScriptReference *TFBType_SurfaceAnim();        // 0x43c130
ScriptReference *TFBType_System();        // 0x43fa20
ScriptReference *TFBMember_System_NextLevel();        // 0x43c410
ScriptReference *TFBMember_System_GameTime();        // 0x43c450
ScriptReference *TFBUser_GlobalPlacementRemoved();        // 0x42f370
int ScriptSound_GetEffectsVolume();                       // 0x43dee0
int ScriptSound_GetMusicVolume();                       // 0x43df50
bool ScriptController_IsRumbleDisabled();                       // 0x43c190
int SymbolRegistry_GetSize();                     // 0x432cb0
void TFB_RegisterType(ScriptReference *t);  // 0x439bc0
namespace Script {
}

// GLOBAL: 0x621f28 g_typeInitCount
extern int g_typeInitCount;
// GLOBAL: 0x621f24 g_symCountBase
extern int g_symCountBase;

// 0x43c870
TypeInitHost *TypeInitHost::Init()
{
	if (g_typeInitCount++ == 0) {
		TFB_RegisterType(TFBType_Value());
		TFB_RegisterType(TFBType_Angle());
		TFB_RegisterType(TFBType_Color());
		TFB_RegisterType(TFBType_TwoD());
		TFB_RegisterType(TFBType_Set());
		TFB_RegisterType(TFBType_User());
		TFB_RegisterType(TFBType_OpCode());
		TFB_RegisterType(TFBType_Behavior());
		TFB_RegisterType(TFBType_Actor());
		TFB_RegisterType(TFBType_Message());
		TFB_RegisterType(TFBType_Sound());
		TFB_RegisterType(TFBType_Camera());
		TFB_RegisterType(TFBType_Controller());
		TFB_RegisterType(TFBType_Sprite());
		TFB_RegisterType(TFBType_Light());
		TFB_RegisterType(TFBType_Particle());
		TFB_RegisterType(TFBType_SurfaceAnim());
		TFB_RegisterType(TFBType_System());
		ScriptSound_GetEffectsVolume();
		ScriptSound_GetMusicVolume();
		ScriptController_IsRumbleDisabled();
		TFBUser_GlobalPlacementRemoved();
		((ScriptReference *)TFBMember_System_NextLevel())->ResolveSymbol(TFBType_System(), 0, 0);
		((ScriptReference *)TFBMember_System_GameTime())->ResolveSymbol(TFBType_System(), 0, 0);
		g_symCountBase = SymbolRegistry_GetSize();
	}
	return this;
}

#include <vector>
#include <string.h>
std::vector<ScriptReference *> *TFB_GetTypeList();   // 0x43c810
void SymbolRegistry_ReleaseFrom(int base);                      // 0x433ec0
ScriptReference *TFBUser_LevelPlacementRemoved();                     // 0x42f3f0

// 0x43c9b0
void TypeRegistry_ResetAll(int base)
{
	std::vector<ScriptReference *> *v = TFB_GetTypeList();
	for (int i = 0; i < (int)TFB_GetTypeList()->size(); i++)
		(*v)[i]->vf10();
	if (base == -1)
		base = g_symCountBase;
	SymbolRegistry_ReleaseFrom(base);
	unsigned bytes = ((UserReference *)TFBUser_LevelPlacementRemoved())->m_dataSize;
	UserReference *levelRemoved = (UserReference *)TFBUser_LevelPlacementRemoved();
	memset(levelRemoved->m_data, 0, bytes);
}
