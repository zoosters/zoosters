// Script::ScriptSurfaceAnim: name and type thunk of the surface animation script type.
#include "script/types/scriptsurfaceanim.h"

ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_SurfaceAnim();

// 0x43bf40
const char *ScriptSurfaceAnim_GetName()
{
	return "surface anim";
}

// 0x43c210
Script::ScriptReference *ScriptSurfaceAnim_GetType()
{
	return TFBType_SurfaceAnim();
}
