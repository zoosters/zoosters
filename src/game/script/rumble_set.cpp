#include "script/actor.h"

// The set of ScriptRumble objects (function-local static ReferenceSet at 0x6179a8; HandleEvents walks its node list)
// 0x4034d0
Script::ReferenceSet *RumbleSet_Get()
{
	static Script::ReferenceSet s_rumbleSet;
	return &s_rumbleSet;
}
