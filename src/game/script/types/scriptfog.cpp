#include "script/membertable.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptcamera.h"
#include "tfb/level/level.h"
#include "tfb/render/postfx.h"
#include "script/types/twodmeasurement.h"
#include "zoosters/memberrow.h"

// GLOBAL: 0x629708 g_vertFogDist
// GLOBAL: 0x629704 g_vertFogColor
// GLOBAL: 0x617d5c g_pixelFogEnable
// GLOBAL: 0x602980 g_dofBluriness
// GLOBAL: 0x617d90 g_pixelFogDist
// GLOBAL: 0x617d98 g_pixelCutoff
// GLOBAL: 0x617d60 g_pixelFogColor
// GLOBAL: 0x617d95 g_bloomEnable
// GLOBAL: 0x602984 g_bloomAddFactor
// GLOBAL: 0x602988 g_bloomHaloSize

namespace Script {

// A colour as the scripts see it: bytes r, g, b, a (one int); the game keeps the vertex fog colour as D3DCOLOR.
struct RGBA8 {
	unsigned char r, g, b, a;
};

static unsigned MakeARGB(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
{
	return (((((a << 8) | r) << 8) | g) << 8) | b;
}

}  // namespace Script

using namespace Script;

// 0x41a650
float ScriptFog::ExposeVertFogDistance::GetFloat(ScriptReference *owner)
{
	return g_vertFogDist * 0.083333336f;
}

// 0x41a660
void ScriptFog::ExposeVertFogDistance::SetValue(float value, ScriptReference *owner)
{
	SetVertFogDistance(value * 12.0f);
}

// 0x41a680
int ScriptFog::ExposeVertFogColor::GetInt(ScriptReference *owner)
{
	unsigned c = g_vertFogColor;
	RGBA8 out;
	out.a = (unsigned char)(c >> 24);
	out.r = (unsigned char)(c >> 16);
	out.g = (unsigned char)(c >> 8);
	out.b = (unsigned char)c;
	return *(int *)&out;
}

// 0x41a6b0
void ScriptFog::ExposeVertFogColor::SetValue(int value, ScriptReference *owner)
{
	RGBA8 in = *(RGBA8 *)&value;
	SetVertFogColor(MakeARGB(in.a, in.r, in.g, in.b));
}

// 0x41a6f0
int ScriptFog::ExposeVertFogColor::ExposeR::GetInt(ScriptReference *owner)
{
	return ((unsigned char *)&g_vertFogColor)[2];
}

// 0x41a700
void ScriptFog::ExposeVertFogColor::ExposeR::SetValue(int value, ScriptReference *owner)
{
	unsigned argb = g_vertFogColor;
	RGBA8 c;
	c.a = (unsigned char)(argb >> 24);
	c.r = (unsigned char)(argb >> 16);
	c.g = (unsigned char)(argb >> 8);
	c.b = (unsigned char)argb;
	c.r = (unsigned char)value;
	SetVertFogColor(MakeARGB(c.a, c.r, c.g, c.b));
}

// 0x41a740
int ScriptFog::ExposeVertFogColor::ExposeG::GetInt(ScriptReference *owner)
{
	return ((unsigned char *)&g_vertFogColor)[1];
}

// 0x41a750
void ScriptFog::ExposeVertFogColor::ExposeG::SetValue(int value, ScriptReference *owner)
{
	unsigned argb = g_vertFogColor;
	RGBA8 c;
	c.a = (unsigned char)(argb >> 24);
	c.r = (unsigned char)(argb >> 16);
	c.g = (unsigned char)(argb >> 8);
	c.b = (unsigned char)argb;
	c.g = (unsigned char)value;
	SetVertFogColor(MakeARGB(c.a, c.r, c.g, c.b));
}

// 0x41a790
int ScriptFog::ExposeVertFogColor::ExposeB::GetInt(ScriptReference *owner)
{
	return g_vertFogColor & 0xff;
}

// 0x41a7a0
void ScriptFog::ExposeVertFogColor::ExposeB::SetValue(int value, ScriptReference *owner)
{
	unsigned argb = g_vertFogColor;
	RGBA8 c;
	c.a = (unsigned char)(argb >> 24);
	c.r = (unsigned char)(argb >> 16);
	c.g = (unsigned char)(argb >> 8);
	c.b = (unsigned char)argb;
	c.b = (unsigned char)value;
	SetVertFogColor(MakeARGB(c.a, c.r, c.g, c.b));
}

// 0x41a7e0
int ScriptFog::ExposeVertFogColor::ExposeA::GetInt(ScriptReference *owner)
{
	return ((unsigned char *)&g_vertFogColor)[3];
}

// 0x41a7f0
void ScriptFog::ExposeVertFogColor::ExposeA::SetValue(int value, ScriptReference *owner)
{
	unsigned argb = g_vertFogColor;
	RGBA8 c;
	c.a = (unsigned char)(argb >> 24);
	c.r = (unsigned char)(argb >> 16);
	c.g = (unsigned char)(argb >> 8);
	c.b = (unsigned char)argb;
	c.a = (unsigned char)value;
	SetVertFogColor(MakeARGB(c.a, c.r, c.g, c.b));
}

// 0x41a830
int ScriptFog::ExposePixelFogEnable::GetInt(ScriptReference *owner)
{
	return g_pixelFogEnable;
}

// 0x41a840
void ScriptFog::ExposePixelFogEnable::SetValue(int value, ScriptReference *owner)
{
	g_pixelFogEnable = value != 0;
}

// 0x41a860
int ScriptFog::ExposeDOFBluriness::GetInt(ScriptReference *owner)
{
	return g_dofBluriness;
}

// 0x41a870
void ScriptFog::ExposeDOFBluriness::SetValue(int value, ScriptReference *owner)
{
	SetDofBluriness(value);
}

// 0x41a880
float ScriptFog::ExposePixelFogDistance::GetFloat(ScriptReference *owner)
{
	return g_pixelFogDist * 0.083333336f;
}

// 0x41a890
void ScriptFog::ExposePixelFogDistance::SetValue(float value, ScriptReference *owner)
{
	SetPixelFogDistance(value * 12.0f);
}

// 0x41a8b0
int ScriptFog::ExposePixelCutoffIntensity::GetInt(ScriptReference *owner)
{
	return (unsigned char)g_pixelCutoff;
}

// 0x41a8c0
void ScriptFog::ExposePixelCutoffIntensity::SetValue(int value, ScriptReference *owner)
{
	g_pixelCutoff = value;
}

// SKIPPED 0x41a8d0 ExposePixelFogColor::GetInt (the game loads word [+2] then dword [+0] of the colour, not reproduced)
// 0x41a8d0 Script::ScriptFog::ExposePixelFogColor::GetInt
int ScriptFog::ExposePixelFogColor::GetInt(ScriptReference *owner)
{
	RGBA8 c;
	c.a = g_pixelFogColor[3];
	c.r = g_pixelFogColor[0];
	c.g = g_pixelFogColor[1];
	c.b = g_pixelFogColor[2];
	return *(int *)&c;
}

// 0x41a900
void ScriptFog::ExposePixelFogColor::SetValue(int value, ScriptReference *owner)
{
	RGBA8 in = *(RGBA8 *)&value;
	g_pixelFogColor[3] = in.a;
	g_pixelFogColor[0] = in.r;
	g_pixelFogColor[1] = in.g;
	g_pixelFogColor[2] = in.b;
}

// 0x41a930
int ScriptFog::ExposePixelFogColor::ExposeR::GetInt(ScriptReference *owner)
{
	return g_pixelFogColor[0];
}

// 0x41a940
void ScriptFog::ExposePixelFogColor::ExposeR::SetValue(int value, ScriptReference *owner)
{
	g_pixelFogColor[0] = (unsigned char)value;
}

// 0x41a950
int ScriptFog::ExposePixelFogColor::ExposeG::GetInt(ScriptReference *owner)
{
	return g_pixelFogColor[1];
}

// 0x41a960
void ScriptFog::ExposePixelFogColor::ExposeG::SetValue(int value, ScriptReference *owner)
{
	g_pixelFogColor[1] = (unsigned char)value;
}

// 0x41a970
int ScriptFog::ExposePixelFogColor::ExposeB::GetInt(ScriptReference *owner)
{
	return g_pixelFogColor[2];
}

// 0x41a980
void ScriptFog::ExposePixelFogColor::ExposeB::SetValue(int value, ScriptReference *owner)
{
	g_pixelFogColor[2] = (unsigned char)value;
}

// 0x41a990
int ScriptFog::ExposePixelFogColor::ExposeA::GetInt(ScriptReference *owner)
{
	return g_pixelFogColor[3];
}

// 0x41a9a0
void ScriptFog::ExposePixelFogColor::ExposeA::SetValue(int value, ScriptReference *owner)
{
	g_pixelFogColor[3] = (unsigned char)value;
}

// 0x41a9b0
int ScriptFog::ExposeBloomEnable::GetInt(ScriptReference *owner)
{
	return g_bloomEnable;
}

// 0x41a9c0
void ScriptFog::ExposeBloomEnable::SetValue(int value, ScriptReference *owner)
{
	g_bloomEnable = value != 0;
}

// 0x41a9e0
float ScriptFog::ExposeBloomAddFactor::GetFloat(ScriptReference *owner)
{
	return g_bloomAddFactor;
}

// 0x41a9f0
void ScriptFog::ExposeBloomAddFactor::SetValue(float value, ScriptReference *owner)
{
	g_bloomAddFactor = value;
}

// 0x41aa00
float ScriptFog::ExposeBloomHaloSize::GetFloat(ScriptReference *owner)
{
	return g_bloomHaloSize;
}

// 0x41aa10
void ScriptFog::ExposeBloomHaloSize::SetValue(float value, ScriptReference *owner)
{
	g_bloomHaloSize = value;
}

namespace Script {
// GLOBAL: 0x5d4760 Script::g_vt_ScriptFog_ExposeVertFogColor_ExposeA
extern char g_vt_ScriptFog_ExposeVertFogColor_ExposeA[];
// GLOBAL: 0x5d47a8 Script::g_vt_ScriptFog_ExposeVertFogColor_ExposeB
extern char g_vt_ScriptFog_ExposeVertFogColor_ExposeB[];
// GLOBAL: 0x5d47f0 Script::g_vt_ScriptFog_ExposeVertFogColor_ExposeG
extern char g_vt_ScriptFog_ExposeVertFogColor_ExposeG[];
// GLOBAL: 0x5d4838 Script::g_vt_ScriptFog_ExposeVertFogColor_ExposeR
extern char g_vt_ScriptFog_ExposeVertFogColor_ExposeR[];

// 0x43f270
void *ScriptFog::ExposeVertFogColor::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptFog::ExposeVertFogColor::ExposeR, ScriptFog::ExposeVertFogColor::ExposeG, ScriptFog::ExposeVertFogColor::ExposeB, ScriptFog::ExposeVertFogColor::ExposeA> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5d4880 Script::g_vt_ScriptFog_ExposePixelFogColor_ExposeA
extern char g_vt_ScriptFog_ExposePixelFogColor_ExposeA[];
// GLOBAL: 0x5d48c8 Script::g_vt_ScriptFog_ExposePixelFogColor_ExposeB
extern char g_vt_ScriptFog_ExposePixelFogColor_ExposeB[];
// GLOBAL: 0x5d4910 Script::g_vt_ScriptFog_ExposePixelFogColor_ExposeG
extern char g_vt_ScriptFog_ExposePixelFogColor_ExposeG[];
// GLOBAL: 0x5d4958 Script::g_vt_ScriptFog_ExposePixelFogColor_ExposeR
extern char g_vt_ScriptFog_ExposePixelFogColor_ExposeR[];

// 0x43f2f0
void *ScriptFog::ExposePixelFogColor::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptFog::ExposePixelFogColor::ExposeR, ScriptFog::ExposePixelFogColor::ExposeG, ScriptFog::ExposePixelFogColor::ExposeB, ScriptFog::ExposePixelFogColor::ExposeA> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

Script::ScriptReference *TFBType_Fog();

namespace Script {

// 0x43f1a0
const char *ScriptFog::ExposeVertFogDistance::GetName()
{
	return "vertex fog distance";
}

// 0x43f850
ScriptReference *ScriptFog::GetType()
{
	return TFBType_Fog();
}

// 0x43e660
const char *ScriptFog::GetName()
{
	return "FOG";
}

}  // namespace Script


namespace Script {

// 0x43f1b0
const char *ScriptFog::ExposeVertFogColor::GetName()
{
	return "vertex fog color";
}

// 0x43f1c0
const char *ScriptFog::ExposePixelFogEnable::GetName()
{
	return "pixel fog toggle";
}

// 0x43f1d0
const char *ScriptFog::ExposePixelFogDistance::GetName()
{
	return "pixel fog/DOF distance";
}

// 0x43f1e0
const char *ScriptFog::ExposePixelCutoffIntensity::GetName()
{
	return "pixel fog/DOF cutoff intensity";
}

// 0x43f200
const char *ScriptFog::ExposePixelFogColor::GetName()
{
	return "pixel fog/DOF color";
}

// 0x43f220
const char *ScriptFog::ExposeBloomEnable::GetName()
{
	return "bloom toggle";
}

// 0x43f240
const char *ScriptFog::ExposeBloomAddFactor::GetName()
{
	return "bloom add factor";
}

// 0x43f260
const char *ScriptFog::ExposeBloomHaloSize::GetName()
{
	return "bloom halo size";
}

// 0x43f370
const char *ScriptFog::ExposePixelFogColor::ExposeA::GetName()
{
	return "far plane alpha";
}

// 0x43f390
const char *ScriptFog::ExposeDOFBluriness::GetName()
{
	return "DOF bluriness factor";
}

}  // namespace Script
