// Declarations shared by the Corpus implementation files.
#ifndef ZOOSTERS_TFB_CORPUS_PRIVATE_H
#define ZOOSTERS_TFB_CORPUS_PRIVATE_H

#include <math.h>
#include "tfb/level/level.h"
#include <string.h>
#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rpcollis.h"
#include "tfb/corpus/sectorview.h"
#include "tfb/corpus/corpus.h"
#include "tfb/corpus/corpusprobe.h"
#include "tfb/collision/collisionbox.h"
#include "modules/tfb/gameactor.h"
#include "modules/tfb/gamecamera.h"
#include "tfb/math/vector_ops.h"
#include "modules/tfb/renderqueue.h"
#include "tfb/render/atomicflags.h"
#include "framework/mainloop/gameclock.h"

using RWS::TFB::ActorParams;
using RWS::TFB::Collider;
using RWS::TFB::GameActor;

// The tick's time step: g_frameDt seconds; the physics works in units of 12 per second (g_corpusGravityDt = 12 * dt, its square
// times dt for accelerations).
extern float g_frameDt;                              // GLOBAL: 0x618318 g_frameDt
extern float g_corpusGravityDt;                      // GLOBAL: 0x618310 g_corpusGravityDt
extern float g_corpusGravityDt2;                     // GLOBAL: 0x61830c g_corpusGravityDt2
extern const float g_floatZero;                      // GLOBAL: 0x5ca5a8 g_floatZero
extern const RwV3d g_corpusYAxis;                    // the up axis
int RotationFromTo(const RwV3d *from, const RwV3d *to, RwMatrix *out);  // 0 if degenerate
int AimAttachmentsCB(int unused, void *id, void *mat, void *actor);        // 0x42b2d0 particle-emitter aim callback

static const float kRadToDeg = 57.2957763671875f;   // 180 / pi (the game's float)
static const float kDegToRad = 0.017453292f;        // pi / 180
static const float kNoFacing = 3.4028235e+38f;      // GameActor::m_targetFacing when there is no target (FLT_MAX)


// GLOBAL: 0x62972c RWS::CLevel::sm_pActiveWorld

// The nearest triangle of a line query; the geometry callback fills dist, index and normal.
struct NearestTriHit {
	float dist;           // the hit's fraction of the line (FLT_MAX: none yet)
	int index;
	char *triangle;       // world queries: the RpTriangle
	RwV3d *vertices;      // world queries: the sector's vertices
	void *preLit;         // world queries: the sector's prelight colours
	float uv[2];          // barycentric u, v of the hit
	RwV3d normal;
};


RpCollisionTriangle *Collision_NearestGeometryTriangleCB(RpIntersection *intersection, RpCollisionTriangle *tri, RwReal dist,
                                                         void *data);
RpCollisionTriangle *Collision_NearestWorldTriangleCB(RpIntersection *intersection, RpWorldSector *sector,
                                                      RpCollisionTriangle *tri, RwReal dist, void *data);

// The probe's sweep against one triangle (triangle_sweep.cpp); also the broad phase's geometry callback.
RpCollisionTriangle *Corpus_SphereTriangleSweepD(RpIntersection *intersection, RpCollisionTriangle *tri, RwReal dist, void *data);

Script::ReferenceSet *CloneList_Get();
int *CutScene_StackSlot(int delta);                  // 0x436c00
namespace RWS { namespace MainLoop { namespace Render { unsigned GetFrameCount(); } } }  // 0x462710

#endif
