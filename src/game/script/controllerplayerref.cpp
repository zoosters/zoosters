#include "script/actor.h"
#include "script/types/scriptactor.h"
#include "script/types/scriptcontroller.h"
using namespace Script;

// The five player-reference objects (index = controller number; the last is "no controller")
// 0x439460
ControllerPlayerRef *TFB_GetPlayerRefs()
{
	static ControllerPlayerRef refs[5];
	return refs;
}

// 0x4395b0
const char *ControllerPlayerRef::GetName()
{
	const char *names[5] = { "controller 1", "controller 2", "controller 3", "controller 4", "no controller" };
	return names[this - TFB_GetPlayerRefs()];
}
