#define ZOOSTERS_RWS_SENDMSG 1
#include "modules/tfb/shadowrender.h"
#include "modules/tfb/camera_views.h"
#include "modules/tfb/gameactor.h"
#include "modules/tfb/tfbshadowcamera.h"
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "framework/core/memoryhandler.h"
#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rwmacros.h"
#include "framework/toolkits/world/smartptrs/catomicptr.h"
#include "framework/toolkits/world/smartptrs/cclumpptr.h"
#include "framework/toolkits/math/maths.h"
#include "tfb/level/level.h"
#include "rw/rpcollis.h"
#include "framework/core/cresourcemanager.h"
#include "framework/toolkits/world/framehelper.h"
#include "framework/toolkits/world/atomichelper.h"
#include "framework/toolkits/world/camerahelper.h"
#include "framework/toolkits/world/lighthelper.h"
#include "framework/toolkits/world/clumphelper.h"
#include "framework/toolkits/world/factory.h"
#include "rw/rpplugins.h"
#include "tfb/render/shadow.h"
#include "tfb/render/tfbrenderstate.h"

// vtables of the two bases and the messages of the CEventId members
// GLOBAL: 0x5d4dac ShadowRender::vftable_for_CEventHandler
// GLOBAL: 0x5d4d9c ShadowRender::vftable_for_CAttributeHandler
// 0x444550
// 0x443500
// GLOBAL: 0x550250 ??0CEventId@RWS@@QAE@XZ
// GLOBAL: 0x464ef0 ??1CEventId@RWS@@QAE@XZ
// GLOBAL: 0x550760 ?UnLinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@@Z
// GLOBAL: 0x5506d0 ?UnRegisterMsg@CEventHandler@RWS@@SAXAAVCEventId@2@@Z
// GLOBAL: 0x5502f0 ?GetLinkedPriority@CEventHandler@RWS@@QAEHAAVCEventId@2@@Z
// GLOBAL: 0x6223d0 g_bigCircleTexture
// GLOBAL: 0x622408 g_headShadowTexture
// GLOBAL: 0x622404 g_shadowRender
// GLOBAL: 0x603500 g_shadowRenderClassId
// GLOBAL: 0x62241c RWS::ERROR_NOT_A_VALID_MESSAGE_ID
// GLOBAL: 0x6182f4 g_shadowCamera
// GLOBAL: 0x606cc4 g_detailedShadows
// GLOBAL: 0x617e7c g_cranePromptShown
// GLOBAL: 0x62a1f0 g_shadowSkip
// GLOBAL: 0x618330 g_msgCorpusShadow
// GLOBAL: 0x622414 g_shadowTexture
// GLOBAL: 0x5d4d70 DAT_005d4d70
// GLOBAL: 0x5d4d7c DAT_005d4d7c

using namespace RWS::TFB;

// The game's RenderWare is nothrow (its headers are extern "C"): cl 13.10 emits no exception state around these calls.

extern const char *g_shadowRenderClassId;   // 0x603500
extern const RwV3d DAT_005d4d70;           // 0x5d4d70 (1, 0, 0)
extern const RwV3d DAT_005d4d7c;           // 0x5d4d7c (0, 1, 0)

enum ShadowRenderAttribute {
	SHADOW_ATTR_CAMERA_MSG = 3,         // name of the "RwCamera*" message HandleEvents answers
	SHADOW_ATTR_FORWARD_MSG = 4,        // name of the message it sends on
	SHADOW_ATTR_CAMERA_MSG_PRIORITY = 5,
	SHADOW_ATTR_SOFTNESS = 6,
	SHADOW_ATTR_UNK2C = 7,
	SHADOW_ATTR_SIZE_SHIFT = 8,
	SHADOW_ATTR_ENABLE_FADING = 9,
	SHADOW_ATTR_FADE_RADIUS = 10,
	SHADOW_ATTR_UNK68 = 11,
	SHADOW_ATTR_UNK6C = 12,
	SHADOW_ATTR_UNK70 = 13,
	SHADOW_ATTR_UNK74 = 14,
	SHADOW_ATTR_WORLD_RENDER_ORDER = 15 // 1: the world is drawn back to front while the shadows are made, else front to back
};

// RpWorld renderOrder values (RW's RpWorldRenderOrder: 1 front to back, 2 back to front).
enum { RENDER_FRONT_TO_BACK = 1, RENDER_BACK_TO_FRONT = 2 };

#include "modules/tfb/shadowrender_views.h"

// 0x442ea0
ShadowRender::ShadowRender(const RWS::CAttributePacket &) : m_softness(0.6f), m_unk2c(0), m_sizeShift(9)
{
	m_worldRenderOrder = RENDER_FRONT_TO_BACK;
	g_shadowRender = this;
	m_unk68 = 0;
	m_unk70 = 0x3F400000;
	m_unk6c = 0x3F400000;
	m_shadowRaster = 0;
	m_zRaster = 0;
	m_blurRaster = 0;
	m_blurCamera = 0;
	m_subRaster = 0;
	m_zSubRaster = 0;
	g_headShadowTexture = RwTextureRead("HEADFRAME_SHADOW", 0);
	if (g_headShadowTexture) {
		TextureView(g_headShadowTexture)->filter = rwFILTERLINEAR;
		TextureView(g_headShadowTexture)->addressing = 0x33;   // addressU = addressV = clamp (3 | 3 << 4)
	}
	g_bigCircleTexture = RwTextureRead("BIGCIRCLE", 0);
}

// 0x443510
ShadowRender::~ShadowRender()
{
	if (g_bigCircleTexture) {
		RwTextureDestroy(g_bigCircleTexture);
		g_bigCircleTexture = 0;
	}
	DestroyRasters();
	UnLinkMsg(m_evtCamera);
	UnRegisterMsg(m_evtCamera);
	UnRegisterMsg(m_evtForward);
	g_shadowRender = 0;
	if (g_headShadowTexture) {
		RwTextureDestroy(g_headShadowTexture);
		g_headShadowTexture = 0;
	}
}

// 0x442fa0
void ShadowRender::DestroyRasters()
{
	if (m_shadowRaster) {
		RwRasterDestroy(m_shadowRaster);
		m_shadowRaster = 0;
	}
	if (m_zRaster) {
		RwRasterDestroy(m_zRaster);
		m_zRaster = 0;
	}
	if (m_subRaster) {
		RwRasterDestroy(m_subRaster);
		m_subRaster = 0;
	}
	if (m_zSubRaster) {
		RwRasterDestroy(m_zSubRaster);
		m_zSubRaster = 0;
	}
	if (m_blurRaster) {
		RwRasterDestroy(m_blurRaster);
		m_blurRaster = 0;
	}
	if (m_blurCamera) {
		ShadowCameraDestroy(m_blurCamera);
		m_blurCamera = 0;
	}
}

// Shadow map: a camera texture, raster format 0x505; the sub rasters of both are inset by 3 pixels (so the border stays white).
// 0x443020
void ShadowRender::CreateRasters()
{
	DestroyRasters();
	int size = 1 << m_sizeShift;
	m_shadowRaster = RwRasterCreate(size, size, 0, 0x505);
	m_zRaster = RwRasterCreate(size, size, 0, 1);
	RwRasterView *r = RasterView(m_shadowRaster);
	m_subRaster = RwRasterCreate(0, 0, r->depth, (r->type & 7) | (r->format << 8));
	RwRect rect;
	rect.y = 3;
	rect.x = 3;
	rect.w = size - 6;
	rect.h = size - 6;
	RwRasterSubRaster(m_subRaster, m_shadowRaster, &rect);
	if (m_zRaster) {
		m_zSubRaster = RwRasterCreate(0, 0, 0, 1);
		RwRasterSubRaster(m_zSubRaster, m_zRaster, &rect);
	}
	m_blurCamera = ShadowCameraCreate(size);
	m_blurRaster = RwRasterCreate(size, size, 0, 0x505);
}

// 0x4434c0
void ShadowRender::HandleEvents(RWS::CMsg &msg)
{
	if (msg.Id == m_evtCamera.pEventId) {
		RWS::CMsg out;
		out.Id = m_evtForward.pEventId;
		out.pData = msg.pData;
		RWS::SendMsg(out);
	}
}

// Updates `camera` around the walk of the actor tree with the shadow render states (no depth, no culling, no fog).
// 0x443680
static __declspec(noinline) void ShadowCameraRenderTree(RwCamera *camera, GameActor *root)
{
	RwFrameGetLTM(RwCameraGetFrame(camera))->flags = 3;
	if (RwCameraBeginUpdate(camera)) {
		RpLODAtomicSetCamera(camera);
		SetBlendAndAlphaTest(rwBLENDSRCALPHA, rwBLENDINVSRCALPHA, 2);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
		RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
		ShadowRenderActorTree(root);
		RwCameraEndUpdate(camera);
	}
}

// 0x443700
void ShadowRender::RenderShadowMap(RwCamera *camera, GameActor *root, bool blur)
{
	RwRGBA color;
	color.red = 0xff;
	color.green = 0xff;
	color.blue = 0xff;
	color.alpha = 0;
	RwCameraClear(camera, &color, rwCAMERACLEARIMAGE | rwCAMERACLEARZ);
	ShadowCameraRenderTree(camera, root);
	if (blur) {
		if (m_blurRaster)
			ShadowBlurRaster(m_shadowRaster, m_blurRaster, m_blurCamera, 1);
	}
}

// ATTRIBUTES (class id g_shadowRenderClassId): the settings of the shadow map and the messages HandleEvents uses.
// 0x444090
void ShadowRender::HandleAttributes(const RWS::CAttributePacket &attr)
{
	bool recreate;
	RWS::CAttributeHandler::HandleAttributes(attr);
	RWS::CAttributeCommandIterator it(attr, g_shadowRenderClassId);
	recreate = false;
	for (; !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case SHADOW_ATTR_UNK68:
			m_unk68 = it->GetAs_RwInt32();
			break;
		case SHADOW_ATTR_UNK6C:
			m_unk6c = it->GetAs_RwInt32();
			break;
		case SHADOW_ATTR_UNK70:
			m_unk70 = it->GetAs_RwInt32();
			break;
		case SHADOW_ATTR_UNK74:
			m_unk74 = it->GetAs_RwInt32();
			break;
		case SHADOW_ATTR_SOFTNESS:
			SetBits(m_softness, it->GetAs_RwInt32());
			break;
		case SHADOW_ATTR_UNK2C:
			m_unk2c = it->GetAs_RwInt32();
			break;
		case SHADOW_ATTR_SIZE_SHIFT:
			m_sizeShift = it->GetAs_RwInt32();
			recreate = true;
			break;
		case SHADOW_ATTR_ENABLE_FADING:
			m_enableFading = it->GetAs_RwInt32();
			break;
		case SHADOW_ATTR_FADE_RADIUS:
			SetBits(m_fadeRadius, it->GetAs_RwInt32());
			break;
		case SHADOW_ATTR_CAMERA_MSG: {
			int prio = GetLinkedPriority(m_evtCamera);
			if (prio < 0)
				prio = 0xFFFF;
			UnLinkMsg(m_evtCamera);
			UnRegisterMsg(m_evtCamera);
			RegisterMsg(m_evtCamera, it->GetAs_RwChar_ptr(), "RwCamera*");
			LinkMsg(m_evtCamera, "RwCamera*", (unsigned short)prio);
			break;
		}
		case SHADOW_ATTR_CAMERA_MSG_PRIORITY:
			UnLinkMsg(m_evtCamera);
			LinkMsg(m_evtCamera, "RwCamera*", it->GetAs_RwUInt16());
			break;
		case SHADOW_ATTR_FORWARD_MSG:
			ReplaceRegisteredMsg(m_evtForward, it->GetAs_RwChar_ptr(), "RwCamera*");
			break;
		case SHADOW_ATTR_WORLD_RENDER_ORDER:
			switch (it->GetAs_RwInt32()) {
			case 1:
				m_worldRenderOrder = RENDER_BACK_TO_FRONT;
				break;
			default:
				m_worldRenderOrder = RENDER_FRONT_TO_BACK;
			}
			break;
		}
	}
	if (recreate)
		CreateRasters();
}

// Shadows of one frame, after the opaque world pass. A clone of the main camera, turned to the shadow camera's pitch and
// heading and rendering into the shadow map's sub rasters, is the shadow camera. CORPUS_SHADOW_PROCESS goes out twice
// (detailed shadows, then the BIGCIRCLE rings if that texture exists). Skipped while handler flag bit 0 is set, and
// unless detailed shadows or one of the two override flags is on.
// No EH state: the byte at [esp+40h] is ShadowPass::m_detailed (1 for the first SendMsg, 0 for the second).
// Near-match of 0x4442e0: the two clone frame / z buffer stores are scheduled before `add esp,48h` / `push 3` in ours, interleaved with the
// RwCameraClear pushes in the game.
void ShadowRender::RenderShadows(RwCamera *camera)
{
	if (g_detailedShadows || g_cranePromptShown || g_shadowSkip) {
		if (!(GetFlags() & 1)) {
			float oldFar = CameraView(camera)->farPlane;
			RwCameraSetFarClipPlane(camera, (oldFar - CameraView(camera)->nearPlane) * 1.175f + CameraView(camera)->nearPlane);
			RpWorld *world = RwCameraGetWorld(camera);
			((RpWorldRenderOrderView *)world)->renderOrder = m_worldRenderOrder;
			RwCamera *clone = RwCameraClone(camera);
			RwCameraSetProjection(clone, rwPARALLEL);
			RwCameraSetNearClipPlane(clone, 1.0f);
			RwFrame *frame = RwFrameCreate_thunk();
			_rwObjectHasFrameSetFrame(clone, frame);
			RwFrameRotate(frame, &DAT_005d4d70, g_shadowCamera->m_pitch, rwCOMBINEREPLACE);
			RwFrameRotate(frame, &DAT_005d4d7c, g_shadowCamera->m_heading, rwCOMBINEPOSTCONCAT);
			CameraView(clone)->zBuffer = m_zRaster;
			CameraView(clone)->frameBuffer = m_shadowRaster;
			RwRGBA white;
			white.red = 0xff;
			white.green = 0xff;
			white.blue = 0xff;
			white.alpha = 0;
			RwCameraClear(clone, &white, rwCAMERACLEARIMAGE | rwCAMERACLEARZ);
			CameraView(clone)->zBuffer = m_zSubRaster;
			CameraView(clone)->frameBuffer = m_subRaster;
			ShadowPass pass;   // its vtable (game 0x5d4dc8, slot 0 = Visit) is the one this TU emits: no GLOBAL line, the port's stub copy has no slot 0
			pass.m_world = world;
			pass.m_camera = camera;
			pass.m_shadowCamera = clone;
			pass.m_lightDir = RwCameraGetFrame(camera)->modelling.at;
			pass.m_lightDir.y = 0.0f;
			RwV3dNormalize(&pass.m_lightDir, &pass.m_lightDir);
			RwTextureSetRaster(g_shadowTexture, m_shadowRaster);
			RWS::CMsg msg;
			msg.Id = g_msgCorpusShadow.pEventId;
			msg.pData = &pass;
			pass.m_detailed = 1;
			RWS::SendMsg(msg);
			if (g_bigCircleTexture) {
				pass.m_detailed = 0;
				RWS::SendMsg(msg);
			}
			RpWorldRemoveCamera(world, clone);
			_rwObjectHasFrameSetFrame(clone, 0);
			RWS::FrameHelper::FrameDestroy(frame);
			RwCameraDestroy(clone);
			RwCameraSetFarClipPlane(camera, oldFar);
		}
	}
}
