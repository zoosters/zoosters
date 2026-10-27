// Measurement defaults and tiny measurement members.
#include "script/ops_message.h"
#include "script/membertable.h"
#include "script/types/scriptcontroller.h"
#include "modules/tfb/camera_views.h"
#include "script/types/twodmeasurement.h"
#include "script/actor.h"
#include "script/types/colormeasurement.h"
#include <math.h>
#include "zoosters/memberrow.h"
#include <float.h>

ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_Value();
ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_Vector();
ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_Position();
ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_Angle();
ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_Color();
Script::ScriptReference *TFBType_Placement();   // 0x42f5c0

extern const float g_floatZero;     // GLOBAL: 0x5ca5a8 g_floatZero
Vec3 *GetZeroVec3();                // 0x4039c0 (shared zero vector)

using namespace Script;

// 0x402b80
float Measurement::GetFloat(ScriptReference *owner)
{
	return (float)GetInt(owner);
}

// 0x402bd0
int ReferenceSet::GetInt(ScriptReference *owner)
{
	return ((ReferenceSet *)owner)->m_count;   // reads the owner, not this (the slot is shared by the set classes)
}

// 0x403460
int Measurement::GetInt(ScriptReference *owner)
{
	return (int)GetFloat(owner);
}

// 0x403480
void Measurement::SetValue(int value, ScriptReference *owner)
{
	SetValue((float)value, owner);
}

// 0x4034c0
ScriptReference *Placement::PlacementSet::GetBaseType()
{
	return m_owner->GetType();
}

// 0x4039b0
float Script::PositionMeasurement::GetFacing()
{
	return g_floatZero;
}

// 0x4039d0
Vec3 *VectorMeasurement::GetVector(ScriptReference *owner)
{
	return GetZeroVec3();
}

namespace Script {
// GLOBAL: 0x5cac50 Script::g_vt_VectorMeasurement_ExposeHorzComponent_1
extern char g_vt_VectorMeasurement_ExposeHorzComponent_1[];
// GLOBAL: 0x5cace0 Script::g_vt_VectorMeasurement_ExposeHorzComponent_2
extern char g_vt_VectorMeasurement_ExposeHorzComponent_2[];
// GLOBAL: 0x5cac98 Script::g_vt_VectorMeasurement_ExposePitch
extern char g_vt_VectorMeasurement_ExposePitch[];
// GLOBAL: 0x5cac08 Script::g_vt_VectorMeasurement_ExposeVertComponent_1
extern char g_vt_VectorMeasurement_ExposeVertComponent_1[];

// 0x403e10
void *VectorMeasurement::GetMembers(int *count)
{
	static zoosters::MemberRow<VectorMeasurement::ExposeHeading, VectorMeasurement::ExposePitch, VectorMeasurement::ExposeHorzComponent, VectorMeasurement::ExposeVertComponent> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5cdfc8 Script::g_vt_PositionMeasurement_ExposeFacing
extern char g_vt_PositionMeasurement_ExposeFacing[];
// GLOBAL: 0x5ce0c0 Script::g_vt_PositionMeasurement_ExposeHeading
extern char g_vt_PositionMeasurement_ExposeHeading[];
// GLOBAL: 0x5ce118 Script::g_vt_PositionMeasurement_ExposePitch
extern char g_vt_PositionMeasurement_ExposePitch[];
// GLOBAL: 0x5ce068 Script::g_vt_PositionMeasurement_ExposeRoll
extern char g_vt_PositionMeasurement_ExposeRoll[];
// GLOBAL: 0x5ce018 Script::g_vt_PositionMeasurement_ExposeTilt
extern char g_vt_PositionMeasurement_ExposeTilt[];
// GLOBAL: 0x5cdec0 Script::g_vt_PositionMeasurement_ExposeViewPyramidDist
extern char g_vt_PositionMeasurement_ExposeViewPyramidDist[];

// 0x430ca0
void *PositionMeasurement::GetMembers(int *count)
{
	static zoosters::MemberRow<PositionMeasurement::ExposeFacing, PositionMeasurement::ExposeTilt, PositionMeasurement::ExposeRoll, PositionMeasurement::ExposeHeading, PositionMeasurement::ExposePitch, VectorMeasurement::ExposeHorzComponent, VectorMeasurement::ExposeVertComponent, PositionMeasurement::ExposeViewPyramidDist> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

// 0x41aa20
float VectorMeasurement::GetFloat(ScriptReference *owner)
{
	return g_floatZero;
}

// 0x41faf0: the largest signed plane distance of the owner's position against the six view-pyramid planes, scaled by 1/12 (units to feet), 0 when inside
// g_camBlk is a copy of the camera's RwFrustumPlane frustumPlanes[6] made by 0x41f190 (m_closest = closestX/Y/Z/pad bytes)

// 0x41faf0
float PositionMeasurement::ExposeViewPyramidDist::GetFloat(ScriptReference *owner)
{
	const Vec3 *p = ((VectorMeasurement *)owner)->GetVector(owner);
	float best = -FLT_MAX;
	const RwFrustumPlaneView *pl = g_camBlk;
	for (int n = 6; n != 0; n--, pl++) {
		float d = pl->normal.x * p->x + pl->normal.y * p->y;
		d += pl->normal.z * p->z;
		d -= pl->distance;
		if (d > best)
			best = d;
	}
	if (best > g_floatZero)
		return best * 0.083333336f;
	return g_floatZero;
}

Script::ScriptReference *TFBType_Int();

namespace Script {

// The owner of the X / Y members is the user variable holding the 2D value (UserReference, value inline in its data word):
// a view of the two halves of m_data, whatever its offset.
struct TwoDVariableView : UserReference {
	short &X() { return ((short *)&m_data)[0]; }
	short &Y() { return ((short *)&m_data)[1]; }
};
#define TWOD_X(o) (((TwoDVariableView *)(o))->X())
#define TWOD_Y(o) (((TwoDVariableView *)(o))->Y())

// 0x43e2a0
int TwoDMeasurement::ExposeX::GetInt(ScriptReference *owner)
{
	return TWOD_X(owner);
}

// 0x43e2b0
void TwoDMeasurement::ExposeX::SetValue(int value, ScriptReference *owner)
{
	TWOD_X(owner) = (short)value;
}

// 0x43e2c0
int TwoDMeasurement::ExposeY::GetInt(ScriptReference *owner)
{
	return TWOD_Y(owner);
}

// 0x43e2d0
void TwoDMeasurement::ExposeY::SetValue(int value, ScriptReference *owner)
{
	TWOD_Y(owner) = (short)value;
}

// 0x43e220
int ColorMeasurement::ExposeR::GetInt(ScriptReference *owner)
{
	return ((ColorMeasurement *)owner)->m_r;
}

// 0x43e230
void ColorMeasurement::ExposeR::SetValue(int value, ScriptReference *owner)
{
	((ColorMeasurement *)owner)->m_r = (unsigned char)value;
}

// 0x43e240
int ColorMeasurement::ExposeG::GetInt(ScriptReference *owner)
{
	return ((ColorMeasurement *)owner)->m_g;
}

// 0x43e250
void ColorMeasurement::ExposeG::SetValue(int value, ScriptReference *owner)
{
	((ColorMeasurement *)owner)->m_g = (unsigned char)value;
}

// 0x43e260
int ColorMeasurement::ExposeB::GetInt(ScriptReference *owner)
{
	return ((ColorMeasurement *)owner)->m_b;
}

// 0x43e270
void ColorMeasurement::ExposeB::SetValue(int value, ScriptReference *owner)
{
	((ColorMeasurement *)owner)->m_b = (unsigned char)value;
}

// 0x43e280
int ColorMeasurement::ExposeA::GetInt(ScriptReference *owner)
{
	return ((ColorMeasurement *)owner)->m_a;
}

// 0x43e290
void ColorMeasurement::ExposeA::SetValue(int value, ScriptReference *owner)
{
	((ColorMeasurement *)owner)->m_a = (unsigned char)value;
}

// 0x430350
float PositionMeasurement::ExposeFacing::GetFloat(ScriptReference *owner)
{
	return ((PositionMeasurement *)owner)->GetFacing();
}

// 0x430360
void PositionMeasurement::ExposeFacing::SetValue(float value, ScriptReference *owner)
{
	((PositionMeasurement *)owner)->SetFacing(value, 1);
}

// 0x430390
float PositionMeasurement::ExposeTilt::GetFloat(ScriptReference *owner)
{
	return ((PositionMeasurement *)owner)->GetTilt();
}

// 0x4303a0
void PositionMeasurement::ExposeTilt::SetValue(float value, ScriptReference *owner)
{
	((PositionMeasurement *)owner)->SetTilt(value, (ScriptReference *)1);
}

// 0x4303c0
float PositionMeasurement::ExposeRoll::GetFloat(ScriptReference *owner)
{
	return ((PositionMeasurement *)owner)->GetRoll();
}

// 0x4303d0
void PositionMeasurement::ExposeRoll::SetValue(float value, ScriptReference *owner)
{
	((PositionMeasurement *)owner)->SetRoll(value, (ScriptReference *)1);
}

}  // namespace Script

// 0x436a50
Script::ScriptReference *IntMeasurement_GetType()
{
	return TFBType_Int();
}

namespace Script {

// 0x430130
const char *ExposeGenericValue::GetName()
{
	return "generic value";
}

}  // namespace Script

// fallback free function; Ghidra name: Script::PositionMeasurement::ExposeViewPyramidDist::GetName
// 0x42feb0
const char *ExposeViewPyramidDist_GetName()
{
	return "view pyramid distance";
}

// fallback free function; Ghidra name: Script::PositionMeasurement::ExposeFacing::GetName
// 0x430340
const char *ExposeFacing_GetName()
{
	return "facing";
}

// fallback free function; Ghidra name: Script::PositionMeasurement::ExposeTilt::GetName
// 0x430380
const char *ExposeTilt_GetName()
{
	return "tilt";
}

// 0x430b70
const char *ExposeHeadingFromMe_GetName()
{
	return "heading from me";
}

// 0x430b80
const char *ExposePitchFromMe_GetName()
{
	return "pitch from me";
}

// 0x436920
const char *ExposeR_GetName()
{
	return "red";
}

// 0x436930
const char *ExposeG_GetName()
{
	return "green";
}

// 0x436940
const char *ExposeB_GetName()
{
	return "blue";
}

// 0x436950
const char *ExposeA_GetName()
{
	return "alpha";
}

// 0x402b00
const char *ValueInterface_GetName()
{
	return "value";
}

// 0x402b10
int ValueInterface_GetMax()
{
	return 0x7fffffff;
}

// 0x402b70
Script::ScriptReference *ValueInterface_GetType()
{
	return TFBType_Value();
}

// 0x4034a0
int FloatMeasurement_GetValueType()
{
	return 16;
}

// fallback free function; Ghidra name: Script::VectorMeasurement::GetName
// 0x4034b0
const char *VectorMeasurement_GetName()
{
	return "distance";
}

// 0x403990
const char *PositionMeasurement_GetName()
{
	return "position";
}

// 0x4039a0
Script::ScriptReference *VectorMeasurement_GetType()
{
	return TFBType_Vector();
}

// 0x4039c0
bool __stdcall PositionMeasurement_SetFacing(int a1, int a2)
{
	return false;
}

// 0x4039e0
Script::ScriptReference *Placement_GetType()
{
	return TFBType_Placement();
}

// 0x4039f0
Script::ScriptReference *PositionMeasurement_GetType()
{
	return TFBType_Position();
}

// 0x403a00
bool __stdcall Placement_Control(int a1)
{
	return true;
}

// 0x403a10
intptr_t __stdcall Placement_Spawn(intptr_t a1, int a2)
{
	return 0;
}

// 0x403a20
bool __stdcall Placement_PlayAnimation(int a1)
{
	return false;
}

// 0x403e90
Script::ScriptReference *AngleMeasurement_GetType()
{
	return TFBType_Angle();
}

// 0x403ea0
const char *ExposeHeading_GetName()
{
	return "heading";
}

// 0x403eb0
const char *ExposeHorzComponent_GetName()
{
	return "horizontal distance";
}

// 0x403ec0
const char *ExposeVertComponent_GetName()
{
	return "vertical distance";
}

// fallback free function; Ghidra name: Script::ColorMeasurement::GetType
// 0x407aa0
Script::ScriptReference *ColorMeasurement_GetType()
{
	return TFBType_Color();
}

// 0x407ad0
int ColorMeasurement_GetValueType()
{
	return 32;
}
