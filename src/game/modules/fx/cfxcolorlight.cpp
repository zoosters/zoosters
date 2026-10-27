#include <math.h>
#include "script/script.h"
#include "rw/rpworld.h"
#include "modules/fx/cfxcolorlight.h"
#include "modules/fx/gamelight.h"

// GLOBAL: 0x602894 g_cfxColorLightClassId
extern const char *g_cfxColorLightClassId;

namespace RWS { namespace FX {

// GLOBAL: 0x5caf70 RWS::FX::CFXColorLight::vftable
// 0x407cd0
// GLOBAL: 0x413360 ?HandleAttributes@CTFBCommand@TFB@RWS@@QAEXABVCAttributePacket@3@@Z

// 0x407c70
CAttributeHandler *(*g_keepMakeNew)(const CAttributePacket &) = &CFXColorLight::MakeNew;

// 0x407bd0
CFXColorLight::CFXColorLight(const CAttributePacket &attr) : CFXBaseLight(attr), CTFBCommand(attr)
{
	m_gameLight = new GameLight(this);
	m_gameLight->AdjustRefCount(1);
	const char *cmd = m_0;
	GameLight *light = m_gameLight;
	((::Script::ScriptReference *)light)->ResolveSymbol(light->GetType(), cmd, 0);
}

// 0x4074d0
CFXColorLight::~CFXColorLight()
{
	m_gameLight->m_colorLight = 0;
	m_gameLight->AdjustRefCount(-1);
}

// 0x4075c0
void CFXColorLight::HandleAttributes(const CAttributePacket &attr)
{
	CFXBaseLight::HandleAttributes(attr);
	for (CAttributeCommandIterator it(attr, g_cfxColorLightClassId); !it.IsFinished(); ++it) {
		if (it->GetCommandId() == 0) {
			unsigned argb = it->GetAs_RwUInt32();
			unsigned char a = (unsigned char)(argb >> 24);
			unsigned char r = (unsigned char)(argb >> 16);
			unsigned char g = (unsigned char)(argb >> 8);
			unsigned char b = (unsigned char)argb;
			RwRGBAReal color;
			color.red = (float)r * 0.003921569f;
			color.green = (float)g * 0.003921569f;
			color.blue = (float)b * 0.003921569f;
			color.alpha = (float)a * 0.003921569f;
			if (m_gameLight) {
				m_gameLight->m_colorBytes[0] = r;
				m_gameLight->m_colorBytes[1] = g;
				m_gameLight->m_colorBytes[2] = b;
				m_gameLight->m_colorBytes[3] = a;
			}
			RpLightSetColor(m_pLight, &color);
		}
	}
	if (m_gameLight) {
		const RwMatrix *ltm = &RpLightGetFrame(m_pLight)->modelling;
		float pitch = (float)atan2((double)ltm->at.y, 1.0);
		float yaw;
		if (pitch < 1.5707964f && pitch > -1.5707964f)
			yaw = (float)atan2((double)-ltm->at.x, (double)ltm->at.z);
		else
			yaw = 0.0f;
		m_gameLight->m_tilt = pitch * 57.295776f;
		m_gameLight->m_facing = yaw * 57.295776f;
	}
	AddLightToWorld();
}

}}
