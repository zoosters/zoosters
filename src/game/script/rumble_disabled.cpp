#include "script/script.h"
#include "script/actor.h"
#include "script/types/scriptcontroller.h"

namespace Script {


}

using namespace Script;

ScriptReference *TFBType_Rumble();   // 0x439770

// GLOBAL: 0x5d2fe8 Script::ScriptController::ScriptRumble::ExposeRumbleDisabled::vftable
// 0x43c0c0
ScriptController::ScriptRumble::ExposeRumbleDisabled::ExposeRumbleDisabled()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Rumble(), 0, 0);
	SetValue(0, this);
}

// 0x43c190
bool ScriptController_IsRumbleDisabled()
{
	static ScriptController::ScriptRumble::ExposeRumbleDisabled s;
	return s.GetInt(&s) != 0;
}
