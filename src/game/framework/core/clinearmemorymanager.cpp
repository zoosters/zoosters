#include <string.h>
#include <vector>
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/memoryhandler.h"
#include "framework/core/streamhandler.h"
#include "rw/rwcore.h"

// std::vector<LinearMemoryPool *> instantiations the game emits out of line.
// 0x551550
// 0x551580
// 0x551640

namespace RWS {

namespace {
// Pool starts and request sizes are multiples of this.
const unsigned PoolAlignment = 64;
}  // namespace

// Oldest first.
// GLOBAL: 0x62fef8 RWS::g_storagePools
std::vector<CLinearMemoryManager::LinearMemoryPool *> g_storagePools;

// GLOBAL: 0x62fef4 RWS::CLinearMemoryManager::m_pCurrentPool
CLinearMemoryManager::LinearMemoryPool *CLinearMemoryManager::m_pCurrentPool = 0;

CLinearMemoryManager::CLinearMemoryManager()
{
	g_storagePools.reserve(8);
}

// 0x401590
CLinearMemoryManager *CLinearMemoryManager::Instance()
{
	static CLinearMemoryManager instance;
	return &instance;
}

void CLinearMemoryManager::DestroyMemoryPool(LinearMemoryPool *pool)
{
	delete[] pool->m_pStorage;
	delete pool;
}

// 0x551820
CLinearMemoryManager *CLinearMemoryManager::CreateMemoryPool(unsigned MemSize)
{
	LinearMemoryPool *pool = RWS_NEW LinearMemoryPool;
	pool->m_pStorage = RWS_NEW unsigned char[MemSize + PoolAlignment];
	uintptr_t start = reinterpret_cast<uintptr_t>(pool->m_pStorage);
	start = (start + PoolAlignment - 1) & ~static_cast<uintptr_t>(PoolAlignment - 1);
	pool->m_pStorageStart = reinterpret_cast<unsigned char *>(start);
	pool->m_pStorageNext = pool->m_pStorageStart;
	pool->m_pStorageEnd = pool->m_pStorageStart + MemSize;
	pool->m_uAllocCount = 0;
	g_storagePools.push_back(pool);
	m_pCurrentPool = pool;
	return Instance();
}

// 0x551510
void *CLinearMemoryManager::RequestMemory(unsigned size)
{
	LinearMemoryPool *pool = m_pCurrentPool;
	if (!pool)
		return 0;
	if (static_cast<size_t>(pool->m_pStorageEnd - pool->m_pStorageNext) < size)
		return 0;
	if (size % PoolAlignment != 0)
		return 0;
	void *memory = pool->m_pStorageNext;
	pool->m_pStorageNext += size;
	pool->m_uAllocCount++;
	return memory;
}

// 0x5515b0
int CLinearMemoryManager::FreeMemory(void *pMem)
{
	uintptr_t address = reinterpret_cast<uintptr_t>(pMem);
	for (size_t i = 0; i < g_storagePools.size(); i++) {
		LinearMemoryPool *pool = g_storagePools[i];
		if (address < reinterpret_cast<uintptr_t>(pool->m_pStorageStart) ||
		    address > reinterpret_cast<uintptr_t>(pool->m_pStorageEnd))
			continue;
		// memory is never reused inside a pool; it goes with its last allocation
		if (--pool->m_uAllocCount == 0) {
			g_storagePools.erase(g_storagePools.begin() + i);
			if (pool == m_pCurrentPool)
				m_pCurrentPool = 0;
			DestroyMemoryPool(pool);
		}
		return 1;
	}
	return 0;
}

// 0x462540
void CLinearMemoryManager::DestroyLinearMemoryManager()
{
	for (size_t i = 0; i < g_storagePools.size(); i++)
		DestroyMemoryPool(g_storagePools[i]);
	g_storagePools.clear();
	m_pCurrentPool = 0;
}

namespace anon {

// Class table chunk (0x71c), first of every stream: one pool sized for the stream's linear-pool instances.
// 0x5518d0
void SetupPlacementNewInfo(RwChunkHeaderInfo &header, RwStream *stream)
{
	unsigned char *buffer = static_cast<unsigned char *>(MemAllocHint(header.length, 2));
	RwStreamRead(stream, buffer, header.length);
	unsigned total = 0;
	if (buffer && header.length >= 4) {
		int count;
		memcpy(&count, buffer, 4);
		const unsigned char *cursor = buffer + 4;
		for (int i = 0; i < count; i++) {
			// padded to a multiple of 4
			const char *className = reinterpret_cast<const char *>(cursor);
			cursor += (strlen(className) + 1 + 3) & ~static_cast<size_t>(3);
			unsigned instances;
			memcpy(&instances, cursor, 4);
			cursor += 4;
			total += CClassFactory::GetRegisteredClassSize(className) * instances;
		}
	}
	CLinearMemoryManager::CreateMemoryPool(total);
	operator delete(buffer);
}

}  // namespace anon

// 0x551980
void CLinearMemoryManager::RegisterStreamChunkHandlers()
{
	CStreamHandler::RegisterChunkHandler(0x71c, anon::SetupPlacementNewInfo);
}

}  // namespace RWS
