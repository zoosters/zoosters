#include "rw/rpworld.h"
#define ZOOSTERS_GAMELIGHT_DEFINITION   // see the constructor declaration in gamelight.h
#include "modules/fx/gamelight.h"
#include "modules/fx/cfxcolorlight.h"

extern const RwV3d g_xAxis;     // GLOBAL: 0x5caf58 g_xAxis
extern const RwV3d g_yAxis;     // GLOBAL: 0x5caf64 g_yAxis

namespace RWS { namespace FX {

// GLOBAL: 0x407a40 ?GetMembers@GameLight@FX@RWS@@UAEPAXPAH@Z
// 0x407b70
GameLight::GameLight(CFXColorLight *owner) : m_facing(0), m_tilt(0), m_refCount(0), m_colorLight(owner)
{
}

// 0x407530
int GameLight::AdjustRefCount(int delta)
{
	m_refCount += delta;
	if (m_refCount == 0) {
		delete this;
		return 0;
	}
	return m_refCount;
}

// 0x407550
float GameLight::GetFacing()
{
	return m_facing;
}

// 0x407560
float GameLight::GetTilt()
{
	return m_tilt;
}

// 0x407740
bool GameLight::SetFacing(float value, int flag)
{
	m_facing = value;
	RwFrame *frame = RpLightGetFrame(m_colorLight->GetLight());
	if (frame) {
		RwV3d pos = frame->modelling.pos;
		RwFrameRotate(frame, &g_xAxis, -m_tilt, rwCOMBINEREPLACE);
		RwFrameRotate(frame, &g_yAxis, m_facing, rwCOMBINEPOSTCONCAT);
		RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);
	}
	return true;
}

// 0x4077c0
bool GameLight::SetTilt(float value, Script::ScriptReference *owner)
{
	m_tilt = value;
	RwFrame *frame = RpLightGetFrame(m_colorLight->GetLight());
	if (frame) {
		RwV3d pos = frame->modelling.pos;
		RwFrameRotate(frame, &g_xAxis, -m_tilt, rwCOMBINEREPLACE);
		RwFrameRotate(frame, &g_yAxis, m_facing, rwCOMBINEPOSTCONCAT);
		RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);
	}
	return true;
}

// 0x407840
Vec3 *GameLight::GetVector(Script::ScriptReference *ownerRef)
{
	GameLight *owner = static_cast<GameLight *>(ownerRef);
	return reinterpret_cast<Vec3 *>(&RpLightGetFrame(owner->m_colorLight->GetLight())->modelling.pos);   // Vec3 = RwV3d layout
}

// 0x407860
void GameLight::SetVector(const Vec3 *v, Script::ScriptReference *ownerRef)
{
	GameLight *owner = static_cast<GameLight *>(ownerRef);
	const RwV3d *pos = reinterpret_cast<const RwV3d *>(v);   // Vec3 = RwV3d layout
	const RwV3d p = *pos;
	RwV3d delta;
	delta.x = (p.x - RpLightGetFrame(owner->m_colorLight->GetLight())->modelling.pos.x);
	RwV3d q = *pos;
	RwReal tmp = (q.y - RpLightGetFrame(owner->m_colorLight->GetLight())->modelling.pos.y);
	delta.y = tmp;
	RwV3d r;
	r = *pos;
	delta.z = r.z - RpLightGetFrame(owner->m_colorLight->GetLight())->modelling.pos.z;
	RwFrameTranslate(RpLightGetFrame(owner->m_colorLight->GetLight()), &delta, rwCOMBINEPOSTCONCAT);
}

// 0x407910
void GameLight::ApplyColor()
{
	RwRGBAReal color;
	color.red = m_colorBytes[0] * (1.0f / 255.0f);
	color.green = m_colorBytes[1] * (1.0f / 255.0f);
	color.blue = m_colorBytes[2] * (1.0f / 255.0f);
	color.alpha = m_colorBytes[3] * (1.0f / 255.0f);
	RpLightSetColor(m_colorLight->GetLight(), &color);
}
}}
