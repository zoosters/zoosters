// Script::ScriptShadow's Expose* members: the script-visible settings of the blob shadows. The object itself has no data; each
// accessor reads or writes the shadow camera (g_shadowCamera: area radius, pitch, heading, vertical offset) or the ShadowRender
// behaviour (g_shadowRender: softness, fading) and ignores `this` and the owner. Lengths are in feet in the script, x12 in the game.
#include "script/types/scriptshadow.h"
#include "modules/tfb/shadowrender.h"
#include "modules/tfb/tfbshadowcamera.h"
#include "script/types/scriptshadow.h"

// GLOBAL: 0x5ca5a8 g_floatZero
// GLOBAL: 0x6182f4 g_shadowCamera
// GLOBAL: 0x622404 g_shadowRender

extern const float g_floatZero;   // 0x5ca5a8 0.0f

namespace Script {

// 0x442970
float ScriptShadow::ExposeAreaRadius::GetFloat(ScriptReference *owner)
{
	if (g_shadowCamera)
		return g_shadowCamera->m_areaRadius * (1.0f / 12.0f);
	return g_floatZero;
}

// 0x442990
void ScriptShadow::ExposeAreaRadius::SetValue(float value, ScriptReference *owner)
{
	if (g_shadowCamera)
		g_shadowCamera->m_areaRadius = value * 12.0f;
}

// 0x4429b0
float ScriptShadow::ExposePitch::GetFloat(ScriptReference *owner)
{
	if (g_shadowCamera)
		return g_shadowCamera->m_pitch;
	return g_floatZero;
}

// 0x4429d0
void ScriptShadow::ExposePitch::SetValue(float value, ScriptReference *owner)
{
	if (g_shadowCamera)
		g_shadowCamera->m_pitch = value;
}

// 0x4429f0
float ScriptShadow::ExposeHeading::GetFloat(ScriptReference *owner)
{
	if (g_shadowCamera)
		return g_shadowCamera->m_heading;
	return g_floatZero;
}

// 0x442a10
void ScriptShadow::ExposeHeading::SetValue(float value, ScriptReference *owner)
{
	if (g_shadowCamera)
		g_shadowCamera->m_heading = value;
}

// 0x442a30
float ScriptShadow::ExposeVerticalOffset::GetFloat(ScriptReference *owner)
{
	if (g_shadowCamera)
		return g_shadowCamera->m_verticalOffset * (1.0f / 12.0f);
	return g_floatZero;
}

// 0x442a50
void ScriptShadow::ExposeVerticalOffset::SetValue(float value, ScriptReference *owner)
{
	if (g_shadowCamera)
		g_shadowCamera->m_verticalOffset = value * 12.0f;
}

// 0x442a70
float ScriptShadow::ExposeSoftness::GetFloat(ScriptReference *owner)
{
	if (g_shadowRender)
		return g_shadowRender->m_softness;
	return g_floatZero;
}

// 0x442a90
void ScriptShadow::ExposeSoftness::SetValue(float value, ScriptReference *owner)
{
	if (g_shadowRender)
		g_shadowRender->m_softness = value;
}

// 0x442ab0
int ScriptShadow::ExposeEnableFading::GetInt(ScriptReference *owner)
{
	if (g_shadowRender) {
		bool b = g_shadowRender->m_enableFading != 0;
		return b;
	}
	return 0;
}

// 0x442ad0
void ScriptShadow::ExposeEnableFading::SetValue(int value, ScriptReference *owner)
{
	if (g_shadowRender) {
		bool b = value != 0;
		g_shadowRender->m_enableFading = b;
	}
}

// 0x442af0
float ScriptShadow::ExposeFadeRadius::GetFloat(ScriptReference *owner)
{
	if (g_shadowRender)
		return g_shadowRender->m_fadeRadius * (1.0f / 12.0f);
	return g_floatZero;
}

// 0x442b10
void ScriptShadow::ExposeFadeRadius::SetValue(float value, ScriptReference *owner)
{
	if (g_shadowRender)
		g_shadowRender->m_fadeRadius = value * 12.0f;
}

} // namespace Script

Script::ScriptReference *TFBType_Shadow();

namespace Script {

// 0x43efb0
const char *ScriptShadow::ExposeAreaRadius::GetName()
{
	return "area radius";
}

// 0x43f840
ScriptReference *ScriptShadow::GetType()
{
	return TFBType_Shadow();
}

// 0x43e650
const char *ScriptShadow::GetName()
{
	return "SHADOW";
}

}  // namespace Script


namespace Script {

// 0x43efc0
const char *ScriptShadow::ExposeVerticalOffset::GetName()
{
	return "vertical offset";
}

// 0x43efd0
const char *ScriptShadow::ExposeSoftness::GetName()
{
	return "softness";
}

// 0x43f000
const char *ScriptShadow::ExposeEnableFading::GetName()
{
	return "fading enabled";
}

// 0x43f020
const char *ScriptShadow::ExposeFadeRadius::GetName()
{
	return "fade radius";
}

}  // namespace Script

// 0x43efe0
int Script_AlwaysTrue()
{
	return 1;
}
