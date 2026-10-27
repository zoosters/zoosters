// Opening and closing the shadow system: the shadow texture, the sector pipeline that projects it, and ShadowRender's cameras.
#include "tfb/render/shadow.h"
#include "modules/tfb/camera_views.h"
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
#include "rw/rwd3d8.h"

// The one member of D3DCAPS8 read here.
struct D3DCaps8View {
	char pad00[0x98];
	unsigned maxSimultaneousTextures;
};
ZOOSTERS_ASSERT_OFFSET(D3DCaps8View, maxSimultaneousTextures, 0x98);

// GLOBAL: 0x622400 g_shadowBuffer
// GLOBAL: 0x622410 g_shadowCasterPipeline
// GLOBAL: 0x622414 g_shadowTexture
// GLOBAL: 0x622418 g_shadowPipeline
// GLOBAL: 0x43e6f0 FUN_0043e6f0
// GLOBAL: 0x44eff0 RwFrameCreate_thunk

void *FUN_0043e6f0();  // the folded `return 0` (the creator of g_shadowCasterPipeline in the game's source)
void *MemAlloc(size_t size);          // 0x550170

// A parallel camera of its own frame with a size x size z raster; 0 on failure.
// 0x442cb0
RwCamera *ShadowCameraCreate(int size)
{
	RwCamera *camera = RwCameraCreate();
	if (camera) {
		_rwObjectHasFrameSetFrame(camera, RwFrameCreate_thunk());
		if (RwCameraGetFrame(camera)) {
			CameraView(camera)->zBuffer = RwRasterCreate(size, size, 0, 1);
			RwCameraSetProjection(camera, rwPARALLEL);
			return camera;
		}
	}
	ShadowCameraDestroy(camera);
	return 0;
}

// Destroys a camera made by ShadowCameraCreate. The frame buffer is only dropped.
// 0x442c50
void ShadowCameraDestroy(RwCamera *camera)
{
	if (camera) {
		RwFrame *frame = RwCameraGetFrame(camera);
		if (frame) {
			_rwObjectHasFrameSetFrame(camera, (RwFrame *)0);
			RWS::FrameHelper::FrameDestroy(frame);
		}
		if (CameraView(camera)->zBuffer) {
			RwRaster *zBuffer = CameraView(camera)->zBuffer;
			CameraView(camera)->zBuffer = 0;
			RwRasterDestroy(zBuffer);
		}
		if (CameraView(camera)->frameBuffer)
			CameraView(camera)->frameBuffer = 0;
		RwCameraDestroy(camera);
	}
}

// Creates the sector pipeline (a device without multitexturing gets a 0xD2F0-byte buffer instead and no shadow texture) and the
// shadow texture (linear filter, clamped). False when the texture could not be made.
// 0x4430f0
bool ShadowSystemOpen()
{
	g_shadowCasterPipeline = (RxPipeline *)FUN_0043e6f0();
	g_shadowPipeline = CreateShadowPipeline();
	if (!g_shadowPipeline) {
		g_shadowBuffer = MemAlloc(0xD2F0);
		return true;
	}
	RwTexture *texture = RwTextureCreate((RwRaster *)0);
	g_shadowTexture = texture;
	if (texture) {
		TextureView(texture)->filter = rwFILTERLINEAR;
		TextureView(texture)->addressing = 0x33;   // addressU = addressV = clamp (3 | 3 << 4)
		return true;
	}
	return false;
}

// 0x443140
void ShadowSystemClose()
{
	if (g_shadowTexture) {
		RwTextureSetRaster(g_shadowTexture, 0);
		RwTextureDestroy(g_shadowTexture);
		g_shadowTexture = 0;
	}
	if (g_shadowPipeline) {
		_rxPipelineDestroy(g_shadowPipeline);
	} else if (g_shadowBuffer) {
		operator delete(g_shadowBuffer);
		g_shadowBuffer = 0;
	}
	RxPipeline *casterPipeline = g_shadowCasterPipeline;
	g_shadowPipeline = 0;
	if (casterPipeline) {
		_rxPipelineDestroy(casterPipeline);
		g_shadowCasterPipeline = 0;
	}
}

// A D3D8 all-in-one world sector pipeline rendering through ShadowPipelineRender; 0 unless the device can blend two textures at once or a step fails.
// 0x444a50
RxPipeline *CreateShadowPipeline()
{
	if (((const D3DCaps8View *)RwD3D8GetCaps())->maxSimultaneousTextures > 1) {
		RxPipeline *pipe = RxPipelineCreate();
		if (pipe) {
			RxLockedPipe *locked = RxPipelineLock(pipe);
			if (locked) {
				RxNodeDefinition *def = RxNodeDefinitionGetD3D8WorldSectorAllInOne();
				RxLockedPipeUnlock(RxLockedPipeAddFragment(locked, 0, def, 0));
				RxPipelineNode *node = RxPipelineFindNodeByName(pipe, def->name, 0, 0);
				RxD3D8AllInOneSetRenderCallBack(node, ShadowPipelineRender);
				return pipe;
			}
			_rxPipelineDestroy(pipe);
		}
	}
	return 0;
}
