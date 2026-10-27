#include "script/types/scriptcontroller.h"
#include "script/types/scriptsets.h"
#include <math.h>

using namespace Script;

// 0x4025b0
Script::ScriptReference *Script::ScriptReference::Deref(Script::ScriptReference *, int)
{
	return this;
}

// 0x402bc0
void ReferenceSet::AssignOrdered(ScriptReference *obj, int unused)
{
	Assign(obj, 0);
}

namespace Script {

// 0x43e6f0
const char *ScriptReference::GetName()
{
	return 0;
}

// 0x436a40
const char *NamedObject::GetAltName()
{
	return GetName();
}

}  // namespace Script


ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_Set();

namespace Script {

// 0x437a10
const char *ExposePlayerSet::GetName()
{
	return "players";
}

// 0x437af0
const char *ExposeNPCsSet::GetName()
{
	return "NPCs";
}

// 0x437cb0
const char *ExposeDeadSet::GetName()
{
	return "dead";
}

// 0x43aa50
const char *ExposeSendersSet::GetName()
{
	return "message senders";
}

}  // namespace Script

// 0x437bd0
const char *ExposeInactiveSet_GetName()
{
	return "inactive";
}

// 0x43e470
const char *ExposeActiveSet_GetName()
{
	return "active";
}

// 0x43e510
const char *ExposeRemovedSet_GetName()
{
	return "removed";
}

// 0x402ba0
const char *ReferenceSet_GetName()
{
	return "set";
}

// 0x402bb0
Script::ScriptReference *ReferenceSet_GetType()
{
	return TFBType_Set();
}


// 0x402b20
ReferenceSet::~ReferenceSet()
{
	Clear();
}

// 0x403bb0
ScriptReference::ScriptReference(const ScriptReference &o)
{
	*(void **)this = *(void **)&o;
}
