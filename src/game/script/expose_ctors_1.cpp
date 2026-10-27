// Vptr-only constructors of script classes: sprite / prop / 2D / colour / angle part.
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
#include "script/types/scriptprop.h"

namespace Script {

// GLOBAL: 0x5cc328 Script::TwoDMeasurement::vftable
// 0x4164f0
TwoDMeasurement::TwoDMeasurement() {}

// GLOBAL: 0x5cc370 Script::ColorMeasurement::vftable
// 0x416510
ColorMeasurement::ColorMeasurement() {}

// GLOBAL: 0x5cc548 Script::AngleMeasurement::vftable
// 0x416650
AngleMeasurement::AngleMeasurement() {}

// GLOBAL: 0x5ccf10 Script::ScriptProp::ExposeLoopStart::vftable
// 0x41ce90
ScriptProp::ExposeLoopStart::ExposeLoopStart() {}

// GLOBAL: 0x5ccf60 Script::ScriptProp::ExposeLoopCount::vftable
// 0x41ceb0
ScriptProp::ExposeLoopCount::ExposeLoopCount() {}

}  // namespace Script

// Prototype getters inlining the vptr-only constructors above: same TU, or a second inline copy of the ctor would clash with the
// out-of-line one at link time.
using namespace Script;

// GLOBAL: 0x5cc370 Script::ColorMeasurement::vftable
// 0x42fdf0
ScriptReference *TFBType_Color()
{
	static ColorMeasurement s;
	return &s;
}

// GLOBAL: 0x5cc328 Script::TwoDMeasurement::vftable
// 0x42fe30
ScriptReference *TFBType_TwoD()
{
	static TwoDMeasurement s;
	return &s;
}

// GLOBAL: 0x5cc548 Script::AngleMeasurement::vftable
// 0x4302c0
ScriptReference *TFBType_Angle()
{
	static AngleMeasurement s;
	return &s;
}
