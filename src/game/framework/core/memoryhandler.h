// Memory handler: MemAlloc and friends forward to the CRT.
#ifndef ZOOSTERS_FRAMEWORK_CORE_MEMORYHANDLER_H
#define ZOOSTERS_FRAMEWORK_CORE_MEMORYHANDLER_H

#include <stddef.h>

class CMemHandler {
public:
	static CMemHandler &GetRef();   // (function-local static)
	CMemHandler() : m_doingMsg(false) {}
	~CMemHandler() {}
	bool m_doingMsg;
};

void *MemAlloc(size_t size);
void *MemAllocHint(size_t size, int userTypeNum);
void *MemCalloc(size_t num, size_t size, int userTypeNum);
void *MemRealloc(void *p, size_t size);

// The game's own operator delete (0x5500f0), defined as operator_delete in memoryhandler.cpp.
void operator delete(void *p) throw();
// The type of `new (type)` is an int: a placement `operator delete(void *, size_t)` would be taken for the usual deallocation
// function under C++14 sized deallocation (C2956).
typedef int RwsMemHint;
inline void *operator new(size_t size, RwsMemHint type)
{
	return MemAllocHint(size, (int)type);
}
inline void operator delete(void *p, RwsMemHint) throw()
{
	operator delete(p);
}
inline void *operator new[](size_t size, RwsMemHint type)
{
	return MemAllocHint(size, (int)type);
}
inline void operator delete[](void *p)
{
	operator delete(p);
}
inline void operator delete[](void *p, RwsMemHint) throw()
{
	operator delete(p);
}

// Type 2: the framework's own allocations.
#define RWS_NEW new(2)

#endif
