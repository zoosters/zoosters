#include "script/runtime.h"
#include "script/script.h"
#include "script/poolnode.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
using namespace Script;

// std::vector<4-byte T>::_Ufill (this unused, ret 0xc): fill_n inlined, returns dst + n
// 0x439700
unsigned *__stdcall U32VectorUfill(unsigned *dst, unsigned n, const unsigned *val)
{
	unsigned *p = dst;
	for (unsigned i = n; i > 0; i--)
		*p++ = *val;
	return dst + n;
}

void operator_delete(void *p);          // 0x5500f0
// GLOBAL: 0x620170 Script::g_nodePool
namespace Script { extern PoolNodeW g_nodePool[]; }   // the script node pool 0x620170-0x621cc8 (0xfa nodes of 0x1c bytes)
// the pool-aware operator delete of the script nodes (called from ActorMessage's scalar deleting destructor 0x42f5a0)
// 0x439d10
void ScriptNodePool_Free(void *node)
{
	if ((char *)node >= ((char *)Script::g_nodePool) && (char *)node < ((char *)Script::g_nodePool) + kPoolNodes * sizeof(PoolNodeW)) {   // x64: the nodes are bigger than 0x1c
		((FreeNode *)node)->m_next = g_nodeFreeList;
		g_nodeFreeList = (FreeNode *)node;
		return;
	}
	operator_delete(node);
}

// GLOBAL: 0x603078 g_crcTable
extern unsigned g_crcTable[256];
// 0x43bb20
unsigned NameHashNoCase(const char *s)
{
	if (s == 0)
		return 0;
	unsigned crc = (unsigned)-1;
	char c = *s++;
	while (c != 0) {
		if (c >= 'A' && c <= 'Z')
			c += 0x20;
		crc = g_crcTable[(crc ^ c) & 0xff] ^ (crc >> 8);
		c = *s++;
	}
	return crc;
}

// 0x43dc80
void *RWS::CAttributeHandler::GetAttrPostUpdateFunc(void *&pThis)
{
	pThis = 0;
	return 0;
}
