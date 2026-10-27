#include <math.h>
#include "tfb/render/shadow.h"
#include "modules/tfb/camera_views.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "tfb/render/tfbclump.h"
#include "tfb/math/boundingsphere.h"

using namespace RWS::TFB;

// GLOBAL: 0x5ca5a8 g_floatZero
// GLOBAL: 0x622434 g_shadowDir

extern const float g_floatZero;  // 0.0f

// Im2D quad of the blur pass as a triangle strip: opaque white, u and v run from `uv` to `uv + 1`.
// 0x442b30
int ShadowDrawQuad(float left, float top, float right, float bottom, float z, float recipZ, float uv)
{
	RwIm2DVertex v[4];
	v[0].x = left;
	v[0].y = top;
	v[0].z = z;
	v[0].rhw = recipZ;
	v[0].emissiveColor = 0xFFFFFFFF;
	v[0].u = uv;
	v[0].v = uv;
	v[1].x = left;
	v[1].y = bottom;
	v[1].z = z;
	v[1].rhw = recipZ;
	v[1].emissiveColor = 0xFFFFFFFF;
	v[1].u = uv;
	v[1].v = uv + 1.0f;
	v[2].x = right;
	v[2].y = top;
	v[2].z = z;
	v[2].rhw = recipZ;
	v[2].emissiveColor = 0xFFFFFFFF;
	v[2].u = uv + 1.0f;
	v[2].v = uv;
	v[3].x = right;
	v[3].y = bottom;
	v[3].z = z;
	v[3].rhw = recipZ;
	v[3].emissiveColor = 0xFFFFFFFF;
	v[3].u = uv + 1.0f;
	v[3].v = uv + 1.0f;
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	return 1;
}

// Blur pass: each round draws `source` over `target` through the camera, then `target` back over `source` with the texture origin shifted by (0.0 * one pixel) so the two copies blend.
// 0x442d00
int ShadowBlurRaster(RwRaster *source, RwRaster *target, RwCamera *camera, unsigned count)
{
	float width = (float)RasterView(source)->width;
	float recipFar, recipWidth;
	recipWidth = 1.0f / width;
	recipFar = 1.0f / CameraView(camera)->farPlane;
	if (count > 0) {
		do {
			CameraView(camera)->frameBuffer = target;
			if (RwCameraBeginUpdate(camera)) {
				RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
				RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
				RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
				RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
				RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source);
				RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONALWAYS);
				ShadowDrawQuad(0.0f, 0.0f, width, width, RwIm2DGetFarScreenZ(), recipFar, recipWidth);
				RwCameraEndUpdate(camera);
			}
			CameraView(camera)->frameBuffer = source;
			if (RwCameraBeginUpdate(camera)) {
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, target);
				ShadowDrawQuad(0.0f, 0.0f, width, width, RwIm2DGetFarScreenZ(), recipFar, recipWidth * g_floatZero);
				RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)1);
				RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)1);
				RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
				RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
				RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONGREATER);
				RwCameraEndUpdate(camera);
			}
		} while (--count);
	}
	CameraView(camera)->frameBuffer = 0;
	return 1;
}

// The first attachment point of an actor; each node holds an AttachPoint whose riders are the actors attached to it.
static Script::SetNode *FirstAttachPoint(GameActor *actor)
{
	return reinterpret_cast<Script::ReferenceSet *>(actor->m_attachmentPoints)->m_head;
}

// Renders the clump of `actor` (unless it casts no shadow) and, recursively, those of the actors riding its attachment points.
// 0x443600
void ShadowRenderActorTree(GameActor *actor)
{
	bool noShadow = (actor->m_params.m_flags & ACTOR_NO_SHADOW) != 0;
	if (!noShadow)
		TFBClump_Render(actor->m_clump);
	for (Script::SetNode *point = FirstAttachPoint(actor); point; point = point->m_next) {
		for (Script::SetNode *rider = ((AttachPoint *)point->m_object)->m_riders.m_head; rider; rider = rider->m_next) {
			if (rider->m_object->GetType() == TFBType_Actor())
				ShadowRenderActorTree(static_cast<GameActor *>(rider->m_object));
		}
	}
}

// Grows `sphere` over the atomics of the clump of `actor` (unless it casts no shadow) and of the actors riding its attachment points.
// 0x443760
void ShadowActorTreeSphere(GameActor *actor, RwSphere *sphere)
{
	bool noShadow = (actor->m_params.m_flags & ACTOR_NO_SHADOW) != 0;
	if (!noShadow) {
		RpClump *clump = actor->m_clump;
		for (RwLLLink *link = clump->atomicList.link.next; link != &clump->atomicList.link;) {
			RwLLLink *next = link->next;
			if (!AtomicGrowSphereCB(RpAtomicFromClumpLink(link), sphere))
				break;
			link = next;
		}
	}
	for (Script::SetNode *point = FirstAttachPoint(actor); point; point = point->m_next) {
		for (Script::SetNode *rider = ((AttachPoint *)point->m_object)->m_riders.m_head; rider; rider = rider->m_next) {
			if (rider->m_object->GetType() == TFBType_Actor())
				ShadowActorTreeSphere(static_cast<GameActor *>(rider->m_object), sphere);
		}
	}
}

// A walk over the atomics of a clump that does nothing: the pipeline is never created in the shipped game, so the loop never runs. Kept as the game has it.
// 0x443490
void ShadowClumpWalk(RpClump *clump)
{
	if (g_shadowCasterPipeline) {
		RwLLLink *end = &clump->atomicList.link;
		for (RwLLLink *link = end->next; link != end; link = link->next) {
			if (RpAtomicFromClumpLink(link) == 0)
				break;
		}
	}
}

// Stores the point the shadow of the next actor is cast at (the centre of its bounding sphere); nothing reads it back. The game's variable is file-static to the shadow unit; shadow.h declares the DLL's view of that address.
// 0x444580
void SetShadowDirection(const RwV3d *point)
{
	if (point)
		g_shadowDir = *point;
}
