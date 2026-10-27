// Shadow system, render side: opens and closes it, renders actor trees into the shadow map, blurs it and draws it on the world with a D3D8 sector pipeline.
// Per frame ShadowRender::RenderShadows sends CORPUS_SHADOW_PROCESS with a ShadowPass; ShadowPass::Visit fits the shadow camera around each grounded actor and draws one of the BIGCIRCLE ring (actors visible on radar, second message), the detailed shadow (first message: the actor tree rendered into the shadow map and projected onto the world sectors both cameras see) or, far away, the HEADFRAME_SHADOW blob.
#ifndef ZOOSTERS_TFB_RENDER_SHADOW_H
#define ZOOSTERS_TFB_RENDER_SHADOW_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "framework/core/ceventhandler.h"

struct RxPipeline;
namespace RWS {
namespace TFB {
class GameActor;
struct Collider;
}
}

extern RwTexture *g_shadowTexture;  // the shadow map as a texture (its raster is ShadowRender::m_shadowRaster)
extern RxPipeline *g_shadowPipeline;  // the sector pipeline that projects it (0 when the device has no multitexturing)
extern RxPipeline *g_shadowCasterPipeline;  // a second pipeline the shipped game never creates (its creator is the folded `return 0`)
extern void *g_shadowBuffer;  // the fallback buffer (0xD2F0 bytes) of a device without the pipeline; nothing reads it
extern RwTexture *g_bigCircleTexture;  // 0x6223d0 "BIGCIRCLE" (ShadowRender and RadarRender both load it)
extern RwTexture *g_headShadowTexture; // 0x622408 "HEADFRAME_SHADOW", the blob far-away actors get
extern RwV3d g_shadowDir;  // no code reads it
extern bool g_detailedShadows;         // 0x606cc4 config "Settings\Display\DetailedShadows"
extern unsigned char g_cranePromptShown;   // 0x617e7c set while the "Prompt_Crane_Button" sprite shows (modules/fx/particles.h)
extern unsigned char g_shadowSkip;     // 0x62a1f0 (set for the escape level by LevelManager, tfb/level/levelmanager.cpp)
extern RWS::CEventId g_msgCorpusShadow;  // CORPUS_SHADOW_PROCESS

// The visitor of one CORPUS_SHADOW_PROCESS message: a stack object of ShadowRender::RenderShadows. Corpus::HandleShadowProbe calls Visit for every grounded body.
class ShadowPass {
public:
	virtual void Visit(RWS::TFB::Collider *collider);

	char m_pad04[0x1c];          // unused
	unsigned char m_detailed;    // 1: the detailed shadows (first message), 0: the BIGCIRCLE rings (second message)
	char m_pad21[3];
	RpWorld *m_world;            // the world the camera renders (stored, nothing reads it)
	RwCamera *m_camera;          // the main camera the shadows are made for
	RwCamera *m_shadowCamera;    // the parallel camera that renders the shadow map (a clone of the main camera)
	RwV3d m_lightDir;            // horizontal view direction of the main camera, normalised (the shadow's length on the ground)
};
ZOOSTERS_ASSERT_SIZE(ShadowPass, 0x3c);
ZOOSTERS_ASSERT_OFFSET(ShadowPass, m_detailed, 0x20);
ZOOSTERS_ASSERT_OFFSET(ShadowPass, m_camera, 0x28);
ZOOSTERS_ASSERT_OFFSET(ShadowPass, m_shadowCamera, 0x2c);
ZOOSTERS_ASSERT_OFFSET(ShadowPass, m_lightDir, 0x30);

bool ShadowSystemOpen();  // the sector pipeline (or the fallback buffer) and the shadow texture
void ShadowSystemClose();
RxPipeline *CreateShadowPipeline();  // a D3D8 all-in-one sector pipeline drawing with ShadowPipelineRender; 0 without multitexturing
RwCamera *ShadowCameraCreate(int size);  // a parallel camera with a z raster of size x size and its own frame
void ShadowCameraDestroy(RwCamera *camera);

// One Im2D quad of the blur pass: corners (left, top) - (right, bottom), depth z, reciprocal depth recipZ, texture origin uv.
int ShadowDrawQuad(float left, float top, float right, float bottom, float z, float recipZ, float uv);
// The blur pass: `count` times copies `source` to `target` and back through the camera `camera` (frame buffer swapped each time).
int ShadowBlurRaster(RwRaster *source, RwRaster *target, RwCamera *camera, unsigned count);
void ShadowRenderActorTree(RWS::TFB::GameActor *actor);  // the actor's clump and those of the actors attached to it
void ShadowActorTreeSphere(RWS::TFB::GameActor *actor, RwSphere *sphere);  // grow `sphere` over the same atomics
void ShadowClumpWalk(RpClump *clump);  // walks the atomics of the clump while g_shadowCasterPipeline is set; does nothing
// Draws the blob `texture` flat on the ground at `position`, size x size, with alpha `strength` (0..1) in the current camera.
void ShadowDrawHead(const RwV3d *position, float size, RwTexture *texture, float strength);
// Projects `texture` (the shadow map, or BIGCIRCLE) seen by `shadowCamera` onto the world sectors that `camera` sees, with `strength`
// (FLT_MAX: the BIGCIRCLE ring) as the shadow's alpha.
void ShadowProject(RwTexture *texture, RwCamera *camera, RwCamera *shadowCamera, float strength);
// Sets the texture matrix and the values ShadowPipelineRender reads: `texture` is projected as `shadowCamera` sees it.
void SetShadowProjection(RwTexture *texture, RwCamera *shadowCamera, float strength, float depthOffset);
void ShadowPipelineRender(RwResEntry *entry, void *sector, RwUInt8 type, RwUInt32 flags);
void SetShadowDirection(const RwV3d *point);

#endif
