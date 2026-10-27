// Prototype getters (static T s; return &s;).
// Classes from gfx/small_e_names.h. TFBType_Color/TwoD/Angle live at the end of expose_ctors_1.cpp (same TU as the vptr-only ctors they inline).
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
using namespace Script;

// GLOBAL: 0x5cca08 Script::ScriptMedia::vftable
// 0x43da40
ScriptReference *TFBType_Media()
{
	static ScriptMedia s;
	return &s;
}
