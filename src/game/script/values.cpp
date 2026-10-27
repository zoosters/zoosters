// GetName and tiny getters of the sprite / media / sound / prop / camera script classes.
#include "script/membertable.h"
#include "framework/toolkits/animation/canimset.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptcamera.h"
#include "script/types/anglemeasurement.h"
#include "script/types/scriptmedia.h"
#include "script/types/colormeasurement.h"
#include "script/types/twodmeasurement.h"
#include "zoosters/memberrow.h"

namespace Script {

// 0x416500
const char *TwoDMeasurement::GetName()
{
	return "2D";
}

// 0x416520
const char *ColorMeasurement::GetName()
{
	return "color";
}

// 0x416660
const char *AngleMeasurement::GetName()
{
	return "angle";
}

// 0x4166e0
const char *TwoDMeasurement::ExposeX::GetName()
{
	return "X";
}

// 0x4166f0
const char *TwoDMeasurement::ExposeY::GetName()
{
	return "Y";
}

// 0x416400
ScriptReference *TwoDMeasurement::GetType()
{
	return TFBType_TwoD();
}

// 0x418b60
const char *ScriptMedia::GetName()
{
	return "media";
}

}  // namespace Script

namespace Script {
// GLOBAL: 0x5cc4f8 Script::g_vtTwoDX
extern char g_vtTwoDX[];
// GLOBAL: 0x5cc4b0 Script::g_vtTwoDY
extern char g_vtTwoDY[];

// 0x4165c0
void *TwoDMeasurement::GetMembers(int *count)
{
	static zoosters::MemberRow<TwoDMeasurement::ExposeX, TwoDMeasurement::ExposeY> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5cc878 Script::g_vtColA
extern char g_vtColA[];
// GLOBAL: 0x5cc8c0 Script::g_vtColB
extern char g_vtColB[];
// GLOBAL: 0x5cc908 Script::g_vtColG
extern char g_vtColG[];
// GLOBAL: 0x5cc950 Script::g_vtColR
extern char g_vtColR[];

// 0x416880
void *ColorMeasurement::GetMembers(int *count)
{
	static zoosters::MemberRow<ColorMeasurement::ExposeR, ColorMeasurement::ExposeG, ColorMeasurement::ExposeB, ColorMeasurement::ExposeA> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script
