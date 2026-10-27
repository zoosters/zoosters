#include "script/membertable.h"
#include "script/types/scriptcontroller.h"
#include <math.h>
#include "framework/core/clinearmemorymanager.h"
#include "script/actor.h"
#include "script/types/twodmeasurement.h"
#include "framework/startup/win32/cfgobj.h"
#include "framework/startup/win32/input/pads.h"
#include "zoosters/memberrow.h"
// GLOBAL: 0x519c90 ?SetOption@CfgObj@@QAEXPBDH@Z


using namespace Script;


// 0x402910
int ScriptController::ExposeControllerUnplugged::GetInt(ScriptReference *owner)
{
	ScriptReference *base = TFB_GetPlayerRefs();
	int idx = (ControllerPlayerRef *)owner - (ControllerPlayerRef *)base;
	return IsWaitingForPad(idx) == 0;
}

// 0x402940
int ScriptController::ScriptRumble::ExposeActuator1Level::GetInt(ScriptReference *owner)
{
	if (owner != TFBType_Rumble())
		return ((ScriptController::ScriptRumble *)owner)->m_actuator1Level;
	return 0;
}

// 0x402960
void ScriptController::ScriptRumble::ExposeActuator1Level::SetValue(int value, ScriptReference *owner)
{
	if (owner != TFBType_Rumble())
		((ScriptController::ScriptRumble *)owner)->m_actuator1Level = value;
}

// 0x402980
int ScriptController::ScriptRumble::ExposeActuator2Level::GetInt(ScriptReference *owner)
{
	if (owner != TFBType_Rumble())
		return ((ScriptController::ScriptRumble *)owner)->m_actuator2Level;
	return 0;
}

// 0x4029a0
void ScriptController::ScriptRumble::ExposeActuator2Level::SetValue(int value, ScriptReference *owner)
{
	if (owner != TFBType_Rumble())
		((ScriptController::ScriptRumble *)owner)->m_actuator2Level = value;
}

// 0x4029c0
float ScriptController::ScriptRumble::ExposeDuration::GetFloat(ScriptReference *owner)
{
	if (owner != TFBType_Rumble())
		return ((ScriptController::ScriptRumble *)owner)->m_duration;
	return 0.0f;
}

// 0x4029e0
void ScriptController::ScriptRumble::ExposeDuration::SetValue(float value, ScriptReference *owner)
{
	if (owner != TFBType_Rumble()) {
		((ScriptController::ScriptRumble *)owner)->m_duration = value;
		*(int *)&((ScriptController::ScriptRumble *)owner)->m_timeRemaining = *(int *)&value;
	}
}

// 0x402a00
float ScriptController::ScriptRumble::ExposeTimeRemaining::GetFloat(ScriptReference *owner)
{
	if (owner != TFBType_Rumble())
		return ((ScriptController::ScriptRumble *)owner)->m_timeRemaining;
	return 0.0f;
}

// 0x402a20
float ScriptController::ScriptRumble::ExposeRadius::GetFloat(ScriptReference *owner)
{
	if (owner != TFBType_Rumble())
		return ((ScriptController::ScriptRumble *)owner)->m_radius;
	return 0.0f;
}

// 0x402a40
void ScriptController::ScriptRumble::ExposeRadius::SetValue(float value, ScriptReference *owner)
{
	if (owner != TFBType_Rumble())
		((ScriptController::ScriptRumble *)owner)->m_radius = value;
}

// 0x402a60
int ScriptController::ScriptRumble::ExposeAttenuation::GetInt(ScriptReference *owner)
{
	if (owner != TFBType_Rumble())
		return ((ScriptController::ScriptRumble *)owner)->m_attenuation;
	return 0;
}

// 0x402a80
void ScriptController::ScriptRumble::ExposeAttenuation::SetValue(int value, ScriptReference *owner)
{
	if (owner != TFBType_Rumble())
		((ScriptController::ScriptRumble *)owner)->m_attenuation = value;
}

// 0x402aa0
int ScriptController::ScriptRumble::ExposeRumbleDisabled::GetInt(ScriptReference *owner)
{
	return g_rumbleOff == 1;
}

// 0x402ac0
void ScriptController::ScriptRumble::ExposeRumbleDisabled::SetValue(int value, ScriptReference *owner)
{
	g_rumbleOff = value != 0;
	if (g_audioDevice)
		g_audioDevice->SetOption("Settings\\Options\\RumbleOff", value);
}

// 0x402e40
int ScriptController::ExposeCircle::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 3, 0.0f) > 0.15f;
}

// 0x402e70
int ScriptController::ExposeTriangle::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 2, 0.0f) > 0.15f;
}

// 0x402ea0
int ScriptController::ExposeX::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 4, 0.0f) > 0.15f;
}

// 0x402ed0
int ScriptController::ExposeSquare::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 5, 0.0f) > 0.15f;
}

// 0x402f00
int ScriptController::ExposeSelect::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 1, 0.0f) > 0.15f;
}

// 0x402f30
int ScriptController::ExposeStart::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 0, 0.0f) > 0.15f;
}

// 0x402f60
int ScriptController::ExposeL1::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 6, 0.0f) > 0.15f;
}

// 0x402f90
int ScriptController::ExposeR1::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 7, 0.0f) > 0.15f;
}

// 0x402fc0
int ScriptController::ExposeL2::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 8, 0.0f) > 0.15f;
}

// 0x402ff0
int ScriptController::ExposeR2::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 9, 0.0f) > 0.15f;
}

// 0x403020
int ScriptController::ExposeDPadL::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 10, 0.0f) > 0.15f;
}

// 0x403050
int ScriptController::ExposeDPadR::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 11, 0.0f) > 0.15f;
}

// 0x403080
int ScriptController::ExposeDPadU::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 12, 0.0f) > 0.15f;
}

// 0x4030b0
int ScriptController::ExposeDPadD::GetInt(ScriptReference *owner)
{
	return GetInputValue((int *)owner, 13, 0.0f) > 0.15f;
}

// 0x4030e0
int ScriptController::ExposeLeftAnalogL::GetInt(ScriptReference *owner)
{
	float f = GetInputValue((int *)owner, 14, 0.0f);
	if (f > 0.0f)
		return (int)(f * 100.0f);
	return 0;
}

// 0x403120
int ScriptController::ExposeLeftAnalogR::GetInt(ScriptReference *owner)
{
	float f = GetInputValue((int *)owner, 14, 0.0f);
	if (f < 0.0f)
		return (int)(f * -100.0f);
	return 0;
}

// 0x403160
int ScriptController::ExposeLeftAnalogU::GetInt(ScriptReference *owner)
{
	float f = GetInputValue((int *)owner, 15, 0.0f);
	if (f > 0.0f)
		return (int)(f * 100.0f);
	return 0;
}

// 0x4031a0
int ScriptController::ExposeLeftAnalogD::GetInt(ScriptReference *owner)
{
	float f = GetInputValue((int *)owner, 15, 0.0f);
	if (f < 0.0f)
		return (int)(f * -100.0f);
	return 0;
}

// 0x4031e0
int ScriptController::ExposeRightAnalogL::GetInt(ScriptReference *owner)
{
	float f = GetInputValue((int *)owner, 17, 0.0f);
	if (f > 0.0f)
		return (int)(f * 100.0f);
	return 0;
}

// 0x403220
int ScriptController::ExposeRightAnalogR::GetInt(ScriptReference *owner)
{
	float f = GetInputValue((int *)owner, 17, 0.0f);
	if (f < 0.0f)
		return (int)(f * -100.0f);
	return 0;
}

// 0x403260
int ScriptController::ExposeRightAnalogU::GetInt(ScriptReference *owner)
{
	float f = GetInputValue((int *)owner, 18, 0.0f);
	if (f > 0.0f)
		return (int)(f * 100.0f);
	return 0;
}

// 0x4032a0
int ScriptController::ExposeRightAnalogD::GetInt(ScriptReference *owner)
{
	float f = GetInputValue((int *)owner, 18, 0.0f);
	if (f < 0.0f)
		return (int)(f * -100.0f);
	return 0;
}

// 0x4032e0
float ScriptController::ExposeLeftAnalogAngle::GetFloat(ScriptReference *owner)
{
	float horz = GetInputValue((int *)owner, 14, 0.0f);
	float vert = GetInputValue((int *)owner, 15, 0.0f);
	if (horz == 0.0f && vert == 0.0f)
		return 0.0f;
	return (float)atan2(horz, vert) * 57.295776f;
}

// `volatile float x = a * a;` reproduces the game: it stores the float-rounded a*a into the dead owner slot before the second call (plain locals keep
// the unrounded product); the later `fadd dword [slot]` is the volatile read.
// 0x403350
float ScriptController::ExposeLeftAnalogMagnitude::GetFloat(ScriptReference *owner)
{
	float a = GetInputValue((int *)owner, 14, 0.0f);
	volatile float x = a * a;
	float y = GetInputValue((int *)owner, 15, 0.0f);
	y *= y;
	float lenSq = y + x;
	if (lenSq >= 1.0f)
		return 1.0f;
	return (float)sqrt(lenSq);
}

// 0x4033a0
float ScriptController::ExposeRightAnalogAngle::GetFloat(ScriptReference *owner)
{
	float horz = GetInputValue((int *)owner, 17, 0.0f);
	float vert = GetInputValue((int *)owner, 18, 0.0f);
	if (horz == 0.0f && vert == 0.0f)
		return 0.0f;
	return (float)atan2(horz, vert) * 57.295776f;
}

// 0x403410
float ScriptController::ExposeRightAnalogMagnitude::GetFloat(ScriptReference *owner)
{
	float a = GetInputValue((int *)owner, 17, 0.0f);
	volatile float x = a * a;
	float y = GetInputValue((int *)owner, 18, 0.0f);
	y *= y;
	float lenSq = y + x;
	if (lenSq >= 1.0f)
		return 1.0f;
	return (float)sqrt(lenSq);
}

// 0x403cb0
int ScriptController::ScriptRumble::AdjustRefCount(int delta)
{
	m_refCount += delta;
	if (m_refCount == 0) {
		delete this;
		return 0;
	}
	return m_refCount;
}

// 0x403d00
void ScriptController::ScriptRumble::SetVector(const Vec3 *v, ScriptReference *owner)
{
	m_position = *v;
}

// 0x403d40
void ScriptController::ScriptRumble::Remove()
{
	if (RumbleSet_Get()->Remove(this)) {
		ScriptReference *owner = m_source;
		if (static_cast<Placement *>(owner)->m_20.Remove(this))
			owner->AdjustRefCount(-1);
	}
}

namespace Script {
// GLOBAL: 0x5d1650 Script::g_vt_ScriptController_ExposeCircle
extern char g_vt_ScriptController_ExposeCircle[];
// GLOBAL: 0x5d1558 Script::g_vt_ScriptController_ExposeControllerUnplugged
extern char g_vt_ScriptController_ExposeControllerUnplugged[];
// GLOBAL: 0x5d1a88 Script::g_vt_ScriptController_ExposeDPadD
extern char g_vt_ScriptController_ExposeDPadD[];
// GLOBAL: 0x5d1998 Script::g_vt_ScriptController_ExposeDPadL
extern char g_vt_ScriptController_ExposeDPadL[];
// GLOBAL: 0x5d19e8 Script::g_vt_ScriptController_ExposeDPadR
extern char g_vt_ScriptController_ExposeDPadR[];
// GLOBAL: 0x5d1a38 Script::g_vt_ScriptController_ExposeDPadU
extern char g_vt_ScriptController_ExposeDPadU[];
// GLOBAL: 0x5d1858 Script::g_vt_ScriptController_ExposeL1
extern char g_vt_ScriptController_ExposeL1[];
// GLOBAL: 0x5d18f8 Script::g_vt_ScriptController_ExposeL2
extern char g_vt_ScriptController_ExposeL2[];
// GLOBAL: 0x5d1ad8 Script::g_vt_ScriptController_ExposeLeftAnalogAngle
extern char g_vt_ScriptController_ExposeLeftAnalogAngle[];
// GLOBAL: 0x5d12f8 Script::g_vt_ScriptController_ExposeLeftAnalogD
extern char g_vt_ScriptController_ExposeLeftAnalogD[];
// GLOBAL: 0x5d11f8 Script::g_vt_ScriptController_ExposeLeftAnalogL
extern char g_vt_ScriptController_ExposeLeftAnalogL[];
// GLOBAL: 0x5d14a8 Script::g_vt_ScriptController_ExposeLeftAnalogMagnitude
extern char g_vt_ScriptController_ExposeLeftAnalogMagnitude[];
// GLOBAL: 0x5d1250 Script::g_vt_ScriptController_ExposeLeftAnalogR
extern char g_vt_ScriptController_ExposeLeftAnalogR[];
// GLOBAL: 0x5d12a8 Script::g_vt_ScriptController_ExposeLeftAnalogU
extern char g_vt_ScriptController_ExposeLeftAnalogU[];
// GLOBAL: 0x5d18a8 Script::g_vt_ScriptController_ExposeR1
extern char g_vt_ScriptController_ExposeR1[];
// GLOBAL: 0x5d1948 Script::g_vt_ScriptController_ExposeR2
extern char g_vt_ScriptController_ExposeR2[];
// GLOBAL: 0x5d1b30 Script::g_vt_ScriptController_ExposeRightAnalogAngle
extern char g_vt_ScriptController_ExposeRightAnalogAngle[];
// GLOBAL: 0x5d1450 Script::g_vt_ScriptController_ExposeRightAnalogD
extern char g_vt_ScriptController_ExposeRightAnalogD[];
// GLOBAL: 0x5d1350 Script::g_vt_ScriptController_ExposeRightAnalogL
extern char g_vt_ScriptController_ExposeRightAnalogL[];
// GLOBAL: 0x5d1500 Script::g_vt_ScriptController_ExposeRightAnalogMagnitude
extern char g_vt_ScriptController_ExposeRightAnalogMagnitude[];
// GLOBAL: 0x5d13a8 Script::g_vt_ScriptController_ExposeRightAnalogR
extern char g_vt_ScriptController_ExposeRightAnalogR[];
// GLOBAL: 0x5d1400 Script::g_vt_ScriptController_ExposeRightAnalogU
extern char g_vt_ScriptController_ExposeRightAnalogU[];
// GLOBAL: 0x5d17a8 Script::g_vt_ScriptController_ExposeSelect
extern char g_vt_ScriptController_ExposeSelect[];
// GLOBAL: 0x5d1750 Script::g_vt_ScriptController_ExposeSquare
extern char g_vt_ScriptController_ExposeSquare[];
// GLOBAL: 0x5d1800 Script::g_vt_ScriptController_ExposeStart
extern char g_vt_ScriptController_ExposeStart[];
// GLOBAL: 0x5d16a8 Script::g_vt_ScriptController_ExposeTriangle
extern char g_vt_ScriptController_ExposeTriangle[];
// GLOBAL: 0x5d1700 Script::g_vt_ScriptController_ExposeX
extern char g_vt_ScriptController_ExposeX[];

// 0x439810
void *ScriptController::GetMembers(int *count)
{
	return Members(count);
}

void *ScriptController::Members(int *count)
{
	static zoosters::MemberRow<ScriptController::ExposeCircle, ScriptController::ExposeTriangle, ScriptController::ExposeX, ScriptController::ExposeSquare, ScriptController::ExposeSelect, ScriptController::ExposeStart, ScriptController::ExposeL1, ScriptController::ExposeR1, ScriptController::ExposeL2, ScriptController::ExposeR2, ScriptController::ExposeDPadL, ScriptController::ExposeDPadR, ScriptController::ExposeDPadU, ScriptController::ExposeDPadD, ScriptController::ExposeLeftAnalogL, ScriptController::ExposeLeftAnalogR, ScriptController::ExposeLeftAnalogU, ScriptController::ExposeLeftAnalogD, ScriptController::ExposeRightAnalogL, ScriptController::ExposeRightAnalogR, ScriptController::ExposeRightAnalogU, ScriptController::ExposeRightAnalogD, ScriptController::ExposeLeftAnalogAngle, ScriptController::ExposeLeftAnalogMagnitude, ScriptController::ExposeRightAnalogAngle, ScriptController::ExposeRightAnalogMagnitude, ScriptController::ExposeControllerUnplugged> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

Script::ScriptReference *TFBType_Controller();
// GLOBAL: 0x603074 g_activeControllers

namespace Script {

// 0x4394d0
const char *ScriptController::ExposeLeftAnalogL::GetName()
{
	return "analog1 left";
}

// 0x439600
const char *ScriptController::ExposeCircle::GetName()
{
	return "circle button";
}

// 0x4396a0
const char *ScriptController::ExposeDPadL::GetName()
{
	return "DPad left";
}

// 0x439580
const char *ScriptController::ExposeActiveControllers::GetName()
{
	return "Active Controllers";
}

// 0x4393c0
const char *ScriptController::GetName()
{
	return "controller";
}

// 0x4395a0
int ScriptController::ExposeActiveControllers::GetInt(ScriptReference *owner)
{
	return g_activeControllers;
}

// 0x439590
void ScriptController::ExposeActiveControllers::SetValue(int value, ScriptReference *owner)
{
	g_activeControllers = value;
}

}  // namespace Script

// 0x439450
Script::ScriptReference *ScriptController_GetType()
{
	return TFBType_Controller();
}


namespace Script {

// 0x4394e0
const char *ScriptController::ExposeLeftAnalogR::GetName()
{
	return "analog1 right";
}

// 0x4394f0
const char *ScriptController::ExposeLeftAnalogU::GetName()
{
	return "analog1 up";
}

// 0x439500
const char *ScriptController::ExposeLeftAnalogD::GetName()
{
	return "analog1 down";
}

// 0x439510
const char *ScriptController::ExposeRightAnalogL::GetName()
{
	return "analog2 left";
}

// 0x439520
const char *ScriptController::ExposeRightAnalogR::GetName()
{
	return "analog2 right";
}

// 0x439530
const char *ScriptController::ExposeRightAnalogU::GetName()
{
	return "analog2 up";
}

// 0x439540
const char *ScriptController::ExposeRightAnalogD::GetName()
{
	return "analog2 down";
}

// 0x439550
const char *ScriptController::ExposeLeftAnalogMagnitude::GetName()
{
	return "analog1 magnitude";
}

// 0x439560
const char *ScriptController::ExposeRightAnalogMagnitude::GetName()
{
	return "analog2 magnitude";
}

// 0x439570
const char *ScriptController::ExposeControllerUnplugged::GetName()
{
	return "connection";
}

// 0x439610
const char *ScriptController::ExposeTriangle::GetName()
{
	return "triangle button";
}

// 0x439620
const char *ScriptController::ExposeX::GetName()
{
	return "X button";
}

// 0x439630
const char *ScriptController::ExposeSquare::GetName()
{
	return "square button";
}

// 0x439640
const char *ScriptController::ExposeSelect::GetName()
{
	return "select button";
}

// 0x439650
const char *ScriptController::ExposeStart::GetName()
{
	return "start button";
}

// 0x439660
const char *ScriptController::ExposeL1::GetName()
{
	return "L1 button";
}

// 0x439670
const char *ScriptController::ExposeR1::GetName()
{
	return "R1 button";
}

// 0x439680
const char *ScriptController::ExposeL2::GetName()
{
	return "L2 button";
}

// 0x439690
const char *ScriptController::ExposeR2::GetName()
{
	return "R2 button";
}

// 0x4396b0
const char *ScriptController::ExposeDPadR::GetName()
{
	return "DPad right";
}

// 0x4396c0
const char *ScriptController::ExposeDPadU::GetName()
{
	return "DPad up";
}

// 0x4396d0
const char *ScriptController::ExposeDPadD::GetName()
{
	return "DPad down";
}

// 0x4396e0
const char *ScriptController::ExposeLeftAnalogAngle::GetName()
{
	return "analog1 heading";
}

// 0x4396f0
const char *ScriptController::ExposeRightAnalogAngle::GetName()
{
	return "analog2 heading";
}

// 0x43c120
const char *ScriptController::ScriptRumble::ExposeRumbleDisabled::GetName()
{
	return "rumble disabled";
}

// 0x403b60
const char *ScriptController::ScriptRumble::ExposeActuator1Level::GetName()
{
	return "high frequency";
}

// 0x403b70
const char *ScriptController::ScriptRumble::ExposeActuator2Level::GetName()
{
	return "low frequency";
}

// 0x403b80
const char *ScriptController::ScriptRumble::ExposeDuration::GetName()
{
	return "duration";
}

// 0x403b90
const char *ScriptController::ScriptRumble::ExposeRadius::GetName()
{
	return "radius";
}

// 0x403ba0
const char *ScriptController::ScriptRumble::ExposeAttenuation::GetName()
{
	return "attenuation";
}

}  // namespace Script

// 0x403a50
const char *ScriptRumble_GetName()
{
	return "rumble";
}

// 0x403a60
Script::ScriptReference *ScriptRumble_GetType()
{
	return TFBType_Rumble();
}
