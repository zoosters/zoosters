// Render queue entry (kind >= 2) that draws PTank atomics with one blend pair. Only the members Render reads are named.
#ifndef ZOOSTERS_MODULES_FX_PARTICLEPASS_H
#define ZOOSTERS_MODULES_FX_PARTICLEPASS_H

#include "zoosters/base.h"

struct ParticlePassNode;

// Follows ParticlePool<T> (particles.h); offsets are the game's with 4-byte pointers (asserted below).
struct ParticlePass {
	void *m_freeRoot;               // +0x00 (x86)
	ParticlePassNode *m_atomics;    // +0x04 list of particle atomics to draw (m_busy)
	void *m_base;                   // +0x08
	int m_capacity;                 // +0x0c
	int m_10;                       // +0x10 pass mode: 0 and 2 draw without z-write
	int m_srcBlend, m_dstBlend;     // +0x14/+0x18 RwBlendFunction pair (copied into each PTank's srcBlend/dstBlend; 5/6 = default)
	unsigned char m_1c;             // +0x1c stencil write on
	void Render();   // draws the atomics (a second additive pass for mode 2)
};
#ifndef ZOOSTERS_PTR64
ZOOSTERS_ASSERT_OFFSET(ParticlePass, m_atomics, 0x04);
ZOOSTERS_ASSERT_OFFSET(ParticlePass, m_10, 0x10);
ZOOSTERS_ASSERT_OFFSET(ParticlePass, m_1c, 0x1c);
#endif

#endif
