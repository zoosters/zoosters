#include "rw/rpworld.h"
#include "tfb/level/level.h"
#include "modules/fx/cfxbaselight.h"
#include "framework/toolkits/world/framehelper.h"

extern int g_levelWorldsReady;                           // GLOBAL: 0x629740 g_levelWorldsReady
void TFBPlugin_C2_SetLevel(RpLight *light, int level);                        // 0x45e0b0
// GLOBAL: 0x44e240 ?RpLightDestroy@@YAHAAVCLightPtr@RWS@@@Z
// GLOBAL: 0x602890 g_cfxBaseLightClassId
extern const char *g_cfxBaseLightClassId;
namespace RWS {
RpAtomic *CreateAtomicInWorldFromResource(const CAttributePacket &attr, CEventHandler *h);   // 0x44e180
namespace FX {

// 0x406f10
// 0x406f20
void (CAtomicPtr::*g_keepAtomicPtrHSC)(const CAttributePacket &) = &CAtomicPtr::HandleSystemCommands;

// GLOBAL: 0x5caf30 RWS::FX::CFXBaseLight::vftable
// 0x407150
CFXBaseLight::CFXBaseLight(const CAttributePacket &attr) : m_type(0)
{
	m_pAtomic = CreateAtomicInWorldFromResource(attr, 0);
	Create();
}

// 0x4074a0
// 0x406f40
CFXBaseLight::~CFXBaseLight()
{
	m_pLight = 0;
}

// 0x406fd0
void CFXBaseLight::Create()
{
	m_pLight = RpLightCreate(m_type);
	if (m_pLight) {
		RwFrame *frame = RwFrameCreate_thunk();
		if (frame) {
			RwFrameSetIdentity(frame);
			RpLightSetFrame(m_pLight, frame);
			RwRGBAReal color = {1, 1, 1, 1};
			RpLightSetConeAngle(m_pLight, 45.0f * (3.1415927f / 180.0f));
			RpLightSetRadius(m_pLight, 1000.0f);
			RpLightSetColor(m_pLight, &color);
			RpLightSetFlags(m_pLight, 3);
		}
	}
}

// 0x4071e0
void CFXBaseLight::SetType(unsigned type)
{
	m_type = type;
	if (type != 0) {
		RpLight *newLight = RpLightCreate(m_type);
		if (newLight) {
			RpWorld *world = RpLightGetWorld(m_pLight);
			RwFrame *frame = RpLightGetFrame(m_pLight);
			if (world)
				RpWorldRemoveLight(world, m_pLight);
			RpLightSetFrame(m_pLight, 0);
			RpLightSetFrame(newLight, frame);
			RpLightSetRadius(newLight, RpLightGetRadius(m_pLight));
			RpLightSetColor(newLight, RpLightGetColor(m_pLight));
			RpLightSetConeAngle(newLight, RpLightGetConeAngle(m_pLight));
			RpLightSetFlags(newLight, RpLightGetFlags(m_pLight));
			if (world)
				RpWorldAddLight(world, newLight);
			RpLightDestroy(m_pLight);
			m_pLight = newLight;
		}
	} else {
		RpLight *light = m_pLight.ptr();
		RpWorld *world = RpLightGetWorld(light);
		if (world)
			RpWorldRemoveLight(world, light);
	}
}

// 0x407080
void CFXBaseLight::AddLightToWorld()
{
	if (m_type) {
		RpWorld *world = RpLightGetWorld(m_pLight);
		if (!world)
			RWS::CLevel::AddLight(m_pLight, m_level);
	}
}

// 0x4070b0
void CFXBaseLight::SetLevel(int level)
{
	if (m_level != level && m_type) {
		TFBPlugin_C2_SetLevel(m_pLight, level);
		if (g_levelWorldsReady) {
			RpLight *light = m_pLight;
			RpWorld *world = RpLightGetWorld(light);
			if (world) {
				RpWorld *w = RpWorldRemoveLight(world, light);
				if (w && m_pAtomic.ptr() && w == RpAtomicGetWorld(m_pAtomic))
					RpWorldRemoveAtomic(w, m_pAtomic);
			}
			RpWorld *nw = RWS::CLevel::AddLight(m_pLight, level);
			if (nw && m_pAtomic.ptr())
				RpWorldAddAtomic(nw, m_pAtomic);
		}
		m_level = level;
	}
}

// 0x4072d0
void CFXBaseLight::HandleAttributes(const CAttributePacket &attr)
{
	CAttributeHandler::HandleAttributes(attr);
	if (m_pAtomic.ptr())
		AtomicHelper::HandleSystemCommands(*m_pAtomic.ptr(), attr);
	if (m_pLight.ptr())
		LightHelper::HandleSystemCommands(*m_pLight.ptr(), attr);
	CAttributeCommandIterator attrIt(attr, g_cfxBaseLightClassId);
	while (!attrIt.IsFinished()) {
		switch (attrIt->GetCommandId()) {
		case 0:
			{
				RwUInt32 type = attrIt->GetAs_RwUInt32();
				switch (type) {
				case 0: type = 0; break;
				case 1: type = 1; break;
				case 2: type = 2; break;
				case 3: type = 0x80; break;
				case 4: type = 0x81; break;
				case 5: type = 0x82; break;
				default: break;
				}
				SetType(type);
			}
			break;
		case 1:
			RpLightSetConeAngle(m_pLight, attrIt->GetAs_RwReal() * (3.1415927f / 180.0f));
			break;
		case 2:
			RpLightSetRadius(m_pLight, attrIt->GetAs_RwReal());
			break;
		case 3:
			{
				const RwUInt32 LightFlags[] = {3, 1, 2};
				RpLightSetFlags(m_pLight, LightFlags[attrIt->GetAs_RwUInt32()]);
			}
			break;
		case 4:
			SetLevel(attrIt->GetAs_RwInt32());
			break;
		}
		++attrIt;
	}
}
}}
