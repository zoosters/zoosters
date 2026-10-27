// One record of the cut-scene stack (CutScene_StackSlot 0x436c00 returns its first word): the layout of Script::CutSceneRec
// (script/actorprotos.cpp) as the cut-scene code reads it. 0x24 bytes on x86.
#ifndef ZOOSTERS_SCRIPT_CUTSCENERECORD_H
#define ZOOSTERS_SCRIPT_CUTSCENERECORD_H

#include "zoosters/base.h"
#include "script/referenceset.h"

namespace Script {

struct CutSceneRecord {
	int m_state;                // 2 = no cut scene
	int m_counter;
	int m_stamp;                // script time at CutScene_Begin
	ReferenceSet m_actors;
};
ZOOSTERS_ASSERT_SIZE(CutSceneRecord, 0x24);

}  // namespace Script

#endif
