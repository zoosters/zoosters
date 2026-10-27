// 0x433ec0 release the script instances of the symbol registry from index `base` on, then shrink
// the registry to `base` entries (Script::TypeRegistry_ResetAll calls it)
#include <vector>
#include "script/runtime.h"
#include "script/symbolregistry.h"
using namespace Script;

// GLOBAL: 0x61f878 g_symRegistryY
int FUN_0043e6f0(const char *fmt, ...);   // 0x43e6f0 (debug print, empty in this build)
// GLOBAL: 0x43e6f0 FUN_0043e6f0

// 0x433ec0 SymbolRegistry_ReleaseFrom
void SymbolRegistry_ReleaseFrom(int base)
{
	FUN_0043e6f0("Clearing %d script instances\n", g_symRegistryY->size());
	unsigned i = base;
	int localOff = (base - 0x3fc0d) * (int)sizeof(ScriptReference *);   // byte offset into g_curScript->m_locals
	while (i < g_symRegistryY->size()) {
		ScriptReference *obj;
		if (localOff >= 0)
			obj = *(ScriptReference **)((char *)g_curScript->m_locals + localOff);
		else {
			uintptr_t slotBits = (*g_symRegistryY)[i].obj;
			if (slotBits & 1) {
				obj = 0;
				(*g_symRegistryY)[i].obj = (uintptr_t)obj;
				i++;
				localOff += (int)sizeof(ScriptReference *);
				continue;
			}
			obj = (ScriptReference *)(slotBits & ~(uintptr_t)1);
		}
		if (obj != 0) {
			if (obj->AdjustRefCount(-1) == 0)
				obj = 0;
			else
				obj->AdjustRefCount(1);
		}
		(*g_symRegistryY)[i].obj = (uintptr_t)obj;
		i++;
		localOff += (int)sizeof(ScriptReference *);
	}
	i = base;
	while (i < g_symRegistryY->size()) {
		ScriptReference *obj = (ScriptReference *)(*g_symRegistryY)[i].obj;
		if (obj != 0) {
			int refs = obj->AdjustRefCount(-1);
			if (refs == -1)
				delete obj;
			else if (refs != 0)
				FUN_0043e6f0("POSSIBLE LEAK: Could not release script instance %d - 0x%p of type '%s' [%d]!\n"   /* the game's text at 0x5ce6b0 */, i, obj, obj->GetType()->GetName(), refs);
		}
		i++;
	}
	SymE8 zero;
	zero.key = 0;
	zero.obj = 0;
	if (g_symRegistryY->size() < (unsigned)base)
		g_symRegistryY->insert(g_symRegistryY->end(), base - g_symRegistryY->size(), zero);
	else
		g_symRegistryY->erase(g_symRegistryY->begin() + base, g_symRegistryY->end());
}
