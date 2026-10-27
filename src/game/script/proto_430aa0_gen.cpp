// Prototype getters (static T s; return &s;).
#include "script/value.h"
#include "script/types/scriptcamera.h"
using namespace Script;

// GLOBAL: 0x5cdf20 Script::ExposeGenericValue::vftable
// 0x430aa0
ScriptReference *TFBMember_GenericValue()
{
	static ExposeGenericValue s;
	return &s;
}

// GLOBAL: 0x5d1108 Script::ScriptCamera::ExposeCameraLocation::vftable
// 0x439320
ScriptReference *TFBMember_Camera_CameraLocation()
{
	static ScriptCamera::ExposeCameraLocation s;
	return &s;
}
