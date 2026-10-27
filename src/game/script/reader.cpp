// TFB script parse-time readers: operand references and values (called from every op's Parse).
#include "script/script.h"
#include "script/runtime.h"
#include "script/symbolregistry.h"
#include <vector>
#include "script/value.h"
#include "script/ops_object.h"

// GLOBAL: 0x603498 g_valueKindNames
extern const char *g_valueKindNames[3];
// 4th argument of the ConstU* readers: the game pushes the immediate 0x616161 ("aaa"), no relocation
enum { kConstReadTag = 0x616161 };
// GLOBAL: 0x43fc50 Script::ScriptValueReader::ConstU32
// GLOBAL: 0x43fc90 Script::ScriptValueReader::ConstU8
// GLOBAL: 0x43fcd0 Script::ScriptValueReader::ConstU16

extern "C" int __cdecl _stricmp(const char *, const char *);

namespace Script {

// 0x43fd20
ScriptValueReader::ScriptValueReader(ScriptValue *v, ScriptReference *type, int label, volatile int a4)
{
	// Distinct int/float overloads of ConstU32 (runtime.h): the game calls the int and the float variant, which the
	// linker folded into one function (0x43fc50); two different callees keep cl from tail-merging the equal case
	// bodies (kind 0 = int constant, 0x10 = float; 0x80 is rewritten to 0x10).
	// The game has one 0x11c-byte object in this frame (esp+0x1c): 0x42fc60 runs on the same storage as the
	// ScriptRefReader (frame 0x120), so the enum reader is constructed in place over `r` rather than as a temporary.
	ScriptRefReader r(0);
	unsigned char *pk = &v->kind;
	if (*pk == 2)
		TFBRead_u8(pk);
	else
		new (&r) ScriptEnumReader(pk, g_valueKindNames, v);
	if (*pk == 2 && (type == 0 || type->IsKindOf(TFBType_Int()) != 0)) {
		TFBType_Int();
		r.Read(&v->lhs);
		// HACK: the game does a dead load of the stack argument a4 here (mov ecx,[esp+13Ch], result unused: some
		// inlined no-op that took a4). `volatile int a4` above (top-level volatile does not change the signature)
		// makes cl keep the load; it lands in eax instead of ecx, the only remaining difference.
		a4;
		TFBRead_u8(&v->op);
		unsigned char *pr = &v->rhsKind;
		if (*pr == 2)
			TFBRead_u8(pr);
		else
			new (&r) ScriptEnumReader(pr, g_valueKindNames, &v->rhs);
		if (*pr != 2) {
			switch (*pr & 0xf0) {
			case 0x80:
				*pr = (*pr & 0xf) | 0x10;
			case 0x10:
				r.ConstU32((float *)&v->rhs, pr, type, kConstReadTag);
				break;
			case 0:
				r.ConstU32((int *)&v->rhs, pr, type, kConstReadTag);
				break;
			}
		} else {
			TFBType_Int();
			r.Read(&v->rhs);
		}
	} else {
		v->rhsKind = 0;
		v->rhs.bits = 0;
		v->op = 0;
		if (v->kind == 2) {
			type->GetType();
			r.Read(&v->lhs);
		} else {
			switch (v->kind & 0xf0) {
			case 0:
				r.ConstU32((int *)v, pk, type, kConstReadTag);
				break;
			case 0x80:
				*pk = (v->kind & 0xf) | 0x10;
			case 0x10:
				r.ConstU32((float *)v, pk, type, kConstReadTag);
				break;
			case 0x20:
				r.ConstU8((char *)v, pk, type, kConstReadTag);
				r.ConstU8((char *)v + 1, pk, type, kConstReadTag);
				r.ConstU8((char *)v + 2, pk, type, kConstReadTag);
				r.ConstU8((char *)v + 3, pk, type, kConstReadTag);
				break;
			case 0x30:
				r.ConstU16(v, pk, type, kConstReadTag);
				r.ConstU16((char *)v + 2, pk, type, kConstReadTag);
				break;
			}
		}
	}
	if (label == 0)
		((ValueInterface *)type)->GetValueType();
}

}  // namespace Script

namespace Script {

// symbol tables used by ScriptRefReader::Read (layouts of the loader 0x4349e0 and the registry 0x434350)
// A table of declared names: entries of 0x10c bytes (name, ..., the reference bits, a use counter).
struct SymEntry {
	char name[0x100];
	int m100;
	unsigned bits;      // +0x104: the reference value this name stands for
	int useCount;       // +0x108
};
ZOOSTERS_ASSERT_SIZE(SymEntry, 0x10c);
// = one of the three name registries (the W2cRegistry of script/w2cregistry.h in the normal build, 0x1c B): a std::map header (allocator, head, size),
// then a std::vector of entries (allocator, first, ...)
typedef std::vector<ScriptReference *> TypeList;

}  // namespace Script

#include "script/w2cregistry.h"
#define SYM_ENTRIES(i) (((Script::SymEntry *)DAT_0061f874->m_vec.m_vec.m_first)[i])
Script::TypeList *TFB_GetTypeList();
// splits the entry name into the symbol name, its type ("user", "set" or a type)
// and, for user/set, the element type
void TFB_SplitTypedName(const char *src, char *a, char *b, char *c);   // 0x432a10

namespace Script {

// 0x432f40
// index of the registry slot with this key, -1 if none (the game passes the key in ebx)
static __declspec(noinline) int FindSymbolIndex(int key)
{
	for (unsigned i = 0; i < g_symRegistryY->size(); i++) {
		if ((*g_symRegistryY)[i].key == key)
			return i;
	}
	return -1;
}

// 0x4346f0
void ScriptRefReader::Read(ScriptRef *ref)
{
	char symName[0x100];
	char typeName[0x100];
	char elemTypeName[0x100];
	TFBRead_u32(&ref->bits);
	if (ref->bits == 0xffffffff)
		return;
	unsigned idx = ref->bits >> 14;
	if (idx < 0x3fc0d) {
		SymEntry *entry = &SYM_ENTRIES(idx);
		bool isFind;
		if (g_tfbParseOp->IsBlock() == 6 && ((VariableFind *)g_tfbParseOp)->m_notUser == -1)
			isFind = true;
		else
			isFind = false;
		int sym = FindSymbolIndex(entry->bits);
		if (!isFind && sym >= 0) {
			ScriptReference *obj;
			if (sym >= 0x3fc0d) {
				obj = g_curScript->m_locals[sym - 0x3fc0d];
			} else {
				SymbolSlot *ss = &(*g_symRegistryY)[sym];
				obj = (ss->obj & 1) ? 0 : (ScriptReference *)(ss->obj & ~(uintptr_t)1);
			}
			if (obj == 0)
				entry->useCount++;
		} else {
			TFB_SplitTypedName(entry->name, symName, typeName, elemTypeName);
			entry->useCount++;
			TypeList *list = TFB_GetTypeList();
			// The first type with this name decides; if none matches, `sym` stays the slot index found above.
			for (unsigned i = 0; i < list->size(); i++) {
				if (_stricmp(typeName, (*list)[i]->GetAltName()) != 0)
					continue;
				if (_stricmp(typeName, TFBType_User()->GetAltName()) != 0 &&
				    _stricmp(typeName, TFBType_Set()->GetAltName()) != 0) {
					ScriptReference **np = &(*list)[i];
					sym = ((ScriptReference *)*np)->ResolveSymbol(0, symName, 0);
					break;
				}
				// a user object or a set: the element type names what it holds
				for (unsigned j = 0; j < list->size(); j++) {
					if (_stricmp(elemTypeName, (*list)[j]->GetAltName()) != 0)
						continue;
					ScriptReference *elemType;
					ScriptReference *&slot = (*list)[j];
					if (slot == TFBType_Value())
						elemType = TFBType_Int();
					else
						elemType = (*list)[j];
					if (isFind) {
						entry->useCount--;
						m_110 = elemType;
						((VariableFind *)g_tfbParseOp)->m_type = elemType;
						int notUser = _stricmp(typeName, TFBType_User()->GetAltName()) != 0;
						m_10c = notUser;
						((VariableFind *)g_tfbParseOp)->m_notUser = notUser;
						ref->bits = entry->bits;
						return;
					}
					if (_stricmp(typeName, TFBType_User()->GetAltName()) == 0)
						sym = ((ScriptReference *)TFBType_User())->ResolveSymbol(0, symName, (ScriptReference *)elemType);
					else
						sym = ((ScriptReference *)TFBType_Set())->ResolveSymbol(0, symName, (ScriptReference *)elemType);
					break;
				}
				break;
			}
		}
		ref->bits = (ref->bits & 0x3fff) | (sym << 14);
	}
	if (g_scriptLastType == 0)
		g_scriptLastType = g_curScript->Lookup(ref, -1);
}

}  // namespace Script
