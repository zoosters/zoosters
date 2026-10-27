// The "start node" word of an OpBehavior (the behaviour block ops and the user "behavior" variable's referent): at +0xc with 4-byte
// pointers (OpBehavior::m_startPC, script/ops_behavior.h). The cl 13.10 build reads the word at that offset; normal builds read the member.
#ifndef ZOOSTERS_SCRIPT_BEHAVIORLAYOUT_H
#define ZOOSTERS_SCRIPT_BEHAVIORLAYOUT_H
#include "script/ops_behavior.h"
#define BEHAVIOR_START(p) (((Script::OpBehavior *)(p))->m_startPC)
#endif
