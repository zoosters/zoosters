// Script::ScriptMathOps: the member table (the NormalizeAngle function member).
#include "script/types/scriptmathops.h"
#include "script/membertable.h"
#include "script/actor.h"
#include <math.h>
#include "zoosters/memberrow.h"

namespace Script {
// GLOBAL: 0x5d4a10 Script::g_vt_ScriptMathOps_ExposeNormalizeAngle
extern char g_vt_ScriptMathOps_ExposeNormalizeAngle[];

// 0x43f3a0
void *ScriptMathOps::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptMathOps::ExposeNormalizeAngle> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

Script::ScriptReference *TFBType_MathOps();
// GLOBAL: 0x6221ac g_normalizedAngle

namespace Script {

// 0x43f400
const char *ScriptMathOps::ExposeNormalizeAngle::GetName()
{
	return "Normalized Angle";
}

// 0x43e670
const char *ScriptMathOps::GetName()
{
	return "MATH OPS";
}

// 0x43f410
float ScriptMathOps::ExposeNormalizeAngle::GetFloat(ScriptReference *owner)
{
	return g_normalizedAngle;
}

// 0x43f420
void ScriptMathOps::ExposeNormalizeAngle::SetValue(float value, ScriptReference *owner)
{
	float r = (float)fmod((double)value, 360.0);
	if (r > 180.0f)
		g_normalizedAngle = r - 360.0f;
	else if (r < -180.0f)
		g_normalizedAngle = r + 360.0f;
	else
		g_normalizedAngle = r;
}

// 0x43f860
ScriptReference *ScriptMathOps::GetType()
{
	return TFBType_MathOps();
}

}  // namespace Script
