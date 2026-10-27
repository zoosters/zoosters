// Script::ScriptSurfaceAnim (name taken from the GetName string "surface anim"; no PS2 RTTI row), the script type of the
// animation / material data objects: AnimTimer (vtable 0x5d56fc) and AnimNodeList (0x5d56d0) derive from it (h/anim.h). No data
// members: the game only builds its prototype (TFBType_SurfaceAnim 0x43c130). Vtable 0x5d2f30 (11 slots), next to
// Script::UserReference 0x5d2f70: 0 GetName, 4 GetType, 7 GetMembers below; 2 is the default 0x416530, 3 the shared deleting
// destructor 0x4314a0, 5 0x43e6f0, 6 the object default 0x42cbe0, 10 0x464ef0 (Fold_Nop).
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTSURFACEANIM_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTSURFACEANIM_H

#include "zoosters/base.h"
#include "script/script.h"

namespace Script {

// GLOBAL: 0x5d2f30 Script::ScriptSurfaceAnim::vftable
class ScriptSurfaceAnim : public ScriptReference { // "surface anim"
public:
	const char *GetName();         // 0: 0x43bf40
	ScriptReference *GetType();    // 4: 0x43c210 (thunk to the prototype getter TFBType_SurfaceAnim)
	void *GetMembers(int *count);  // 7: 0x43c220
};
ZOOSTERS_ASSERT_SIZE(ScriptSurfaceAnim, 4);

} // namespace Script

#endif
