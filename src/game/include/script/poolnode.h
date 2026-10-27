// Script::PoolNodeW: a node of the script message pool g_nodePool (0xfa nodes, 0x620170). The pool serves ActorMessage objects
// (FreeListAlloc from ActorMessage::operator new), so a node must hold one: 0x1c bytes in the game, 0x28 on x64 (the free-list
// link is a real pointer).
#ifndef ZOOSTERS_SCRIPT_POOLNODE_H
#define ZOOSTERS_SCRIPT_POOLNODE_H
#include "zoosters/base.h"

namespace Script {

struct PoolNodeW {
	int m_00;
	PoolNodeW *m_next;
	char m_08[0x14];
};
ZOOSTERS_ASSERT_SIZE(PoolNodeW, 0x1c);
static const unsigned kPoolNodes = 0xfa;

}  // namespace Script

#endif
