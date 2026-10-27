#include "modules/tfb/ctfbworld.h"
#include "tfb/level/level.h"
#include "framework/core/cresourcemanager.h"
#include "rw/rpplugins.h"
#include "tfb/render/postfx.h"

// GLOBAL: 0x602ec8 g_ctfbWorldClassId
// GLOBAL: 0x4500b0 ?GetGuid@LevelResourceRef@@QBEXPAVRWSGUID@RWS@@@Z
// GLOBAL: 0x629724 RWS::CLevel::sm_pOpaqueWorld
// GLOBAL: 0x62972c RWS::CLevel::sm_pActiveWorld
// GLOBAL: 0x629728 RWS::CLevel::sm_pTranslucentWorld
// GLOBAL: 0x629730 RWS::CLevel::sm_pPreOpaqueWorld
// GLOBAL: 0x629734 RWS::CLevel::sm_pSkyWorld
// GLOBAL: 0x629738 RWS::CLevel::sm_pFogWorld
// GLOBAL: 0x629700 g_fogMode
// GLOBAL: 0x6296f0 g_fogFlag
// GLOBAL: 0x6296f4 g_fogParamB
// GLOBAL: 0x6296f8 g_fogParamA
// GLOBAL: 0x60297c g_levelFarDefault
// GLOBAL: 0x62a59c g_distClip
// GLOBAL: 0x629710 g_nearClip
// GLOBAL: 0x62970c g_farClip
// GLOBAL: 0x629708 g_vertFogDist
// GLOBAL: 0x5ca5a8 g_floatZero
// GLOBAL: 0x617d5c g_pixelFogEnable
// GLOBAL: 0x617d98 g_pixelCutoff
// GLOBAL: 0x617d60 g_pixelFogColor
// GLOBAL: 0x617d95 g_bloomEnable
// GLOBAL: 0x602984 g_bloomAddFactor
// GLOBAL: 0x602988 g_bloomHaloSize
extern const char *g_ctfbWorldClassId;
extern const float g_floatZero;
// The pixel fog and bloom state belongs to the post-effect code (g/postfx_411.cpp); the attributes only store into it.
extern unsigned char g_pixelFogEnable;
extern int g_pixelCutoff;
extern unsigned char g_pixelFogColor[4];       // 0x617d60 r, g, b, a
extern unsigned char g_bloomEnable;
extern float g_bloomAddFactor;
extern float g_bloomHaloSize;
void SetPixelFogDistance(float distance);      // 0x411200
int GetCurrentLevel();                         // 0x460c20

namespace RWS { namespace TFB {

// Attribute commands of class CTFBWorld (jump table 0x41bc48):
//  0 resource reference (type 0 opaque world + PVS, 1/2/3/5/6 world slots, 4 camera/clump), 2 near clip (max 6, kept below far, > 0),
//  3 far clip (kept above near), 4 camera event registration, 5/6 re-register events, 7 link priority, 8 fog mode 0..3,
//  9 vertical fog colour, 10 level fog param, 11 vertical fog distance, 12 member +0x4c, 13/14 fog params, 15 DOF bluriness,
//  16..19 pixel fog (enable, distance*12, cutoff, colour), 20..22 bloom (enable, halo size, add factor).
// 0x41b640
void CTFBWorld::HandleAttributes(const CAttributePacket &attr)
{
	CAttributeHandler::HandleAttributes(attr);
	for (CAttributeCommandIterator it(attr, g_ctfbWorldClassId); !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 2: {
			float v = it->GetAs_RwReal();
			if (v > 6.0f)
				v = 6.0f;
			g_nearClip = v;
			if (g_nearClip >= g_farClip)
				g_nearClip = g_farClip - 1.0f;
			if (g_nearClip <= g_floatZero)
				g_nearClip = 0.1f;
			break;
		}
		case 3:
			SetFarClip(it->GetAs_RwReal());
			if (g_nearClip >= g_farClip)
				SetFarClip(g_nearClip + 1.0f);
			if (g_levelFarDefault < g_floatZero)
				g_levelFarDefault = g_farClip;
			break;
		case 13:
			SetFogFarClip(it->GetAs_RwReal());
			if (g_nearClip >= g_fogParamA)
				SetFogFarClip(g_nearClip + 1.0f);
			break;
		case 14:
			SetFogPlane(it->GetAs_RwReal());
			if (g_fogParamB >= g_floatZero)
				g_fogFlag = 1;
			else
				g_fogFlag = 0;
			break;
		case 0: {
			const LevelResourceRef *ref = static_cast<const LevelResourceRef *>(it->GetAs_void_ptr());
			RWSGUID tmp;
			ref->GetGuid(&tmp);
			RWSGUID guid = tmp;
			const char *type = 0;
			const void *obj = CResourceManager::FindById(&guid, &type, 0, 0);
			if (ref->m_type == 4) {
				AddClumpResource((RpClump *)obj, type);
			} else if (obj) {
				RpWorld *world = (RpWorld *)obj;
				switch (ref->m_type) {
				case 0:
					CLevel::sm_pOpaqueWorld = world;
					if (RpPVSQuery(world)) {
						RpPVSHook(world);
						m_renderSectors = &CTFBWorld::RenderSectorsPVS;
					} else {
						m_renderSectors = &CTFBWorld::RenderSectors;
					}
					break;
				case 1:
					CLevel::sm_pTranslucentWorld = world;
					break;
				case 2:
					CLevel::sm_pSkyWorld = world;
					break;
				case 5:
					CLevel::sm_pFogWorld = world;
					break;
				case 3:
					CLevel::sm_pActiveWorld = world;
					break;
				case 6:
					CLevel::sm_pPreOpaqueWorld = world;
					break;
				}
			}
			break;
		}
		case 4: {
			CEventId &id = m_renderEvent;
			int prio = GetLinkedPriority(id);
			if (prio < 0)
				prio = 0xFFFF;
			UnLinkMsg(id);
			UnRegisterMsg(id);
			RegisterMsg(id, it->GetAs_RwChar_ptr(), "RwCamera*");
			LinkMsg(id, "RwCamera*", (unsigned short)prio);
			break;
		}
		case 5:
			ReplaceRegisteredMsg(m_afterOpaqueEvent, it->GetAs_RwChar_ptr(), "RwCamera*");
			break;
		case 6:
			ReplaceRegisteredMsg(m_afterRenderEvent, it->GetAs_RwChar_ptr(), "RwCamera*");
			break;
		case 7: {
			CEventId &id = m_renderEvent;
			UnLinkMsg(id);
			LinkMsg(id, "RwCamera*", it->GetAs_RwUInt16());
			break;
		}
		case 8:
			switch (it->GetAs_RwInt32()) {
			case 0:
				g_fogMode = 0;
				break;
			case 1:
				g_fogMode = 1;
				break;
			case 2:
				g_fogMode = 2;
				break;
			case 3:
				g_fogMode = 3;
				break;
			}
			break;
		case 9:
			SetVertFogColor(it->GetAs_RwUInt32());
			break;
		case 10:
			SetLevelFogParam(it->GetAs_RwReal());
			break;
		case 11:
			SetVertFogDistance(it->GetAs_RwReal());
			if (g_vertFogDist <= 1.0f)
				SetVertFogDistance(1.0f);
			if (g_vertFogDist > g_farClip)
				SetVertFogDistance(g_farClip);
			break;
		case 12:
			m_unk4c = it->GetAs_RwInt32();
			break;
		case 15:
			SetDofBluriness(it->GetAs_RwInt32());
			break;
		case 16:
			g_pixelFogEnable = it->GetAs_RwInt32() != 0;
			break;
		case 17:
			SetPixelFogDistance(it->GetAs_RwReal() * 12.0f);
			break;
		case 19: {
			unsigned v = it->GetAs_RwUInt32();
			if (!g_distClip || GetCurrentLevel() == 1 || GetCurrentLevel() == 3 || GetCurrentLevel() == 8 || GetCurrentLevel() == 0xb ||
			    GetCurrentLevel() == 0xe || GetCurrentLevel() == 0xd || GetCurrentLevel() == 0xf)
				g_pixelFogColor[3] = (unsigned char)(v >> 24);
			else
				g_pixelFogColor[3] = 0xff;
			g_pixelFogColor[0] = (unsigned char)(v >> 16);
			g_pixelFogColor[1] = (unsigned char)(v >> 8);
			g_pixelFogColor[2] = (unsigned char)v;
			break;
		}
		case 18:
			g_pixelCutoff = it->GetAs_RwInt32();
			break;
		case 21:
			*(int *)&g_bloomHaloSize = it->GetAs_RwInt32();
			break;
		case 22:
			*(int *)&g_bloomAddFactor = it->GetAs_RwInt32();
			break;
		case 20:
			g_bloomEnable = it->GetAs_RwInt32() != 0;
			break;
		}
	}
}

}}
