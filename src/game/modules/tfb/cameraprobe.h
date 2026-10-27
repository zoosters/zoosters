// Also the payload of CORPUS_CAMERA_PROCESS: Corpus::HandleCameraProbe reads m_isect, and a Corpus handler can call slot 0 (ForEachAtomic).
#ifndef ZOOSTERS_MODULES_TFB_CAMERAPROBE_H
#define ZOOSTERS_MODULES_TFB_CAMERAPROBE_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "rw/rpcollis.h"

namespace RWS { namespace TFB { struct Collider; } }

// 0x68 bytes, built on the stack. The game gives it a vtable (0x5cd0c0, in Game.exe's data) so a Corpus handler can call slot 0; m_vptr points at it, the tree emits no vtable.
using RWS::TFB::Collider;

struct CameraProbe {
	RpIntersection m_isect;  // +0x04 the query: box (sup, inf), then sphere (centre, radius), then line (start, end)
	RwV3d m_anchor;          // +0x20 the anchor; during the line casts the line's direction
	float m_best;            // +0x2c nearest hit fraction of the line cast
	unsigned char m_noHit;   // +0x30 cleared by the first line hit
	char m_pad31[3];
	RwV3d m_pts[4];          // +0x34 the end points of the four sight lines (right, left, below, above)
	RpClump *m_skipClump;    // +0x64 the actor's clump: its atomics are not probed

	virtual void ForEachAtomic(const Collider *collider);   // slot 0; the compiler's vptr is at +0 as the game's
};
ZOOSTERS_ASSERT_OFFSET(CameraProbe, m_isect, 0x04);
ZOOSTERS_ASSERT_OFFSET(CameraProbe, m_anchor, 0x20);
ZOOSTERS_ASSERT_OFFSET(CameraProbe, m_pts, 0x34);
ZOOSTERS_ASSERT_OFFSET(CameraProbe, m_skipClump, 0x64);

#endif
