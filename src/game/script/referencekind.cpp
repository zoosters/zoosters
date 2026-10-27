#include "script/script.h"
#include "script/runtime.h"

using namespace Script;

// 0x43bab0
int ScriptReference::IsKindOf(ScriptReference *type)
{
	ScriptReference *wantType = type->GetType();
	ScriptReference *ownType = GetType();
	ScriptReference *cur = this;   // walk starts at `this` unless this is a Set: then Int (a Set matches a Set directly)
	if (ownType == TFBType_Set()) {
		if (wantType == TFBType_Set())
			return 1;
		cur = TFBType_Int();
	}
	int res = 1;
	do {
		if (cur->GetType() == wantType)
			return res;
		res = -1;
		cur = cur->GetBaseType();
	} while (cur);
	return 0;
}

// "name::type[::othertype]" into a shared 0x100-byte buffer
#include <stdio.h>
extern char g_typedNameBuf[];   // GLOBAL: 0x61f8a8 g_typedNameBuf
Script::ScriptReference *TFBType_OpCode();     // 0x4310c0
Script::ScriptReference *TFBType_Behavior();   // 0x4318f0

namespace Script {
// 0x432af0
const char *ScriptReference::FormatTypedName(const char *name, ScriptReference *other)
{
	char suffix[0x100];
	suffix[0] = 0;
	if (other && GetType() != other->GetType())
		sprintf(suffix, "::%s", other->GetType()->GetAltName());
	if (!name) {
		if (IsKindOf(TFBType_OpCode()) && GetType() != TFBType_Behavior())
			name = GetAltName();
		else
			name = GetName();
	}
	sprintf(g_typedNameBuf, "%s::%s%s", name, GetType()->GetAltName(), suffix);
	return g_typedNameBuf;
}
}  // namespace Script
