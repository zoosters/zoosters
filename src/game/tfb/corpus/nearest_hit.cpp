// RpCollision callbacks that keep the nearest triangle hit.
#define ZOOSTERS_RWS_SENDMSG 1
#include "tfb/corpus/corpus.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus_private.h"
#include "tfb/collision/raytriangle.h"

// Keeps the nearest triangle of an atomic's geometry (distance, index, normal).
// 0x4228e0
RpCollisionTriangle *Collision_NearestGeometryTriangleCB(RpIntersection *intersection, RpCollisionTriangle *tri, RwReal dist,
                                                         void *data)
{
	(void)intersection;
	NearestTriHit *res = (NearestTriHit *)data;
	if (dist < res->dist) {
		res->dist = dist;
		res->index = tri->index;
		res->normal.x = tri->normal.x;
		res->normal.y = tri->normal.y;
		res->normal.z = tri->normal.z;
	}
	return tri;
}

// Line query: keeps the nearest world triangle with its sector's vertices and prelight colours, and the barycentric coordinates of the hit.
// DIFF: one independent load/push order
// 0x422920
RpCollisionTriangle *Collision_NearestWorldTriangleCB(RpIntersection *intersection, RpWorldSector *sector,
                                                      RpCollisionTriangle *tri, RwReal dist, void *data)
{
	NearestTriHit *res = (NearestTriHit *)data;
	const RpWorldSectorView *view = (const RpWorldSectorView *)sector;
	if (dist < res->dist) {
		res->dist = dist;
		res->vertices = view->vertices;
		res->preLit = view->preLitLum;
		int idx = tri->index;
		res->index = idx;
		res->triangle = view->triangles + idx * 8;
		const RwLine *line = &intersection->t.line;
		RwV3d dir;
		RwV3dSub(&dir, &line->end, &line->start);
		RayTriangleIntersect(&line->start, &dir, tri->vertices[0], tri->vertices[1], tri->vertices[2], 0, res->uv, 0);
		res->normal.x = tri->normal.x;
		res->normal.y = tri->normal.y;
		res->normal.z = tri->normal.z;
	}
	return tri;
}
