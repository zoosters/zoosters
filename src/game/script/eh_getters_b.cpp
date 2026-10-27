// Prototype getters whose constructor is an out-of-line call. The classes and their constructors live in
// eh_ctors_b.cpp / eh_ctors_b.h; a getter that calls its constructor must not share the TU of a defined (inlinable) one.
#include "script/types/scriptsets.h"
#include "script/value.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptmessage.h"

using namespace Script;

// 0x437a20
ReferenceSet *TFB_GetPlayerSet()
{
	static ExposePlayerSet s;
	return &s;
}

// 0x437b00
ReferenceSet *TFB_GetNPCSet()
{
	static ExposeNPCsSet s;
	return &s;
}

// 0x437be0
ReferenceSet *TFB_GetInactiveSet()
{
	static ExposeInactiveSet s;
	return &s;
}

// 0x437ce0
ReferenceSet *TFB_GetDeadSet()
{
	static ExposeDeadSet s;
	return &s;
}

// 0x439da0
ScriptReference *TFBMember_HitWallMessage()
{
	static ExposeHitWallMessage s;
	return &s;
}

// 0x439e60
ScriptReference *TFBMember_HitTriggerMessage()
{
	static ExposeHitTriggerMessage s;
	return &s;
}

// 0x439f20
ScriptReference *TFBMember_WallNormalMessage()
{
	static ExposeWallNormalMessage s;
	return &s;
}

// 0x439fe0
ScriptReference *TFBMember_LedgeNearbyMessage()
{
	static ExposeLedgeNearbyMessage s;
	return &s;
}

// 0x43a0a0
ScriptReference *TFBMember_FootPrintMessage()
{
	static ExposeFootPrintMessage s;
	return &s;
}

// 0x43a160
ScriptReference *TFBMember_BounceNormalMessage()
{
	static ExposeBounceNormalMessage s;
	return &s;
}

// 0x43a220
ScriptReference *TFBMember_HitGroundMessage()
{
	static ExposeHitGroundMessage s;
	return &s;
}

// 0x43a2e0
ScriptReference *TFBMember_LeftGroundMessage()
{
	static ExposeLeftGroundMessage s;
	return &s;
}
