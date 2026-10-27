// An actor's attachment point: other bodies ride it (Corpus::RideAttachPoint, GameParticle::SetAttachTarget).
#ifndef ZOOSTERS_MODULES_FX_ATTACHPOINT_H
#define ZOOSTERS_MODULES_FX_ATTACHPOINT_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "script/referenceset.h"

struct AttachPoint {
	void *m_vptr;
	char m_04[4];
	RwV3d m_worldPos;
	Script::ReferenceSet m_riders;   // +0x14 the bodies attached to this point
	intptr_t m_2c;
	RwMatrix m_local;                // +0x30 local matrix (valid when m_dirty is clear)
	RwMatrix m_localInv;             // +0x70 its inverse
	float m_rotY;                    // +0xb0 yaw (about Y)
	float m_rotX;                    // +0xb4 pitch (about X)
	float m_rotZ;                    // +0xb8 roll (about Z)
	RwV3d m_position;                // +0xbc
	RwFrame *m_frame;                // +0xc8 the point's frame (SetAttachTarget stores it in the rider's link)
	unsigned char m_dirty;           // +0xcc the local matrix needs rebuilding

	void RebuildLocalMatrix();
	RwMatrix *GetLocalMatrix();
};
// Same offsets as Script::ActorAttachment (types/actorattachment.h).
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_vptr, 0x00);
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_04, 0x04);
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_worldPos, 0x08);
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_2c, 0x2c);
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_riders, 0x14);
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_local, 0x30);
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_localInv, 0x70);
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_rotY, 0xb0);
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_position, 0xbc);
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_frame, 0xc8);
ZOOSTERS_ASSERT_OFFSET(AttachPoint, m_dirty, 0xcc);

#endif
