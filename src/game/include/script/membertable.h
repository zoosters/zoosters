// One entry of a GetMembers table: a bare ScriptReference-sized object that carries only its prototype class's vptr,
// destroyed at exit by ScriptReference's destructor 0x43b3d0.
#ifndef ZOOSTERS_SCRIPT_MEMBERTABLE_H
#define ZOOSTERS_SCRIPT_MEMBERTABLE_H

#include "script/script.h"

namespace Script {

struct MemberSlot {
	void *m_vptr;
	~MemberSlot();   // 0x43b3d0 (ScriptReference::~ScriptReference)
};
// GLOBAL: 0x43b3d0 ??1MemberSlot@Script@@QAE@XZ

}  // namespace Script

#endif
