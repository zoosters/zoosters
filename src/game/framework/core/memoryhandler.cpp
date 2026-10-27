// The malloc wrappers behind `new` / `delete`.
#include "framework/core/memoryhandler.h"

#include <stdlib.h>

// 0x550130
CMemHandler &CMemHandler::GetRef()
{
	static CMemHandler memHandler;
	return memHandler;
}

// 0x550170
void *MemAlloc(size_t size)
{
	CMemHandler::GetRef();
	if (size)
		return malloc(size);
	return 0;
}

// 0x550190
void *MemAllocHint(size_t size, int userTypeNum)
{
	CMemHandler::GetRef();
	if (size)
		return malloc(size);
	return 0;
}

// 0x5500f0
// Plain name: annotating the real operator would make arity_check match the headers' inline operator delete(void*, unsigned) to it.
void operator_delete(void *pMem)
{
	if (pMem)
		free(pMem);
}

// 0x5500e0
// Placement delete of the RWS_NEW funclets; a plain function because the headers define the inline operator.
void operator_delete_unwind(void *pMem, unsigned type)
{
	if (pMem)
		free(pMem);
}

// 0x550110
void *MemCalloc(size_t num, size_t size, int hint)
{
	if (num && size)
		return calloc(num, size);
	return 0;
}

// 0x5501b0
void *MemRealloc(void *p, size_t size)
{
	if (size) {
		CMemHandler::GetRef();
		return realloc(p, size);
	}
	return 0;
}
