// PartEntry: one 0x40-byte entry of a CTFBModel's part table (CTFBModel::m_entries, ctfbmodel.h): the descriptor GameActor::CreateParts
// (0x42e240) builds an actor part (attachment point, turret, corpus foot point) from, and that the part keeps (ActorTurret::m_entry,
// +0x108). One definition for every user (CreateParts, the part constructor 0x42e0c0, the aim update 0x42b2d0). Float fields are
// declared as int bits where the cl 13.10 view of the users copies them as integers.
#ifndef ZOOSTERS_SCRIPT_TYPES_PARTENTRY_H
#define ZOOSTERS_SCRIPT_TYPES_PARTENTRY_H

#include "zoosters/base.h"
#include "script/script.h"

struct PartEntry {
	int m_00;
	unsigned m_kind;  // 0 attachment point, 1 turret, 2 corpus part
	int m_08;
	unsigned m_sortKey;
	unsigned m_boneIndex;  // node index in the owner's HAnim hierarchy (the aim update compares it with a callback id)
	int m_14;  // a percentage (the part stores 1 - m_14 / 100 at +0x104)
	char m_pad18[4];
	int m_roll;
	Vec3 m_localOffset;
	int m_facing;
	int m_tilt;
	int m_fov;
	float m_margin;  // aim: added to both limits; beyond limit + margin the aim resets to 0
	float m_pitchLimit;
};
ZOOSTERS_ASSERT_SIZE(PartEntry, 0x40);
ZOOSTERS_ASSERT_OFFSET(PartEntry, m_boneIndex, 0x10);
ZOOSTERS_ASSERT_OFFSET(PartEntry, m_localOffset, 0x20);
ZOOSTERS_ASSERT_OFFSET(PartEntry, m_fov, 0x34);

#endif
