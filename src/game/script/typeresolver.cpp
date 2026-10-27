// TFB_ResolveSymbol 0x434350 (the script symbol registry: find or create the object that a name stands for)
#include <vector>
#include "script/script.h"
#include "script/symbolregistry.h"
#include "script/types/scriptcamera.h"
#include "modules/fx/gameparticle.h"
#include "script/ops_message.h"
#include "script/ops_behavior.h"
#include "script/value.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptmessage.h"
using namespace Script;

extern "C" int __cdecl _stricmp(const char *, const char *);
void *MemAlloc(size_t n);                // 0x550170
void operator_delete(void *p);           // 0x5500f0
unsigned NameHashNoCase(const char *s);    // 0x43bb20
int FUN_0043e6f0(const char *msg);       // 0x43e6f0
ScriptReference *TFBType_Message();      // 0x43ad50
// GLOBAL: 0x43bb20 NameHashNoCase
// GLOBAL: 0x43e6f0 FUN_0043e6f0
// GLOBAL: 0x550170 MemAlloc
// GLOBAL: 0x5500f0 operator_delete
// GLOBAL: 0x43c2a0 Script::UserReference::UserReference
// GLOBAL: 0x5ce520 Script::OpBehavior::vftable
// GLOBAL: 0x5ce590 Script::ScriptMessage::vftable

// the symbol vector 0x61f9a8: 8-byte slots (object | flag bit 0, key); reserve 0x4338d0, push_back 0x433d30

// the three name registries (0x1c B each; the array pointer at 0x61f874), built by 0x4342a0 / 0x434300
#include "script/w2cregistry.h"
// GLOBAL: 0x4342a0 ??0W2cRegistry@@QAE@XZ
// GLOBAL: 0x434300 ??1W2cRegistry@@QAE@XZ

// GLOBAL: 0x61f874 DAT_0061f874
// GLOBAL: 0x61f878 g_symRegistryY
// GLOBAL: 0x621f24 g_symCountBase
extern int g_symCountBase;

// 0x42cb40 (ret 0xc): the temporary set 'subset' with a name and a type (stand-in; ScriptSubset in ops_set.h)
// the 0x28-byte stand-in is too small for ScriptSubset on x64 (heap overrun in the debug heap)
#include "script/ops_set.h"
typedef Script::ScriptSubset SubsetN;
// GLOBAL: 0x42cb40 SubsetN::SubsetN

#ifdef ZOOSTERS_DEBUGUI   // debug overlay only (port/debugui/debugui_triggers.cpp)
void port_dbg_symbol_name(Script::ScriptReference *object, const char *name);
void port_dbg_hash_name(unsigned key, const char *name);   // debugui_vars.cpp: hash -> name for the variable panel (locals too)
#endif

namespace Script {

static __declspec(noinline) int FindSymbolIndex(uintptr_t key)
{
	for (unsigned i = 0; i < g_symRegistryY->size(); i++) {
		if ((*g_symRegistryY)[i].key == key)
			return i;
	}
	return -1;
}

// 0x434350 Script::ScriptReference::ResolveSymbol
int ScriptReference::ResolveSymbol(NamedObject *script, const char *name, ScriptReference *other)
{
	if (g_symRegistryY == 0) {
		DAT_0061f874 = new W2cRegistry[3];
		static SymbolVec registry;
		g_symRegistryY = &registry;
		registry.reserve(0x9c4);
	}
	uintptr_t key = NameHashNoCase(FormatTypedName(name, other));
#ifdef ZOOSTERS_DEBUGUI
	port_dbg_hash_name((unsigned)key, name);   // before the local-script branch: locals are never in the registry
#endif
	if (script != 0 && _stricmp(script->GetAltName(), "TFB Script") == 0) {
		ScriptReference *obj;
		if (this == TFBType_User()) {
			UserReference *u = new UserReference((const char *)key, other, 0, 0);
			obj = u;
		} else if (this == TFBType_Set()) {
			SubsetN *t = new SubsetN((const char *)key, other, 0);
			obj = t;
		} else {
			OpBehavior *b = new (g_scriptPool) OpBehavior((const char *)key);
			b->m_startPC = 0;
			g_scriptPool += sizeof(OpBehavior);
			obj = b;
		}
		((OpContext *)script)->m_locals[((OpContext *)script)->m_localCount] = obj;   // a "TFB Script": the OpContext
		((OpContext *)script)->m_localCount++;
		obj->AdjustRefCount(1);
		FUN_0043e6f0(name);
		return -1;
	}
	int idx = FindSymbolIndex(key);
	if (idx == -1) {
		ScriptReference *obj;
		if (this == TFBType_User())
			obj = new UserReference((const char *)key, other, 0, 0);
		else if (this == TFBType_Set())
			obj = new SubsetN((const char *)key, other, 0);
		else if (this == TFBType_Message())
			obj = new ScriptMessage;
		else {
			obj = this;
			if (g_symCountBase != 0 && GetType() == this)
				obj = (ScriptReference *)((uintptr_t)this | 1);
		}
		int newIdx = g_symRegistryY->size();
		SymE8 slot;
		slot.obj = (uintptr_t)obj;
		slot.key = key;
		g_symRegistryY->push_back(slot);
		ScriptReference *registered;
		if (newIdx >= 0x3fc0d)
			registered = g_curScript->m_locals[newIdx - 0x3fc0d];
		else {
			uintptr_t v = (*g_symRegistryY)[newIdx].obj;
			if (v & 1)
				return newIdx;
			registered = (ScriptReference *)(v & ~(uintptr_t)1);
		}
#ifdef ZOOSTERS_DEBUGUI
		port_dbg_symbol_name((ScriptReference *)(slot.obj & ~(uintptr_t)1), name);
#endif
		if (registered != 0) {
			registered->AdjustRefCount(1);
			if (g_symCountBase != 0)
				FUN_0043e6f0(name);
		}
		return newIdx;
	}
	ScriptReference *found;
	if (idx >= 0x3fc0d)
		found = g_curScript->m_locals[idx - 0x3fc0d];
	else
		found = (ScriptReference *)((*g_symRegistryY)[idx].obj & ~(uintptr_t)1);
#ifdef ZOOSTERS_DEBUGUI
	port_dbg_symbol_name(found, name);
#endif
	if (found == this)
		idx = -1;
	return idx;
}

}  // namespace Script

