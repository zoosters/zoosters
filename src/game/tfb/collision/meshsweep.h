// The probe of a body swept against another actor's meshes (meshsweep.cpp).
#ifndef ZOOSTERS_TFB_COLLISION_MESHSWEEP_H
#define ZOOSTERS_TFB_COLLISION_MESHSWEEP_H

#include "rw/rpcollis.h"

struct CorpusProbe;
namespace RWS {
namespace TFB {
struct Collider;
}
}

// Every triangle of a geometry without a collision tree that touches the box of `intersection` goes to the callback.
void GeometryForAllTrianglesInBox(RpGeometry *geometry, RpIntersection *intersection,
                                  RpIntersectionCallBackGeometryTriangle callBack, void *data);   // 0x447940
// Every triangle of `geometry` near the box the probe sweeps through goes to the callback (data: the probe).
void ProbeSweepGeometry(RpGeometry *geometry, RpIntersectionCallBackGeometryTriangle callBack, CorpusProbe *probe);   // 0x447a00
// Sweeps the probe against one atomic of probe->m_hitActor; returns the atomic (RpClumpForAllAtomics callback).
RpAtomic *ProbeSweepAtomic(RpAtomic *atomic, CorpusProbe *probe);   // 0x447f50
// Sweeps the probe against every atomic of the collider's clump.
void ProbeSweepColliderMesh(RWS::TFB::Collider *collider, CorpusProbe *probe);   // 0x448790

#endif
