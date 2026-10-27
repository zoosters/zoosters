// Probe record of the physics step: one body's swept box and the earliest hit found for it.
#ifndef ZOOSTERS_TFB_CORPUS_CORPUSPROBE_H
#define ZOOSTERS_TFB_CORPUS_CORPUSPROBE_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "tfb/collision/collisionbox.h"

struct RpAtomic;
class Corpus;
namespace RWS {
namespace TFB {
struct Collider;
}
}

// Filled by every body's HandleEvents (ResolveGroundContact) in the probing pass; the triangle callbacks and the broad phase keep the earliest hit.
// While another actor's mesh is tested, positions and directions are in that atomic's frame.
struct CorpusProbe {
	float m_t;                        // fraction of the remaining time free of hits (0: touching already)
	float m_rem;                      // the remaining fraction of the tick (1 at the start)
	RwV3d m_hitPos;                   // the earliest hit's point
	RwV3d m_normal;                   // its normal
	RwV3d m_up;                       // the probing body's up axis (0, 1, 0)
	RwV3d m_groundDir;                // up when airborne, else the ground normal: the ground rays point against it
	RwV3d m_move;                     // displacement over the tick (Collider::m_velocity); the physics step reuses it for the relative velocity of the hit body
	RwV3d m_pos;                      // start position (the frame's)
	RwV3d m_pos2;                     // start of the second ground ray (m_pos + 0.75 ground normal)
	RwV3d m_ledge;                    // normal of the ledge triangle touched (with CORPUS_HIT_LEDGE)
	float m_u, m_v;                   // barycentric coordinates of a ray hit
	Corpus *m_corpus;                 // the body probing now
	Corpus *m_hitCorpus;              // the body of the earliest hit
	RWS::TFB::Collider *m_hitActor;   // the actor whose mesh is being tested (0: the world)
	RWS::TFB::Collider *m_hit;        // the actor the earliest hit is on (0: world geometry)
	RpAtomic *m_hitAtomic;            // the atomic of the earliest hit
	CollisionBox m_box;               // the probing body's world box
	unsigned char m_firstPass;        // 1 in the probing pass
	unsigned char m_rayHit;           // the earliest hit came from a ray test (1) or the box sweep (0)
	unsigned char m_useTriFlags;      // the world has the triangle flag table
	char m_padD3;
	unsigned char *m_triFlags;        // per-triangle collision flags (8 B entries, byte +6): 1 trigger, 4 ignored, 0x20 ledge
	char m_padD8[4];
};
ZOOSTERS_ASSERT_SIZE(CorpusProbe, 0xdc);
ZOOSTERS_ASSERT_OFFSET(CorpusProbe, m_corpus, 0x70);
ZOOSTERS_ASSERT_OFFSET(CorpusProbe, m_box, 0x84);
ZOOSTERS_ASSERT_OFFSET(CorpusProbe, m_firstPass, 0xd0);
ZOOSTERS_ASSERT_OFFSET(CorpusProbe, m_triFlags, 0xd4);

#endif
