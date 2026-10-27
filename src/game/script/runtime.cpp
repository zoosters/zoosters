#include "script/runtime.h"
#include "script/actor.h"
#include "script/value.h"
#include "script/ops_message.h"
#include <math.h>
#include <stddef.h>

Script::ScriptReference *TFBType_Vector();   // 0x42fe70
bool HeadingFromXZ(float x, float z, float *heading);          // 0x4404c0 (script/bearings.h)
bool PitchFromXYZ(float x, float y, float z, float *pitch);   // 0x440520

extern "C" int __cdecl rand(void);

// GLOBAL: 0x5cc9ac g_randScale
extern const float g_randScale;  // 1 / 32767: scales rand() to [0, 1]

namespace Script {

// 0x440450
void ScriptValue::ApplyTo(ScriptRef *target)
{
	ScriptReference *owner;
	ScriptReference *set;
	Measurement *m = (Measurement *)target->Resolve(&owner, &set, true);
	if (m != 0) {
		union {
			int i;
			float f;
		} r;
		switch (Evaluate(&r.f, m->GetMax())) {
		case 0:
			m->SetValue(r.i, owner);
			break;
		case 1:
			m->SetValue(r.f, owner);
			break;
		}
	}
}

// 0x430160
bool ScriptValue::EvalInt(int *out)
{
	int r;
	int kind = Evaluate((float *)&r, ((ValueInterface *)TFBType_Int())->GetMax());
	switch (kind) {
	case -1:
		return false;
	case 0:
		*out = r;
		return true;
	case 1:
		*out = (int)*(float *)&r;
		return true;
	}
	return true;  // the game returns true without storing for any other kind (Evaluate only gives -1/0/1)
}

// 0x4301c0
bool ScriptValue::EvalFloat(float *out)
{
	float r;
	int kind = Evaluate(&r, ((ValueInterface *)TFBType_Int())->GetMax());
	switch (kind) {
	case -1:
		return false;
	case 0:
		*out = (float)*(int *)&r;
		return true;
	case 1:
		*out = r;
		return true;
	}
	return true;
}

// 0x4400c0
int ScriptValue::Evaluate(float *out, int max)
{
	switch (kind) {
	case 0:
	case 0x20:
	case 0x30:
		*(unsigned *)out = lhs.bits;
		return 0;
	case 1:
		*(int *)out = (int)((float)rand() * (int)lhs.bits * g_randScale + 0.5f);
		return 0;
	case 0x10:
		*(unsigned *)out = lhs.bits;
		return 1;
	case 0x11:
		*out = (float)rand() * *(float *)&lhs.bits * g_randScale;
		return 1;
	}
	ScriptReference *owner;
	Measurement *lhsMeas;
	{
		ScriptReference *set;
		lhsMeas = (Measurement *)lhs.Resolve(&owner, &set, true);
	}
	if (lhsMeas == 0) {
		return -1;
	}
	ScriptReference *owner2;
	ScriptReference *set2;
	Measurement *rhsMeas;
	char rhsFloat;
	if (rhsKind == 2) {
		rhsMeas = (Measurement *)rhs.Resolve(&owner2, &set2, true);
		if (rhsMeas == 0) {
			return -1;
		}
		rhsFloat = rhsMeas->GetValueType() == 0x10;
	} else {
		rhsFloat = (rhsKind & 0xf0) == 0x10;
	}
	char lhsFloat = lhsMeas->GetValueType() == 0x10;
	if (!(lhsFloat | rhsFloat)) {
		int x = lhsMeas->GetInt(owner);
		int y;
		switch (rhsKind) {
		case 0:
			y = rhs.bits;
			break;
		case 1:
			y = (int)((float)rand() * (int)rhs.bits * g_randScale + 0.5f);
			break;
		default:
			y = rhsMeas->GetInt(owner2);
			break;
		}
		switch (op) {
		case 0:
			*(int *)out = y + x;
			break;
		case 1:
			*(int *)out = x - y;
			break;
		case 2:
			*(int *)out = y * x;
			break;
		case 3:
			if (y == 0) {
				return -1;
			}
			*(int *)out = x / y;
			break;
		}
		if (max != 0) {
			if (*(int *)out < 0) {
				*(int *)out = 0;
				return 0;
			}
			if (*(int *)out > max) {
				*(int *)out = max;
			}
		}
		return 0;
	}
	float fa = lhsMeas->GetFloat(owner);
	float fb;
	switch (rhsKind) {
	case 0:
		fb = (float)(int)rhs.bits;
		break;
	case 0x10:
		fb = *(float *)&rhs.bits;
		break;
	case 1:
		fb = (float)rand() * (int)rhs.bits * g_randScale;
		break;
	case 0x11:
		fb = (float)rand() * *(float *)&rhs.bits * g_randScale;
		break;
	default:
		fb = rhsMeas->GetFloat(owner2);
		break;
	}
	switch (op) {
	case 0:
		*out = fb + fa;
		break;
	case 1:
		*out = fa - fb;
		break;
	case 2:
		*out = fb * fa;
		break;
	case 3:
		if (fb == 0.0f) {
			return -1;
		}
		*out = fa / fb;
		break;
	}
	if (max != 0) {
		if (*out < 0.0f) {
			*out = 0.0f;
			return 1;
		}
		if ((float)max < *out) {
			*out = (float)max;
		}
	}
	return 1;
}

// GLOBAL: 0x621f14 g_scriptSelf
// GLOBAL: 0x621f18 g_scriptOwner
// GLOBAL: 0x62239c ?g_zeroVec3@Script@@3PAMA
extern float g_zeroVec3[3];              // 0x62239c

// 0x440570 Script::ScriptValue::EvalAngle
char ScriptValue::EvalAngle(void *outp, bool pitch)
{
	float *out = (float *)outp;
	switch (kind) {
	case 0:
		*out = (float)(int)lhs.bits;
		return 1;
	case 1:
		*out = (float)rand() * (int)lhs.bits * g_randScale;
		return 1;
	case 0x10:
		*(unsigned *)out = lhs.bits;
		return 1;
	case 0x11:
		*out = (float)rand() * *(float *)&lhs.bits * g_randScale;
		return 1;
	}
	ScriptReference *owner;
	ScriptReference *set;
	Measurement *lhsMeas = (Measurement *)lhs.Resolve(&owner, &set, true);
	if (lhsMeas == 0)
		return 0;
	float rhsValue;
	switch (rhsKind) {
	case 0:
		rhsValue = (float)(int)rhs.bits;
		break;
	case 1:
		rhsValue = (float)rand() * (int)rhs.bits * g_randScale;
		break;
	case 0x10:
		*(unsigned *)&rhsValue = rhs.bits;
		break;
	case 0x11:
		rhsValue = (float)rand() * *(float *)&rhs.bits * g_randScale;
		break;
	default: {
		ScriptReference *owner2;
		ScriptReference *set2;
		Measurement *rhsMeas = (Measurement *)rhs.Resolve(&owner2, &set2, true);
		if (rhsMeas == 0)
			return 0;
		rhsValue = rhsMeas->GetFloat(owner2);
		break;
	}
	}
	float lhsValue;
	if (!lhsMeas->IsKindOf(TFBType_Vector())) {
		lhsValue = lhsMeas->GetFloat(owner);
	} else {
		ScriptReference *selfOwner = g_scriptOwner;
		ScriptReference *self = g_scriptSelf;
		bool usedSelf = false;
		VectorMeasurement *lhsVec = (VectorMeasurement *)lhsMeas;
		float *pos = (float *)lhsVec->GetVector(owner);
		float *from;
		if (!lhsMeas->IsKindOf(TFBType_Position()))
			from = g_zeroVec3;
		else if (!self->IsKindOf(TFBType_Position()))
			from = pos;
		else {
			usedSelf = true;
			from = (float *)((PositionMeasurement *)self)->GetVector(selfOwner);
		}
		float dz = pos[2] - from[2];
		float angle;
		if (pitch) {
			PitchFromXYZ(pos[0] - from[0], pos[1] - from[1], dz, &angle);
			lhsValue = angle;
		} else if (HeadingFromXZ(pos[0] - from[0], dz, &angle)) {
			lhsValue = angle;
		} else if (usedSelf) {
			lhsValue = ((PositionMeasurement *)self)->GetFacing();
		} else {
			lhsValue = 0.0f;
		}
	}
	switch (op) {
	case 0:
		*out = lhsValue + rhsValue;
		return 1;
	case 1:
		*out = lhsValue - rhsValue;
		return 1;
	}
	return 1;
}

}  // namespace Script

// GLOBAL: 0x620168 g_nodeFreeList

// The "Global Placement Removed" / "Level Placement Removed" variables (function-local statics, placementmessages.cpp); SetScriptContext
// stores the placement id into their slot +0x20 (UserReference::m_index).
Script::ScriptReference *TFBUser_GlobalPlacementRemoved();   // 0x42f370
Script::ScriptReference *TFBUser_LevelPlacementRemoved();    // 0x42f3f0

namespace Script {

// 0x43bb70
void SetScriptContext(ScriptReference *self, ScriptReference *owner)
{
	g_scriptOwner = owner;
	g_scriptSelf = self;
	if (self != 0) {
		if (self->IsKindOf(TFBType_Placement())) {
			// The game reads the int at +4 of anything whose type IsKindOf the placement type, a Placement (m_id) or a "user" variable of
			// that type (UserReference: there it is m_refCount; UBSan reports the latter at run time). Read at Placement::m_id's offset (+4 on
			// x86, +8 on x64), which UserReference::m_refCount shares, without claiming the object is a Placement.
			static_assert(offsetof(Placement, m_id) == offsetof(UserReference, m_refCount), "SetScriptContext reads a UserReference's m_refCount as the id");
			int id = *(const int *)((const char *)self + offsetof(Placement, m_id));
			if (id >= 0) {
				((Script::UserReference *)TFBUser_LevelPlacementRemoved())->m_index = id % 800;
				((Script::UserReference *)TFBUser_GlobalPlacementRemoved())->m_index = id;
				return;
			}
		}
		((Script::UserReference *)TFBUser_LevelPlacementRemoved())->m_index = -1;
		((Script::UserReference *)TFBUser_GlobalPlacementRemoved())->m_index = -1;
	}
}

}  // namespace Script

// 0x439cf0
void *FreeListAlloc(size_t n)
{
	FreeNode *p = g_nodeFreeList;
	if (p != 0) {
		g_nodeFreeList = p->m_next;
		return p;
	}
	return MemAlloc(n);
}
