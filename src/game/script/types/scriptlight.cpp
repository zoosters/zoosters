// Script::ScriptLight: the colour members the script reads and writes on a light (a RWS::FX::GameLight).
#include "script/membertable.h"
#include "script/types/scriptlight.h"
#include "modules/fx/cfxcolorlight.h"
#include "modules/fx/gamelight.h"
#include "zoosters/memberrow.h"

namespace Script {

// 0x4075b0
int ScriptLight::ExposeLightColor::GetInt(ScriptReference *owner)
{
	return static_cast<RWS::FX::GameLight *>(owner)->m_color;
}

// 0x407a00
void ScriptLight::ExposeLightColor::SetValue(int value, ScriptReference *owner)
{
	RWS::FX::GameLight *light = static_cast<RWS::FX::GameLight *>(owner);
	light->m_color = value;
	if (light->m_colorLight->GetLight())
		light->ApplyColor();
}

// 0x407570
int ScriptLight::ExposeLightColor::ExposeR::GetInt(ScriptReference *owner)
{
	return static_cast<RWS::FX::GameLight *>(owner)->m_colorBytes[0];
}

// 0x407980
void ScriptLight::ExposeLightColor::ExposeR::SetValue(int value, ScriptReference *owner)
{
	RWS::FX::GameLight *light = static_cast<RWS::FX::GameLight *>(owner);
	light->m_colorBytes[0] = (unsigned char)value;
	if (light->m_colorLight->GetLight())
		light->ApplyColor();
}

// 0x407580
int ScriptLight::ExposeLightColor::ExposeG::GetInt(ScriptReference *owner)
{
	return static_cast<RWS::FX::GameLight *>(owner)->m_colorBytes[1];
}

// 0x4079a0
void ScriptLight::ExposeLightColor::ExposeG::SetValue(int value, ScriptReference *owner)
{
	RWS::FX::GameLight *light = static_cast<RWS::FX::GameLight *>(owner);
	light->m_colorBytes[1] = (unsigned char)value;
	if (light->m_colorLight->GetLight())
		light->ApplyColor();
}

// 0x407590
int ScriptLight::ExposeLightColor::ExposeB::GetInt(ScriptReference *owner)
{
	return static_cast<RWS::FX::GameLight *>(owner)->m_colorBytes[2];
}

// 0x4079c0
void ScriptLight::ExposeLightColor::ExposeB::SetValue(int value, ScriptReference *owner)
{
	RWS::FX::GameLight *light = static_cast<RWS::FX::GameLight *>(owner);
	light->m_colorBytes[2] = (unsigned char)value;
	if (light->m_colorLight->GetLight())
		light->ApplyColor();
}

// 0x4075a0
int ScriptLight::ExposeLightColor::ExposeA::GetInt(ScriptReference *owner)
{
	return static_cast<RWS::FX::GameLight *>(owner)->m_colorBytes[3];
}

// 0x4079e0
void ScriptLight::ExposeLightColor::ExposeA::SetValue(int value, ScriptReference *owner)
{
	RWS::FX::GameLight *light = static_cast<RWS::FX::GameLight *>(owner);
	light->m_colorBytes[3] = (unsigned char)value;
	if (light->m_colorLight->GetLight())
		light->ApplyColor();
}

}  // namespace Script

namespace Script {
// GLOBAL: 0x5cb088 Script::g_vt_ScriptLight_ExposeLightColor_ExposeA
extern char g_vt_ScriptLight_ExposeLightColor_ExposeA[];
// GLOBAL: 0x5cb0d0 Script::g_vt_ScriptLight_ExposeLightColor_ExposeB
extern char g_vt_ScriptLight_ExposeLightColor_ExposeB[];
// GLOBAL: 0x5cb118 Script::g_vt_ScriptLight_ExposeLightColor_ExposeG
extern char g_vt_ScriptLight_ExposeLightColor_ExposeG[];
// GLOBAL: 0x5cb160 Script::g_vt_ScriptLight_ExposeLightColor_ExposeR
extern char g_vt_ScriptLight_ExposeLightColor_ExposeR[];

// 0x407af0
void *ScriptLight::ExposeLightColor::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptLight::ExposeLightColor::ExposeR, ScriptLight::ExposeLightColor::ExposeG, ScriptLight::ExposeLightColor::ExposeB, ScriptLight::ExposeLightColor::ExposeA> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5cb038 Script::g_vt_ScriptLight_ExposeLightColor
extern char g_vt_ScriptLight_ExposeLightColor[];

// 0x407a40
void *ScriptLight::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptLight::ExposeLightColor> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_Light();

namespace Script {

// 0x407ae0
const char *ScriptLight::ExposeLightColor::GetName()
{
	return "light color";
}

}  // namespace Script

// 0x407a20
const char *ScriptLight_GetName()
{
	return "light";
}

// fallback free function; Ghidra name: Script::ScriptLight::GetType
// 0x407a30
Script::ScriptReference *ScriptLight_GetType()
{
	return TFBType_Light();
}
