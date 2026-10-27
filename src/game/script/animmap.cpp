#include "script/actor.h"
#include "script/types/animmap.h"
#include "script/bearings.h"
#include "script/runtime.h"
#include "script/membertable.h"
#include <math.h>
#include "zoosters/memberrow.h"

namespace Script {
// GLOBAL: 0x5cef88 Script::g_vt_ExposeAnimIndex
extern char g_vt_ExposeAnimIndex[];

// 0x436440
void *AnimMapMeasurement::GetMembers(int *count)
{
	static zoosters::MemberRow<ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex, ExposeAnimIndex> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

// GLOBAL: 0x621f14 g_scriptSelf

namespace Script {

// 0x4351e0
int AnimMapMeasurement::GetInt(ScriptReference *owner)
{
	Placement *self = (Placement *)g_scriptSelf;
	if (self->IsKindOf(TFBType_Placement())) {
		self = (Placement *)g_scriptSelf;
		return self->AccessBehavior(-1, -1);
	}
	return 0;
}

// 0x4385c0
int ExposeAnimIndex::GetInt(ScriptReference *owner)
{
	Placement *self = (Placement *)g_scriptSelf;
	if (self->IsKindOf(TFBType_Placement())) {
		self = (Placement *)g_scriptSelf;
		int n;
		ScriptReference *p = (ScriptReference *)TFBType_AnimMap()->GetMembers(&n);
		n = 0;
		while (this != p) {
			p++;
			n++;
		}
		return self->AccessBehavior(n, -1);
	}
	return 0;
}

// 0x438630
void ExposeAnimIndex::SetValue(int value, ScriptReference *owner)
{
	Placement *self = (Placement *)g_scriptSelf;
	if (self->IsKindOf(TFBType_Actor())) {
		self = (Placement *)g_scriptSelf;
		int n;
		ScriptReference *p = (ScriptReference *)TFBType_AnimMap()->GetMembers(&n);
		n = 0;
		while (this != p) {
			p++;
			n++;
		}
		self->AccessBehavior(n, value);
	}
}

}  // namespace Script

// 0x4386a0
int AnimMapMemberCount()
{
	int n;
	TFBType_AnimMap()->GetMembers(&n);
	return n;
}

namespace Script {

// 0x438900
ScriptReference *AnimMapMeasurement::GetType()
{
	return TFBType_AnimMap();
}

}  // namespace Script


namespace Script {

// 0x436a30
const char *AnimMapMeasurement::GetName()
{
	return "animation mapping";
}

}  // namespace Script
