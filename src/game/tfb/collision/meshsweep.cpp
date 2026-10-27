// The probe of a body swept against the meshes of another actor (actors with ACTOR_MESH_COLLIDEE: the broad phase hands such pairs
// here): every atomic of the actor's clump, the triangles of its geometry near the swept box (through the geometry's collision tree,
// the box query of RpCollision that the game inlines), each tested by the corpus's triangle sweep.
#include "tfb/collision/meshsweep.h"
#include "tfb/collision/collider.h"
#include "tfb/corpus/corpus.h"
#include "tfb/corpus/corpusprobe.h"
#include "modules/tfb/gameactor.h"
#include "rw/rpcollis.h"
#include "rw/rttoolkits.h"
#include <math.h>

// GLOBAL: 0x62a7b0 _rpCollisionGeometryDataOffset

using RWS::TFB::Collider;
using RWS::TFB::GameActor;

// The corpus's sweep of the probe against one triangle (tfb/corpus/triangle_sweep.cpp)
RpCollisionTriangle *Corpus_SphereTriangleSweepD(RpIntersection *intersection, RpCollisionTriangle *tri, RwReal dist, void *data);   // 0x4229d0

// Component `axis` (0 x, 1 y, 2 z) of a vector
static inline RwReal &Axis(RwV3d &v, int axis)
{
	return (&v.x)[axis];
}
static inline RwReal Axis(const RwV3d &v, int axis)
{
	return (&v.x)[axis];
}

// The triangles of a geometry built without a collision tree: each one that touches the box of `intersection` goes to the callback
// until it returns 0. Only the triangle's index and vertices are filled in.
// FUNCTION: 0x447940 GeometryForAllTrianglesInBox
void GeometryForAllTrianglesInBox(RpGeometry *geometry, RpIntersection *intersection,
                                  RpIntersectionCallBackGeometryTriangle callBack, void *data)
{
	RwV3d *vertices = RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geometry, 0));
	const RpTriangle *triangles = RpGeometryGetTriangles(geometry);
	for (int i = 0; i < RpGeometryGetNumTriangles(geometry); i++) {
		RwV3d *v0 = &vertices[triangles[i].vertIndex[0]];
		RwV3d *v1 = &vertices[triangles[i].vertIndex[1]];
		RwV3d *v2 = &vertices[triangles[i].vertIndex[2]];
		if (!RtIntersectionBBoxTriangle(&intersection->t.box, v0, v1, v2))
			continue;
		RpCollisionTriangle tri; // NOTE: normal and point stay unset, as in the game (the sweep callback does not read them)
		tri.index = i;
		tri.vertices[0] = v0;
		tri.vertices[1] = v1;
		tri.vertices[2] = v2;
		if (!callBack(intersection, &tri, 0.0f, data))
			return;
	}
}

// The box the probe's box sweeps through: the box's bounds (its extent along each world axis, plus 0.5), stretched to the body's
// position and along the motion of the remaining time (m_move * m_t), then grown by 1 on every side.
static void ProbeSweepBounds(const CorpusProbe &probe, RwBBox *box)
{
	const CollisionBox &obb = probe.m_box;
	for (int i = 0; i < 3; i++) {
		RwReal extent = fabsf(Axis(obb.m_frame.right, i) * obb.m_extent[0]) + fabsf(Axis(obb.m_frame.at, i) * obb.m_extent[2]) +
		                fabsf(Axis(obb.m_frame.up, i) * obb.m_extent[1]) + 0.5f;
		RwReal &lo = Axis(box->inf, i);
		RwReal &hi = Axis(box->sup, i);
		lo = Axis(obb.m_frame.pos, i) - extent;
		hi = extent + Axis(obb.m_frame.pos, i);
		RwReal start = Axis(probe.m_pos, i);
		if (start < lo)
			lo = start;
		else if (hi < start)
			hi = start;
		RwReal move = Axis(probe.m_move, i) * probe.m_t;
		if (move <= 0.0f)
			lo += move;
		else
			hi += move;
	}
	for (int i = 0; i < 3; i++) {
		Axis(box->inf, i) -= 1.0f;
		Axis(box->sup, i) += 1.0f;
	}
}

// Hands the triangles of `count` collision-tree entries from `first` on to the callback (the entry map gives the triangle index).
// Returns false when the callback stopped the query.
static bool CollTreeForEntries(const RpCollTree *tree, RpGeometry *geometry, unsigned first, unsigned count, RpIntersection *intersection,
                               RpIntersectionCallBackGeometryTriangle callBack, void *data)
{
	RwV3d *vertices = RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geometry, 0));
	const RpTriangle *triangles = RpGeometryGetTriangles(geometry);
	int end = (RwUInt16)count + (RwUInt16)first;
	for (RwUInt16 entry = (RwUInt16)first; entry < end; entry++) {
		RwUInt16 index = tree->map ? tree->map[entry] : entry;
		const RpTriangle &t = triangles[index];
		RpCollisionTriangle tri; // NOTE: normal and point stay unset, as in the game
		tri.index = index;
		tri.vertices[0] = &vertices[t.vertIndex[0]];
		tri.vertices[1] = &vertices[t.vertIndex[1]];
		tri.vertices[2] = &vertices[t.vertIndex[2]];
		if (!callBack(intersection, &tri, 0.0f, data))
			return false;
	}
	return true;
}

// The walk of a collision tree for the leaves near a box (RW's RpCollSectorBBoxIt, inlined by the game). Each stack node carries
// clip flags: bit 2k / 2k+1 set while the node may still reach past the box's low / high side on axis k. A node with no clip flags
// lies inside the box, so all its leaves are taken as one run of entries without further tests.
struct CollTreeBoxWalk {
	struct Node {
		RpCollSector *sector;
		RwUInt32 clipFlags;
	};
	RpCollTree *tree;
	RwBBox bbox;     // rejects the nodes outside it
	RwBBox clipBBox; // clears the clip flags of the nodes inside it
	RpCollSector root;
	Node stack[rpCOLLTREE_MAXDEPTH + 1];
	Node *curr;
};

// Starts the walk at the tree's root with the clip flags of the tree's own box against `box`.
static void CollTreeBoxWalkInit(CollTreeBoxWalk *walk, RpCollTree *tree, const RwBBox *box)
{
	walk->tree = tree;
	walk->root.index = 0;
	walk->root.contents = rpCOLLSECTOR_CONTENTS_SPLIT;
	if (tree->numSplits == 0)
		walk->root.contents = (RwUInt8)tree->numEntries;
	walk->bbox = *box;
	walk->clipBBox = *box;
	RwUInt32 clipFlags = 0;
	for (int k = 0; k < 3; k++) {
		if (Axis(tree->bbox.inf, k) < Axis(box->inf, k))
			clipFlags |= 1 << (2 * k);
		if (Axis(tree->bbox.sup, k) > Axis(box->sup, k))
			clipFlags |= 2 << (2 * k);
	}
	walk->stack[0].sector = &walk->root;
	walk->stack[0].clipFlags = clipFlags;
	walk->curr = walk->stack + 1;
}

// The next run of entries to test: pops a node and descends to a leaf (pushing the other child where the box reaches both), or stops at
// a node inside the box and takes its whole subtree.
static void CollTreeBoxWalkNext(CollTreeBoxWalk *walk, unsigned *first, unsigned *count)
{
	RpCollTree *tree = walk->tree;
	walk->curr--;
	RpCollSector *sector = walk->curr->sector;
	while (sector->contents == rpCOLLSECTOR_CONTENTS_SPLIT) {
		if (walk->curr->clipFlags == 0) {
			// inside the box: the run from the subtree's leftmost leaf to the end of its rightmost leaf
			const RpCollSector *left = &tree->splits[sector->index].leftSector;
			while (left->contents == rpCOLLSECTOR_CONTENTS_SPLIT)
				left = &tree->splits[left->index].leftSector;
			const RpCollSector *right = &tree->splits[sector->index].rightSector;
			while (right->contents == rpCOLLSECTOR_CONTENTS_SPLIT)
				right = &tree->splits[right->index].rightSector;
			*first = left->index;
			*count = (RwUInt16)(right->contents + right->index - *first);
			return;
		}
		RpCollSplit *split = &tree->splits[sector->index];
		int axis = (split->leftSector.type & rpCOLLSECTOR_TYPE_AXISMASK) >> 2;
		if (split->rightSector.value > Axis(walk->bbox.sup, axis)) {
			walk->curr->sector = &split->leftSector; // the box ends before the right child
		} else {
			walk->curr->sector = &split->rightSector;
			if (split->leftSector.value >= Axis(walk->bbox.inf, axis)) {
				// the box reaches the left child too: it waits on the stack; the children inside the clip box lose that side's flag
				CollTreeBoxWalk::Node *parent = walk->curr;
				walk->curr++;
				walk->curr->sector = &split->leftSector;
				walk->curr->clipFlags = parent->clipFlags;
				if (split->leftSector.value <= Axis(walk->clipBBox.sup, axis))
					walk->curr->clipFlags &= ~(2u << (2 * axis));
				if (split->rightSector.value >= Axis(walk->clipBBox.inf, axis))
					parent->clipFlags &= ~(1u << (2 * axis));
			}
		}
		sector = walk->curr->sector;
	}
	*first = sector->index;
	*count = sector->contents;
}

// Sweeps the probe against the triangles of `geometry` (in the geometry's frame): every triangle near the box the probe's box
// passes through goes to the callback with the probe as its data, until the callback returns 0.
// FUNCTION: 0x447a00 ProbeSweepGeometry
void ProbeSweepGeometry(RpGeometry *geometry, RpIntersectionCallBackGeometryTriangle callBack, CorpusProbe *probe)
{
	RpIntersection intersection;
	intersection.type = rpINTERSECTBOX;
	RwBBox &box = intersection.t.box;
	ProbeSweepBounds(*probe, &box);
	RpCollTree *tree = RpCollisionGeometryGetCollTree(geometry);
	if (!tree) {
		GeometryForAllTrianglesInBox(geometry, &intersection, callBack, probe);
		return;
	}
	for (int k = 0; k < 3; k++) {
		if (!(Axis(tree->bbox.inf, k) <= Axis(box.sup, k)) || !(Axis(tree->bbox.sup, k) >= Axis(box.inf, k)))
			return;
	}
	CollTreeBoxWalk walk;
	CollTreeBoxWalkInit(&walk, tree, &box);
	do {
		unsigned first, count;
		CollTreeBoxWalkNext(&walk, &first, &count);
		if (!CollTreeForEntries(tree, geometry, first, count, &intersection, callBack, probe))
			return;
	} while (walk.curr > walk.stack);
}

// A tall body that moves (and is not a free mover standing still) tests a box lifted by 9 and 9 shorter, so it walks over
// steps up to that height.
static const float kStepHeight = 9.0f;
static const float kStepMinHalfHeight = 12.0f; // only bodies with at least this half height
static const float kGroundRayOffset = 0.75f;   // the second ground ray starts this far along the ground normal

// Brings the probe into the atomic's frame: the body's world box, displacement (relative to the moving actor whose mesh this is),
// position, up axis and ground direction. A body standing on this very atomic is first moved by how far its contact point lies from
// its frame (it rides the carrier).
static void ProbeToAtomicFrame(CorpusProbe *probe, RpAtomic *atomic, const RwMatrix *inverse)
{
	Corpus *body = probe->m_corpus;
	GameActor *owner = body->m_owner;
	RwMatrixMultiply(&probe->m_box.m_frame, &owner->m_worldBox.m_frame, inverse);
	probe->m_box.m_extent[0] = owner->m_worldBox.m_extent[0];
	probe->m_box.m_extent[1] = owner->m_worldBox.m_extent[1];
	probe->m_box.m_extent[2] = owner->m_worldBox.m_extent[2];
	probe->m_move = owner->m_velocity;
	if (probe->m_hitActor)
		RwV3dSub(&probe->m_move, &probe->m_move, &probe->m_hitActor->m_velocity);
	RwV3dTransformVectors(&probe->m_move, &probe->m_move, 1, inverse);
	RwV3dTransformPoint(&probe->m_pos, &body->m_frame->modelling.pos, inverse);
	probe->m_up = inverse->up;
	if ((body->m_flags & (CORPUS_AIRBORNE | CORPUS_LEFT_GROUND)) == CORPUS_AIRBORNE) {
		probe->m_groundDir = inverse->up;
		return;
	}
	if (body->m_link.m_atomic == atomic) {
		RwV3d offset;
		RwV3dSub(&offset, &body->m_contactPoint, &body->m_lastFrame.pos);
		RwV3dTransformVectors(&offset, &offset, 1, inverse);
		RwV3dAdd(&probe->m_box.m_frame.pos, &probe->m_box.m_frame.pos, &offset);
		RwV3dAdd(&probe->m_pos, &probe->m_pos, &offset);
		RwV3dSub(&probe->m_move, &probe->m_move, &offset);
	}
	if (body->m_flags & CORPUS_MOVING) {
		const RWS::TFB::ActorParams &params = owner->m_params;
		bool freeMoverAtRest = params.m_groundAccel == 0.0f && params.m_moveType == RWS::TFB::ACTOR_MOVE_FREE;
		if (!freeMoverAtRest && probe->m_box.m_extent[1] >= kStepMinHalfHeight) {
			RwV3dIncrementScaled(&probe->m_box.m_frame.pos, &probe->m_box.m_frame.up, kStepHeight);
			probe->m_box.m_extent[1] -= kStepHeight;
		}
	}
	RwV3dTransformVectors(&probe->m_groundDir, &body->m_groundNormal, 1, inverse);
	RwV3dScale(&probe->m_pos2, &probe->m_groundDir, kGroundRayOffset);
	RwV3dAdd(&probe->m_pos2, &probe->m_pos2, &probe->m_pos);
}

// The prelight colour of the ground triangle the body stands on (the hit triangle of the ground rays, at their barycentric
// coordinates), stored as the body's ground colour.
static void SampleGroundColor(Corpus *body, RpGeometry *geometry)
{
	const RwRGBA *colors = RpGeometryGetPreLightColors(geometry);
	const RpTriangle &t = RpGeometryGetTriangles(geometry)[body->m_link.m_index];
	RwV3d c[3];
	for (int i = 0; i < 3; i++) {
		const RwRGBA &color = colors[t.vertIndex[i]];
		c[i].x = (float)color.red;
		c[i].y = (float)color.green;
		c[i].z = (float)color.blue;
	}
	RwV3d d1, d2;
	RwV3dSub(&d1, &c[1], &c[0]);
	RwV3dSub(&d2, &c[2], &c[0]);
	float u = body->m_link.m_u;
	float v = body->m_link.m_v;
	body->m_groundColor[0] = (unsigned char)(int)(d1.x * u + d2.x * v + c[0].x);
	body->m_groundColor[1] = (unsigned char)(int)(d1.y * u + d2.y * v + c[0].y);
	body->m_groundColor[2] = (unsigned char)(int)(d1.z * u + d2.z * v + c[0].z);
}

// Sweeps the probe against one atomic of the actor in probe->m_hitActor (an RpClumpForAllAtomics callback: returns the atomic to go
// on). The test runs in the atomic's frame; a hit is brought back to world space. A hit of the sweep (the free fraction m_t fell below
// the remaining time) becomes the probe's earliest hit; otherwise the ground rays found this mesh under the body, which then records it
// as its carrier candidate (atomic, actor, contact point) and takes its ground colour from it.
// FUNCTION: 0x447f50 ProbeSweepAtomic
RpAtomic *ProbeSweepAtomic(RpAtomic *atomic, CorpusProbe *probe)
{
	RpGeometry *geometry = RpAtomicGetGeometry(atomic);
	if (!geometry || !RpGeometryGetTriangles(geometry))
		return atomic;
	RwMatrix *ltm = RwFrameGetLTM(RpAtomicGetFrame(atomic));
	RwMatrix inverse;
	RwMatrixInvert(&inverse, ltm);
	ProbeToAtomicFrame(probe, atomic, &inverse);

	Corpus *savedHitCorpus = probe->m_hitCorpus;
	probe->m_hitCorpus = 0;
	RwV3d savedNormal;
	if (probe->m_t == 0.0f) {
		savedNormal = probe->m_normal;
		RwV3dTransformVectors(&probe->m_normal, &probe->m_normal, 1, &inverse);
	}
	ProbeSweepGeometry(geometry, Corpus_SphereTriangleSweepD, probe);
	if (!probe->m_hitCorpus) {
		probe->m_hitCorpus = savedHitCorpus;
		if (probe->m_t == 0.0f)
			probe->m_normal = savedNormal;
		return atomic;
	}

	Collider *actor = probe->m_hitActor;
	if (probe->m_t < probe->m_rem) {
		probe->m_hitAtomic = atomic;
		RwV3dTransformPoint(&probe->m_hitPos, &probe->m_hitPos, ltm);
		probe->m_hit = actor;
		if (actor)
			RwV3dIncrementScaled(&probe->m_hitPos, &actor->m_velocity, probe->m_t);
		RwV3dTransformVectors(&probe->m_normal, &probe->m_normal, 1, ltm);
		return atomic;
	}
	Corpus *body = probe->m_corpus;
	body->m_hitAtomic = atomic;
	RwV3dTransformVectors(&body->m_frameStep, &body->m_frameStep, 1, ltm);
	RwV3dTransformPoint(&body->m_carrierFrame.pos, &body->m_carrierFrame.pos, ltm);
	body->m_hitCarrier = actor;
	if (body->m_hitCarrier) {
		RwV3dIncrementScaled(&body->m_carrierFrame.pos, &actor->m_velocity, probe->m_t);
		return atomic;
	}
	if (RpGeometryGetPreLightColors(RpAtomicGetGeometry(atomic)))
		SampleGroundColor(body, RpAtomicGetGeometry(atomic));
	return atomic;
}

// Sweeps the probe against every atomic of the actor's clump (`collider`: the actor whose mesh the body may hit), stopping when an
// atomic's sweep says so. A probe that already touches something (m_t 0) only goes on when that something is its own body.
// FUNCTION: 0x448790 ProbeSweepColliderMesh
void ProbeSweepColliderMesh(Collider *collider, CorpusProbe *probe)
{
	if (probe->m_t == 0.0f && probe->m_corpus != probe->m_hitCorpus)
		return;
	if (!collider->m_clump)
		return;
	probe->m_hitActor = collider;
	probe->m_useTriFlags = 0;
	RwLLLink *end = &collider->m_clump->atomicList.link;
	for (RwLLLink *link = end->next; link != end;) {
		RwLLLink *next = link->next;
		if (!ProbeSweepAtomic(RpAtomicFromClumpLink(link), probe))
			break;
		link = next;
	}
}
