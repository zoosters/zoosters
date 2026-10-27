// Prototype getter, next to Script::UserReference.
// 0x5d2f70) has no known name; name "ScriptSurfaceAnim" taken from its GetName string "surface anim".
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptsurfaceanim.h"
using namespace Script;


// GLOBAL: 0x5d2f30 Script::ScriptSurfaceAnim::vftable
// 0x43c130
ScriptReference *TFBType_SurfaceAnim()
{
	static ScriptSurfaceAnim s;
	return &s;
}
