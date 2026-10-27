// VectorMeasurement bearings: heading, pitch, horizontal and vertical distance.
#include <math.h>
#include "script/bearings.h"
#include "script/runtime.h"
#include "script/actor.h"

// GLOBAL: 0x621f14 g_scriptSelf
// GLOBAL: 0x621f18 g_scriptOwner
// GLOBAL: 0x5ca5a8 g_floatZero

// 0x4404c0
bool HeadingFromXZ(float x, float z, float *heading)
{
	if (fabsf(x) < 0.0001f && fabsf(z) < 0.0001f)
		return false;
	float h = (float)atan2(x, z) * 57.295776f;
	*heading = h;
	if (h < 0.0f)
		*heading = h + 360.0f;
	return true;
}

// 0x440520
bool PitchFromXYZ(float x, float y, float z, float *pitch)
{
	if (fabsf(y) < 0.0001f) {
		*pitch = 0.0f;
		return true;
	}
	*pitch = (float)atan2(y, sqrt(x * x + z * z)) * 57.295776f;
	return true;
}


namespace Script {

// 0x440820
float VectorMeasurement::ExposeHeading::GetFloat(ScriptReference *owner)
{
	ScriptReference *other = g_scriptOwner;
	ScriptReference *self = g_scriptSelf;
	if (!self->IsKindOf(TFBType_Position()))
		return g_floatZero;
	Vec3 *a = ((VectorMeasurement *)owner)->GetVector(owner);
	Vec3 *b = ((VectorMeasurement *)self)->GetVector(other);
	float *heading = (float *)&owner;
	if (!HeadingFromXZ(a->x - b->x, a->z - b->z, heading))
		return ((PositionMeasurement *)self)->GetFacing();
	return *heading;
}

// 0x4408a0
float VectorMeasurement::ExposePitch::GetFloat(ScriptReference *owner)
{
	ScriptReference *other = g_scriptOwner;
	ScriptReference *self = g_scriptSelf;
	if (!self->IsKindOf(TFBType_Position()))
		return g_floatZero;
	Vec3 *a = ((VectorMeasurement *)owner)->GetVector(owner);
	Vec3 *b = ((VectorMeasurement *)self)->GetVector(other);
	float *pitch = (float *)&owner;
	PitchFromXYZ(a->x - b->x, a->y - b->y, a->z - b->z, pitch);
	return *pitch;
}

// 0x440920
float VectorMeasurement::ExposeHorzComponent::GetFloat(ScriptReference *owner)
{
	ScriptReference *other = g_scriptOwner;
	ScriptReference *self = g_scriptSelf;
	if (!self->IsKindOf(TFBType_Position()))
		return g_floatZero;
	Vec3 *a = ((VectorMeasurement *)owner)->GetVector(owner);
	Vec3 *b = ((VectorMeasurement *)self)->GetVector(other);
	float dx = a->x - b->x;
	float dz = a->z - b->z;
	return (float)sqrt(dx * dx + dz * dz) * (1.0f / 12.0f);
}

// 0x440990
float VectorMeasurement::ExposeVertComponent::GetFloat(ScriptReference *owner)
{
	ScriptReference *other = g_scriptOwner;
	ScriptReference *self = g_scriptSelf;
	if (!self->IsKindOf(TFBType_Position()))
		return g_floatZero;
	Vec3 *a = ((VectorMeasurement *)owner)->GetVector(owner);
	Vec3 *b = ((VectorMeasurement *)self)->GetVector(other);
	return fabsf(a->y - b->y) * (1.0f / 12.0f);
}

}  // namespace Script
