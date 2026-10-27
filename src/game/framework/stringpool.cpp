// String interning pool: a chain of blocks of NUL-terminated strings.
#include "zoosters/base.h"
#include <string.h>
#include "framework/stringpool.h"
#include "framework/core/cattributehandler.h"

void *MemAlloc(size_t n);
void operator_delete(void *p);

struct StrBlock {
	char m_data[0x200];
	unsigned m_used;        // +0x200
	StrBlock *m_next;       // +0x204
	StrBlock(StrBlock *next) : m_used(0), m_next(next) {}
	char *Find(const char *s);
	ZOOSTERS_NOINLINE StrBlock *Destroy(unsigned flags);
};

struct StrPool {
	StrBlock *m_head;
	StrPool() : m_head(0) {}
	void AddBlock();
	char *Add(const char *s);
};

// 0x464b30
char *StrBlock::Find(const char *s)
{
	char *p = (char *)this;
	char *end = p + m_used;
	while (p < end) {
		if (strcmp(s, p) == 0)
			return p;
		p += strlen(p) + 1;
	}
	return 0;
}

// 0x464bb0
void StrPool::AddBlock()
{
	m_head = new StrBlock(m_head);
}

// 0x464be0
char *StrPool::Add(const char *s)
{
	StrBlock *block;
	for (block = m_head; block; block = block->m_next) {
		char *found = block->Find(s);
		if (found)
			return found;
	}
	unsigned need = strlen(s) + 1;
	for (block = m_head; block; block = block->m_next) {
		if (0x200 - block->m_used >= need) {
			char *dst = (char *)block + block->m_used;
			strcpy(dst, s);
			block->m_used += strlen(s) + 1;
			return dst;
		}
	}
	AddBlock();
	block = m_head;
	char *dst = (char *)block + block->m_used;
	strcpy(dst, s);
	block->m_used += strlen(s) + 1;
	return dst;
}

// 0x464cd0
StrBlock *StrBlock::Destroy(unsigned flags)
{
	if (m_next)
		m_next->Destroy(1);
	if (flags & 1)
		operator_delete(this);
	return this;
}

extern StrPool *g_strPool;   // GLOBAL: 0x62a604 g_strPool

// 0x464d00
const char *InternString(const char *s)
{
	if (s && *s) {
		if (!g_strPool)
			g_strPool = new StrPool;
		return g_strPool->Add(s);
	}
	return 0;
}

// 0x464e20
void StrPool_Destroy()
{
	StrPool *pool = g_strPool;
	if (!pool)
		return;
	StrBlock *head = pool->m_head;
	if (head) {
		if (head->m_next)
			head->m_next->Destroy(1);
		operator_delete(head);
	}
	operator_delete(pool);
	g_strPool = 0;
}


// The second parameter is the thunk's own argument slot.
void ScanSystemCommandChunks(const RWS::CAttributeDataChunk &first);
void ScanSystemCommandChunksThunk(const RWS::CAttributePacket &attr, int)
{
	ScanSystemCommandChunks(*(const RWS::CAttributeDataChunk *)&attr);
}
