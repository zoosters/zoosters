#include "script/runtime.h"
#include "script/symbolregistry.h"

// 0x416220
uintptr_t TFB_ObjectFromIndex(int index, char userFlag)
{
	if (index >= 0x3fc0d)
		return (uintptr_t)g_curScript->m_locals[index - 0x3fc0d];
	SymbolSlot *slot = &(*g_symRegistryY)[index];
	int f = userFlag == 1;
	if (slot->obj & f)
		return 0;
	return slot->obj & ~(uintptr_t)1;
}
