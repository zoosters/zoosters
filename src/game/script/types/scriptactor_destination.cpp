#include <math.h>
#include "script/actor.h"
#include "script/bearings.h"
#include "modules/tfb/gameactor.h"

namespace Script {

// The owner handed to the actor accessors of this file is a game actor.
static inline RWS::TFB::GameActor *Actor(ScriptReference *owner)
{
	return static_cast<RWS::TFB::GameActor *>(owner);
}


// 0x42ac50
float ScriptActor::ExposeOrigin::ExposeHeading::GetFloat(ScriptReference *owner)
{
	ScriptReference *s = g_scriptSelf;
	if (!s->IsKindOf(TFBType_Position()))
		return g_floatZero;
	s = g_scriptSelf;
	Vec3 *pt = &Actor(owner)->m_origin;
	Vec3 *selfPos = ((VectorMeasurement *)s)->GetVector(s);
	float h;
	float *heading = &h;
	if (!HeadingFromXZ(pt->x - selfPos->x, pt->z - selfPos->z, heading))
		return ((PositionMeasurement *)s)->GetFacing();
	return *heading;
}

// 0x42acd0
float ScriptActor::ExposeOrigin::ExposePitch::GetFloat(ScriptReference *owner)
{
	ScriptReference *s = g_scriptSelf;
	if (!s->IsKindOf(TFBType_Position()))
		return g_floatZero;
	Vec3 *pt = &Actor(owner)->m_origin;
	Vec3 *selfPos = ((VectorMeasurement *)g_scriptSelf)->GetVector(g_scriptSelf);
	float p;
	float *pitch = &p;
	PitchFromXYZ(pt->x - selfPos->x, pt->y - selfPos->y, pt->z - selfPos->z, pitch);
	return *pitch;
}

// 0x42ada0
float ScriptActor::ExposeOrigin::ExposeHorzComponent::GetFloat(ScriptReference *owner)
{
	ScriptReference *s = g_scriptSelf;
	if (!s->IsKindOf(TFBType_Position()))
		return g_floatZero;
	Vec3 *pt = &Actor(owner)->m_origin;
	Vec3 *selfPos = ((VectorMeasurement *)g_scriptSelf)->GetVector(g_scriptSelf);
	float dx = pt->x - selfPos->x;
	float dz = pt->z - selfPos->z;
	return (float)sqrt(dx * dx + dz * dz) * (1.0f / 12.0f);
}

// 0x42ae10
void ScriptActor::ExposeDestination::SetPosition(const Vec3 *v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	const int *src = (const int *)v;
	int *dst = (int *)&a->m_destination;
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

// 0x42ae40
float ScriptActor::ExposeDestination::ExposeHeading::GetFloat(ScriptReference *owner)
{
	ScriptReference *s = g_scriptSelf;
	if (!s->IsKindOf(TFBType_Position()))
		return g_floatZero;
	s = g_scriptSelf;
	Vec3 *pt = &Actor(owner)->m_destination;
	Vec3 *selfPos = ((VectorMeasurement *)s)->GetVector(s);
	float h;
	float *heading = &h;
	if (!HeadingFromXZ(pt->x - selfPos->x, pt->z - selfPos->z, heading))
		return ((PositionMeasurement *)s)->GetFacing();
	return *heading;
}

// 0x42aec0
float ScriptActor::ExposeDestination::ExposePitch::GetFloat(ScriptReference *owner)
{
	ScriptReference *s = g_scriptSelf;
	if (!s->IsKindOf(TFBType_Position()))
		return g_floatZero;
	Vec3 *pt = &Actor(owner)->m_destination;
	Vec3 *selfPos = ((VectorMeasurement *)g_scriptSelf)->GetVector(g_scriptSelf);
	float p;
	float *pitch = &p;
	PitchFromXYZ(pt->x - selfPos->x, pt->y - selfPos->y, pt->z - selfPos->z, pitch);
	return *pitch;
}

// 0x42af90
float ScriptActor::ExposeDestination::ExposeHorzComponent::GetFloat(ScriptReference *owner)
{
	ScriptReference *s = g_scriptSelf;
	if (!s->IsKindOf(TFBType_Position()))
		return g_floatZero;
	Vec3 *pt = &Actor(owner)->m_destination;
	Vec3 *selfPos = ((VectorMeasurement *)g_scriptSelf)->GetVector(g_scriptSelf);
	float dx = pt->x - selfPos->x;
	float dz = pt->z - selfPos->z;
	return (float)sqrt(dx * dx + dz * dz) * (1.0f / 12.0f);
}

}  // namespace Script
