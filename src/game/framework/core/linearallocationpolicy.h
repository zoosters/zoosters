// Base of the behaviours whose instances live in the linear memory pools.
#ifndef ZOOSTERS_FRAMEWORK_CORE_LINEARALLOCATIONPOLICY_H
#define ZOOSTERS_FRAMEWORK_CORE_LINEARALLOCATIONPOLICY_H

#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"

namespace RWS {

class LinearAllocationPolicy {
public:
	void *operator new(size_t size);
	void operator delete(void *p);
};

inline void *LinearAllocationPolicy::operator new(size_t size)
{
	const size_t align = CClassFactory::m_AlignmentSize;
	size_t rounded = (size + align - 1) / align * align;
	return CLinearMemoryManager::Instance()->RequestMemory(static_cast<unsigned>(rounded));
}

inline void LinearAllocationPolicy::operator delete(void *p)
{
	if (p)
		CLinearMemoryManager::Instance()->FreeMemory(p);
}

}  // namespace RWS

#endif
