#include "script/types/scriptlight.h"
#include "script/types/scriptmotionblur.h"
#include "script/actor.h"
#include "modules/fx/cfxmotionblur.h"
#include "script/types/scriptmotionblur.h"


extern const float g_floatZero;

using namespace Script;

// 0x410c40
float ScriptMotionBlur::ExposeAmount::GetFloat(ScriptReference *owner)
{
	if (g_motionBlur)
		return g_motionBlur->m_state.m_blurAmount;
	return g_floatZero;
}

// 0x410c60
void ScriptMotionBlur::ExposeAmount::SetValue(float value, ScriptReference *owner)
{
	RWS::FX::CFXMotionBlur *blur = g_motionBlur;
	if (blur) {
		blur->m_state.m_blurAmount = value;
		blur->m_state.m_updateBlendPoly = 1;
	}
}

// 0x410c80
float ScriptMotionBlur::ExposeOffsetX::GetFloat(ScriptReference *owner)
{
	if (g_motionBlur)
		return g_motionBlur->m_state.m_offsetX;
	return g_floatZero;
}

// 0x410ca0
void ScriptMotionBlur::ExposeOffsetX::SetValue(float value, ScriptReference *owner)
{
	RWS::FX::CFXMotionBlur *blur = g_motionBlur;
	if (blur) {
		blur->m_state.m_offsetX = value;
		blur->m_state.m_updateBlendPoly = 1;
	}
}

// 0x410cc0
float ScriptMotionBlur::ExposeOffsetY::GetFloat(ScriptReference *owner)
{
	if (g_motionBlur)
		return g_motionBlur->m_state.m_offsetY;
	return g_floatZero;
}

// 0x410ce0
void ScriptMotionBlur::ExposeOffsetY::SetValue(float value, ScriptReference *owner)
{
	RWS::FX::CFXMotionBlur *blur = g_motionBlur;
	if (blur) {
		blur->m_state.m_offsetY = value;
		blur->m_state.m_updateBlendPoly = 1;
	}
}

// 0x410d00
float ScriptMotionBlur::ExposeScaleX::GetFloat(ScriptReference *owner)
{
	if (g_motionBlur)
		return g_motionBlur->m_state.m_scaleX;
	return g_floatZero;
}

// 0x410d20
void ScriptMotionBlur::ExposeScaleX::SetValue(float value, ScriptReference *owner)
{
	RWS::FX::CFXMotionBlur *blur = g_motionBlur;
	if (blur) {
		blur->m_state.m_scaleX = value;
		blur->m_state.m_updateBlendPoly = 1;
	}
}

// 0x410d40
float ScriptMotionBlur::ExposeScaleY::GetFloat(ScriptReference *owner)
{
	if (g_motionBlur)
		return g_motionBlur->m_state.m_scaleY;
	return g_floatZero;
}

// 0x410d60
void ScriptMotionBlur::ExposeScaleY::SetValue(float value, ScriptReference *owner)
{
	RWS::FX::CFXMotionBlur *blur = g_motionBlur;
	if (blur) {
		blur->m_state.m_scaleY = value;
		blur->m_state.m_updateBlendPoly = 1;
	}
}

// 0x410d80
float ScriptMotionBlur::ExposeIsoScale::GetFloat(ScriptReference *owner)
{
	if (g_motionBlur)
		return g_motionBlur->m_state.m_scale;
	return g_floatZero;
}

// 0x410da0
void ScriptMotionBlur::ExposeIsoScale::SetValue(float value, ScriptReference *owner)
{
	RWS::FX::CFXMotionBlur *blur = g_motionBlur;
	if (blur) {
		blur->m_state.m_scale = value;
		blur->m_state.m_updateBlendPoly = 1;
	}
}

// 0x410dc0
float ScriptMotionBlur::ExposeField40::GetFloat(ScriptReference *owner)
{
	if (g_motionBlur)
		return g_motionBlur->m_state.m_angle;
	return g_floatZero;
}

// 0x410de0
void ScriptMotionBlur::ExposeField40::SetValue(float value, ScriptReference *owner)
{
	RWS::FX::CFXMotionBlur *blur = g_motionBlur;
	if (blur) {
		blur->m_state.m_angle = value;
		blur->m_state.m_updateBlendPoly = 1;
	}
}

Script::ScriptReference *TFBType_MotionBlur();

namespace Script {

// 0x43f580
const char *ScriptMotionBlur::ExposeAmount::GetName()
{
	return "amount";
}

// 0x43f870
ScriptReference *ScriptMotionBlur::GetType()
{
	return TFBType_MotionBlur();
}

// 0x43e680
const char *ScriptMotionBlur::GetName()
{
	return "MOTION BLUR";
}

}  // namespace Script


namespace Script {

// 0x43f590
const char *ScriptMotionBlur::ExposeOffsetX::GetName()
{
	return "offset-x";
}

// 0x43f5a0
const char *ScriptMotionBlur::ExposeOffsetY::GetName()
{
	return "offset-y";
}

// 0x43f5b0
const char *ScriptMotionBlur::ExposeScaleX::GetName()
{
	return "scale x-axis";
}

// 0x43f5d0
const char *ScriptMotionBlur::ExposeScaleY::GetName()
{
	return "scale y-axis";
}

}  // namespace Script

// 0x43f5f0
const char *ExposeScales_GetName()
{
	return "scale";
}
