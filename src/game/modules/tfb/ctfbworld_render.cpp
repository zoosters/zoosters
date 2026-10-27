#include "modules/tfb/ctfbworld.h"
#include "modules/tfb/camera_views.h"
#include "modules/tfb/renderqueue.h"
#include "modules/tfb/shadowrender.h"
#include "tfb/level/level.h"
#include "tfb/render/tfbrenderstate.h"
#include "tfb/render/tfbworld.h"
#include "tfb/render/yonbox.h"
#include "rw/rwd3d8.h"
#include "rw/rpplugins.h"
#include "tfb/render/occluder.h"
#include "tfb/render/postfx.h"

// GLOBAL: 0x629724 RWS::CLevel::sm_pOpaqueWorld
// GLOBAL: 0x629728 RWS::CLevel::sm_pTranslucentWorld
// GLOBAL: 0x629730 RWS::CLevel::sm_pPreOpaqueWorld
// GLOBAL: 0x629734 RWS::CLevel::sm_pSkyWorld
// GLOBAL: 0x629738 RWS::CLevel::sm_pFogWorld
// GLOBAL: 0x62971c g_lightList
// GLOBAL: 0x6296f8 g_fogParamA
// GLOBAL: 0x6296f4 g_fogParamB
// GLOBAL: 0x6296f0 g_fogFlag
// GLOBAL: 0x629704 g_vertFogColor
// GLOBAL: 0x6296fc g_levelFogParam
// GLOBAL: 0x629700 g_fogMode
// GLOBAL: 0x629710 g_nearClip
// GLOBAL: 0x62970c g_farClip
// GLOBAL: 0x617d5c g_pixelFogEnable
// GLOBAL: 0x617d60 g_pixelFogColor
// GLOBAL: 0x618150 g_fogColorAlpha
// GLOBAL: 0x617d50 g_pixelFogEnd
// GLOBAL: 0x618110 g_renderQueueHead
// GLOBAL: 0x618118 g_renderQueueCount
// GLOBAL: 0x618128 RWS::TFB::g_noMessageId
// GLOBAL: 0x622404 g_shadowRender
extern unsigned char g_pixelFogEnable;     // 0x617d5c (post-effect state, g/postfx_411.cpp)
extern unsigned char g_pixelFogColor[4];   // 0x617d60 r, g, b, a
extern unsigned g_fogColorAlpha;
extern float g_pixelFogEnd;                // 0x617d50 (the float the post effect stores; the render state takes its bits)
extern ShadowRender *g_shadowRender;

float GetPixelFogStartDistance();                                           // 0x411180 pixel fog start distance (leaves the float on the x87 stack)
void ApplyCameraFog(RwCamera *camera);                          // 0x41ac20 (modules/tfb/ctfbworld_fog.cpp)
void Corpus_SendPhysicsMessage();
#ifdef ZOOSTERS_WIDESCREEN
extern "C" float port_wide_underlay_zoom(void *camera);
extern "C" void port_wide_clear_color(void *camera, RwRGBA *color);   // port/fixes/wide_scenery.cpp
#endif

namespace RWS { namespace FX { void QueueVisibleParticles(RwCamera *cam); } }   // 0x40b3d0 (modules/fx/gameparticle_life.cpp)
void CameraCopyFrustumPlanes(RwCamera *camera);

namespace RWS { namespace TFB {

// The world's sector render order (RpWorld::renderOrder at +0x0c: RW's RpWorldRenderOrder, 1 front to back, 2 back to front; the
// game writes the member).
enum WorldRenderOrder { WORLD_FRONT_TO_BACK = 1, WORLD_BACK_TO_FRONT = 2 };
static inline void SetWorldRenderOrder(RpWorld *world, WorldRenderOrder order)
{
	((RpWorld *)world)->head.renderOrder = order;
}

// RwMatrix::flags of a camera frame's ltm: orthonormal (RW's rwMATRIXTYPEORTHONORMAL), set before every begin-update.
static const unsigned kMatrixOrthonormal = 3;
static const float kSkyFarClip = 120000.0f;
// RwCameraClear modes: the sky pass clears image, z and stencil.
static const int kClearAll = rwCAMERACLEARIMAGE | rwCAMERACLEARZ | rwCAMERACLEARSTENCIL;
// Direct3D 8 render states D3DRS_FOGSTART / D3DRS_FOGEND, which RW's fog states do not cover (the pixel fog sets them directly).
enum { kD3dRsFogStart = 36, kD3dRsFogEnd = 37 };

// The world arrives in esi (register argument): move the pending lights into it
// 0x41abe0
static __declspec(noinline) void MovePendingLightsToWorld(RpWorld *world)
{
	if (world) {
		while (g_lightList.next != &g_lightList) {
			RwLLLink *n = g_lightList.next;
			n->prev->next = n->next;
			RpLight *light = RpLightFromWorldLink(n);
			RpLightGetWorldLink(light)->next->prev = RpLightGetWorldLink(light)->prev;
			RpWorldAddLight(world, light);
		}
	}
}

// 0x41aca0
void SetFogRenderStates(RwCamera *camera)
{
	if (g_pixelFogEnable == 1) {
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)1);
		RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)rwFOGTYPELINEAR);
		unsigned color = (((g_pixelFogColor[0] << 8) | g_pixelFogColor[1]) << 8 | g_pixelFogColor[2]) | g_fogColorAlpha;
		RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void *)(intptr_t)color);
		RwRenderStateSet(rwRENDERSTATEFOGDENSITY, (void *)&g_levelFogParam);
		float fogStart = GetPixelFogStartDistance();
		RwD3D8SetRenderState(kD3dRsFogStart, *(unsigned *)&fogStart);
		RwD3D8SetRenderState(kD3dRsFogEnd, *(const int *)&g_pixelFogEnd);   // the bits of the float
	} else if (g_fogMode == 0) {
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
	} else {
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)1);
		if (!RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)(intptr_t)g_fogMode))
			RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)rwFOGTYPELINEAR);
		RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void *)(intptr_t)g_vertFogColor);
		RwRenderStateSet(rwRENDERSTATEFOGDENSITY, (void *)&g_levelFogParam);
	}
}

// 0x41bdb0
void CTFBWorld::RenderTransparentPass(RwCamera *camera)
{
	if (!CLevel::sm_pTranslucentWorld) {
		MovePendingLightsToWorld(CLevel::sm_pOpaqueWorld);
		return;
	}
	ApplyCameraFog(camera);
	SetWorldRenderOrder(CLevel::sm_pTranslucentWorld, WORLD_BACK_TO_FRONT);
	RpWorldAddCamera(CLevel::sm_pTranslucentWorld, camera);
	RwFrameGetLTM(CameraFrame(camera))->flags = kMatrixOrthonormal;
	if (RwCameraBeginUpdate(camera)) {
		RpLODAtomicSetCamera(camera);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)1);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)1);
		RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
		SetBlendAndAlphaTest(rwBLENDSRCALPHA, rwBLENDINVSRCALPHA, 2);
		SetFogRenderStates(camera);
		RenderSectors(camera, CLevel::sm_pTranslucentWorld);
		FlushRenderQueue();
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
		RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)rwFOGTYPENAFOGTYPE);
		RwCameraEndUpdate(camera);
	}
	RpWorldRemoveCamera(CLevel::sm_pTranslucentWorld, camera);
	RpWorldForAllLights(CLevel::sm_pTranslucentWorld, LevelMoveLightCB, CLevel::sm_pOpaqueWorld);
}

// 0x41bec0
void CTFBWorld::RenderSkyPass(RwCamera *camera)
{
#ifdef ZOOSTERS_WIDESCREEN   // widescreen port build only (port/fixes/wide_scenery.cpp): a sky-coloured clear where the sky card ends
	RwRGBA clearColor = m_clearColor;
	port_wide_clear_color(camera, &clearColor);
	RwCameraClear(camera, &clearColor, kClearAll);
#else
	RwCameraClear(camera, &m_clearColor, kClearAll);
#endif
	if (CLevel::sm_pSkyWorld) {
		RwCameraSetFarClipPlane(camera, kSkyFarClip);
		RwFrame *frame = CameraFrame(camera);
		RwV3d *pos = &frame->modelling.pos;
		RwV3d saved = *pos;
		pos->x = 0;
		pos->y = 0;
		pos->z = 0;
		RwFrameUpdateObjects(frame);
		SetWorldRenderOrder(CLevel::sm_pSkyWorld, WORLD_BACK_TO_FRONT);
		RpWorldAddCamera(CLevel::sm_pSkyWorld, camera);
		RwFrameGetLTM(CameraFrame(camera))->flags = kMatrixOrthonormal;
		if (RwCameraBeginUpdate(camera)) {
			RpLODAtomicSetCamera(camera);
			SetBlendAndAlphaTest(rwBLENDONE, rwBLENDZERO, 2);
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
			RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
			RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
			RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
			RenderSectors(camera, CLevel::sm_pSkyWorld);
			RwCameraEndUpdate(camera);
		}
		RpWorldRemoveCamera(CLevel::sm_pSkyWorld, camera);
		*pos = saved;
		RwFrameUpdateObjects(frame);
	}
}

// 0x41bff0
void CTFBWorld::RenderFogPass(RwCamera *camera)
{
	if (CLevel::sm_pFogWorld) {
		RwCameraSetFarClipPlane(camera, g_fogParamA);
		if (!g_pixelFogEnable && g_fogFlag)
			CameraView(camera)->fogPlane = g_fogParamB;
		SetWorldRenderOrder(CLevel::sm_pFogWorld, WORLD_BACK_TO_FRONT);
		RpWorldAddCamera(CLevel::sm_pFogWorld, camera);
		RwFrameGetLTM(CameraFrame(camera))->flags = kMatrixOrthonormal;
		if (RwCameraBeginUpdate(camera)) {
			RpLODAtomicSetCamera(camera);
			SetBlendAndAlphaTest(rwBLENDSRCALPHA, rwBLENDINVSRCALPHA, 2);
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)1);
			RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)1);
			RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
			if (!g_pixelFogEnable) {
				if (g_fogFlag == 0) {
					RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
				} else {
					RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)1);
					if (!RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)(intptr_t)g_fogFlag))
						RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)rwFOGTYPELINEAR);
					RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void *)(intptr_t)g_vertFogColor);
					RwRenderStateSet(rwRENDERSTATEFOGDENSITY, (void *)&g_levelFogParam);
				}
			}
			RenderSectors(camera, CLevel::sm_pFogWorld);
			RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
			RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)rwFOGTYPENAFOGTYPE);
			RwCameraEndUpdate(camera);
		}
		RpWorldRemoveCamera(CLevel::sm_pFogWorld, camera);
		RwRGBA black;
		black.red = 0;
		black.green = 0;
		black.blue = 0;
		black.alpha = 0;
		RwCameraClear(camera, &black, rwCAMERACLEARZ);
	}
}

// 0x41b300
void CTFBWorld::RenderWorldPass(RwCamera *camera, RpWorld *world)
{
	ApplyCameraFog(camera);
	SetWorldRenderOrder(world, WORLD_FRONT_TO_BACK);
	RpWorldAddCamera(world, camera);
	RwFrameGetLTM(CameraFrame(camera))->flags = kMatrixOrthonormal;
	if (RwCameraBeginUpdate(camera)) {
		RpLODAtomicSetCamera(camera);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)1);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)1);
		RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
		SetBlendAndAlphaTest(rwBLENDSRCALPHA, rwBLENDINVSRCALPHA, 2);
		SetFogRenderStates(camera);
		(this->*m_renderSectors)(camera, world);
		if (world == CLevel::sm_pOpaqueWorld) {
			if (m_yonBox)
				m_yonBox->QueueVisibleAtomics(camera);
			Corpus_SendPhysicsMessage();
			RWS::FX::QueueVisibleParticles(camera);
			if (!CLevel::sm_pTranslucentWorld)
				FlushRenderQueue();
			CameraCopyFrustumPlanes(camera);
		}
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
		RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)rwFOGTYPENAFOGTYPE);
		StencilWrite(false);
		RwCameraEndUpdate(camera);
	}
	if (world == CLevel::sm_pOpaqueWorld) {
		RpWorldForAllLights(world, LevelMoveLightCB, CLevel::sm_pTranslucentWorld);
		g_shadowRender->RenderShadows(camera);
	}
	RpWorldRemoveCamera(world, camera);
}

// Near-miss, not kept (376 B vs 377 B): the game loads CLevel::sm_pOpaqueWorld into eax, pushes it and copies it to esi afterwards; cl loads
// straight into esi.
// 0x41c190
void CTFBWorld::RenderFrame(RwCamera *camera)
{
	if (camera) {
		OccluderNode planeBuffer[0x2260 / 0x58];   // the game's 100 nodes; OccluderNode is 0x60 bytes on x64 (the char buffer held 91: stack overrun)
		RwCameraSetNearClipPlane(camera, g_nearClip);
		_rwFrameSyncDirty();
		RpWorld *opaqueWorld = CLevel::sm_pOpaqueWorld;
		BuildOccluderPlanes((CamB *)camera, TFBWorld_GetRecords(opaqueWorld), TFBWorld_GetRecordCount(opaqueWorld) * 4, (OccluderNode *)planeBuffer);
		g_renderQueueHead = 0;
		g_renderQueueCount = 0;
		RenderSkyPass(camera);
		if (m_yonIgnore)
			RenderFogPass(camera);
		UpdatePixelFogCutoffAndRange(camera, g_shadowRender ? g_shadowRender->m_shadowRaster : 0);
		if (g_pixelFogEnable == 1)
			DrawPixelFogQuad(camera);
		RwCameraSetFarClipPlane(camera, g_farClip);
		UpdatePixelFogCutoffAndRange(camera, g_shadowRender ? g_shadowRender->m_shadowRaster : 0);
#ifdef ZOOSTERS_WIDESCREEN   // widescreen port build only (port/fixes/wide_scenery.cpp): scenery that ends outside the 4:3 frame
		float underlayZoom = port_wide_underlay_zoom(camera);
		if (underlayZoom > 1.0f) {
			// The worlds once more under the real pass, zoomed in so the 4:3 frame's content reaches the screen edges; the depth clear lets the real pass draw over it
			RwV2d vw = *RwCameraGetViewWindow(camera), zoomed;
			zoomed.x = vw.x / underlayZoom;
			zoomed.y = vw.y / underlayZoom;
			RwCameraSetViewWindow(camera, &zoomed);
			RpWorld *worlds[3] = { CLevel::sm_pPreOpaqueWorld, CLevel::sm_pOpaqueWorld, CLevel::sm_pTranslucentWorld };
			for (int i = 0; i < 3; i++) {
				RpWorld *world = worlds[i];
				if (!world)
					continue;
				ApplyCameraFog(camera);
				SetWorldRenderOrder(world, i == 2 ? WORLD_BACK_TO_FRONT : WORLD_FRONT_TO_BACK);
				RpWorldAddCamera(world, camera);
				RwFrameGetLTM(CameraFrame(camera))->flags = kMatrixOrthonormal;
				if (RwCameraBeginUpdate(camera)) {
					RpLODAtomicSetCamera(camera);
					RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)1);
					RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)1);
					RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
					SetBlendAndAlphaTest(rwBLENDSRCALPHA, rwBLENDINVSRCALPHA, 2);
					SetFogRenderStates(camera);
					(this->*m_renderSectors)(camera, world);
					RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
					RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)rwFOGTYPENAFOGTYPE);
					RwCameraEndUpdate(camera);
				}
				RpWorldRemoveCamera(world, camera);
			}
			RwCameraSetViewWindow(camera, &vw);
			g_renderQueueHead = 0;
			g_renderQueueCount = 0;
			RwRGBA none = { 0, 0, 0, 0 };
			RwCameraClear(camera, &none, rwCAMERACLEARZ);
		}
#endif
		if (CLevel::sm_pPreOpaqueWorld)
			RenderWorldPass(camera, CLevel::sm_pPreOpaqueWorld);
		if (!CLevel::sm_pOpaqueWorld)
			MovePendingLightsToWorld(CLevel::sm_pTranslucentWorld);
		else
			RenderWorldPass(camera, CLevel::sm_pOpaqueWorld);
		SendLinkedMessage(m_afterOpaqueEvent, camera);
		RenderTransparentPass(camera);
		DrawBloomIfEnabled(camera);
		SendLinkedMessage(m_afterRenderEvent, camera);
	}
}

}}
