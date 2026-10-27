// The pools behind LinearAllocationPolicy. No instance data; the pool list lives in the .cpp.
#ifndef ZOOSTERS_FRAMEWORK_CORE_CLINEARMEMORYMANAGER_H
#define ZOOSTERS_FRAMEWORK_CORE_CLINEARMEMORYMANAGER_H

#include "zoosters/base.h"

namespace RWS {

class CLinearMemoryManager {
public:
	// m_pStorageStart: first 64-byte aligned address of m_pStorage
	struct LinearMemoryPool {
		unsigned char *m_pStorage;
		unsigned char *m_pStorageStart;
		unsigned char *m_pStorageNext;
		unsigned char *m_pStorageEnd;
		unsigned m_uAllocCount;
	};

	static CLinearMemoryManager *Instance();   // function-local static
	static CLinearMemoryManager *CreateMemoryPool(unsigned MemSize);
	static void *RequestMemory(unsigned size);
	static int FreeMemory(void *pMem);
	static void RegisterStreamChunkHandlers();
	static void DestroyLinearMemoryManager();

private:
	CLinearMemoryManager();
	static void DestroyMemoryPool(LinearMemoryPool *pool);
	static LinearMemoryPool *m_pCurrentPool;
};
typedef CLinearMemoryManager::LinearMemoryPool LinearMemoryPoolRecord;
ZOOSTERS_ASSERT_SIZE(LinearMemoryPoolRecord, 20);

}  // namespace RWS

#endif
