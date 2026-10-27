// PS2 base order: CEventHandler@0, CAttributeHandler@8, CSystemCommands@36, LinearAllocationPolicy@40 (CSystemCommands NOT first).
// Vtables 0x5d4dac (CEventHandler part: ~ 0x444550, HandleEvents 0x4434c0) and 0x5d4d9c (CAttributeHandler part at +8:
// thunk 0x443500, HandleAttributes 0x444090, base, base). Registered size 0x78 (RegisterFactory 0x5c6180).
#ifndef ZOOSTERS_MODULES_TFB_SHADOWRENDER_H
#define ZOOSTERS_MODULES_TFB_SHADOWRENDER_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"

namespace RWS {
namespace TFB {
class GameActor;
}
}

class ShadowRender : public RWS::CEventHandler, public RWS::CAttributeHandler, public RWS::CSystemCommands, public RWS::LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(ShadowRender);
	ShadowRender(const RWS::CAttributePacket &attr);
	~ShadowRender();
	virtual void HandleEvents(RWS::CMsg &msg);
	virtual void HandleAttributes(const RWS::CAttributePacket &attr);

	// Renders the shadows of the current frame into the shadow map (called from CTFBWorld's opaque world pass): sends CORPUS_SHADOW_PROCESS
	// twice with a ShadowPass,
	// then the shadow camera is destroyed.
	void RenderShadows(RwCamera *camera);
	// Clears `camera`, renders the clump tree of `root` into it with the shadow render states (0x443700, ret 12) and, if `blur`,
	// blurs the shadow map with the second raster.
	void RenderShadowMap(RwCamera *camera, RWS::TFB::GameActor *root, bool blur);
	void DestroyRasters();
	void CreateRasters();

	float m_softness;            // +0x28 attribute 6, script "softness": the strength of the projected shadow (0.6)
	int m_unk2c;  // +0x2c attribute 7; 0 from the constructor; written by the constructor and HandleAttributes, never read
	int m_sizeShift;             // +0x30 attribute 8: the shadow map is (1 << m_sizeShift) pixels square (9: 512)
	int m_enableFading;          // +0x34 attribute 9, script "fading enabled" (no code in the game reads it)
	float m_fadeRadius;          // +0x38 attribute 10, script "fade radius", in feet (stored * 12; no code in the game reads it)
	RWS::CEventId m_evtCamera;   // +0x3c attribute 3 (name) / 5 (priority): the "RwCamera*" message HandleEvents answers
	RWS::CEventId m_evtForward;  // +0x44 attribute 4: the "RwCamera*" message HandleEvents sends on, with the same camera
	int m_worldRenderOrder;      // +0x4c RpWorld renderOrder RenderShadows sets while the world is drawn (attribute 15: 1 front to back, 2 back to front)
	RwRaster *m_shadowRaster;    // +0x50 the shadow map: camera texture (1 << m_sizeShift square), g_shadowTexture's raster
	RwRaster *m_zRaster;         // +0x54 its z-buffer raster
	RwRaster *m_subRaster;       // +0x58 sub raster of m_shadowRaster, inset by 3 pixels on each side
	RwRaster *m_zSubRaster;      // +0x5c sub raster of m_zRaster (same rectangle)
	RwRaster *m_blurRaster;      // +0x60 second camera texture of the same size: the ping-pong partner of the blur pass 0x442d00
	RwCamera *m_blurCamera;      // +0x64 camera with its own z buffer (0x442cb0) that draws the blur pass
	int m_unk68;  // +0x68 attribute 11; 0 from the constructor; never read
	int m_unk6c;  // +0x6c attribute 12; 0.75f from the constructor (float bits); never read
	int m_unk70;  // +0x70 attribute 13; 0.75f from the constructor (float bits); never read
	int m_unk74;  // +0x74 attribute 14; not set by the constructor (0 in the tour); never read
};
ZOOSTERS_ASSERT_SIZE(ShadowRender, 0x78);
ZOOSTERS_ASSERT_OFFSET(ShadowRender, m_softness, 0x28);
ZOOSTERS_ASSERT_OFFSET(ShadowRender, m_sizeShift, 0x30);
ZOOSTERS_ASSERT_OFFSET(ShadowRender, m_evtCamera, 0x3c);
ZOOSTERS_ASSERT_OFFSET(ShadowRender, m_evtForward, 0x44);
ZOOSTERS_ASSERT_OFFSET(ShadowRender, m_worldRenderOrder, 0x4c);
ZOOSTERS_ASSERT_OFFSET(ShadowRender, m_shadowRaster, 0x50);
ZOOSTERS_ASSERT_OFFSET(ShadowRender, m_blurCamera, 0x64);
ZOOSTERS_ASSERT_OFFSET(ShadowRender, m_unk68, 0x68);

extern ShadowRender *g_shadowRender;   // the instance (0 while there is none)

#endif
