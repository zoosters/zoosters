// Script::OpContext::Load 0x4349e0 (loads a compiled script: header, three symbol tables, the node stream)
#include "script/runtime.h"
#include "script/symbolregistry.h"
#include "script/script.h"
#include "script/ops_behavior.h"
#include <string.h>
#include <vector>
using namespace Script;

extern "C" int __cdecl _stricmp(const char *, const char *);
void *MemAlloc(size_t n);                 // 0x550170
void operator_delete(void *p);            // 0x5500f0
// GLOBAL: 0x550170 MemAlloc
// GLOBAL: 0x5500f0 operator_delete
ScriptReference *TFBType_Behavior();      // 0x4318f0
ScriptReference *TFBType_User();          // 0x43c530
ScriptReference *TFBType_Set();           // 0x43d2d0
ScriptReference *TFBType_Value();         // 0x42f900
ScriptReference *TFBType_Int();           // 0x42fdb0
void TFBRead_u32(unsigned int *out);      // 0x432920
void TFBRead_pstr(char *out);             // 0x4329a0
void TFB_SplitTypedName(const char *src, char *a, char *b, char *c);   // 0x432a10
// GLOBAL: 0x432920 TFBRead_u32
// GLOBAL: 0x4329a0 TFBRead_pstr
// GLOBAL: 0x432a10 TFB_SplitTypedName
// GLOBAL: 0x4318f0 TFBType_Behavior
// GLOBAL: 0x43c530 TFBType_User
// GLOBAL: 0x43d2d0 TFBType_Set
// GLOBAL: 0x42f900 TFBType_Value
// GLOBAL: 0x42fdb0 TFBType_Int

// GLOBAL: 0x61f884 g_tfbLoadArg
// GLOBAL: 0x61f888 g_tfbReadPos
// GLOBAL: 0x61f88c g_tfbReadBuf
// GLOBAL: 0x61f890 g_scriptPool
// GLOBAL: 0x61f89c g_tfbPending
// GLOBAL: 0x61f8a0 g_tfbPending2
// GLOBAL: 0x61f8a4 g_tfbPending3
// GLOBAL: 0x61f874 DAT_0061f874
// GLOBAL: 0x61f878 g_symRegistryY
// GLOBAL: 0x61f658 g_scriptPC
// GLOBAL: 0x61f65c g_curScript
// GLOBAL: 0x61f87c g_tfbParseEnd
// GLOBAL: 0x61f880 g_tfbParseOp
// GLOBAL: 0x61f870 g_scriptLastType
extern unsigned g_tfbLoadArg;
extern int g_tfbReadPos;
extern unsigned char *g_tfbReadBuf;
extern void *g_tfbPending;
extern int g_tfbPending2;
extern int g_tfbPending3;

// symbol table entry (0x10c B), the same as SymEntry in reader.cpp
struct LdEntry {
	char name[0x100];
	int m100;
	unsigned bits;      // +0x104
	int useCount;       // +0x108
};
// std::vector<SymE10C>: allocator at +0, first/last/end
struct LdVec {
	int alloc;
	LdEntry *first;
	LdEntry *last;
	LdEntry *end;
	LdVec() : first(0), last(0), end(0) {}
	~LdVec() { if (first) operator_delete(first); }
	unsigned size() const { return first == 0 ? 0 : last - first; }
	void ReadTable();   // 0x434100 TFBRead_SymbolTable
	void Clear();       // 0x4341f0
};
// GLOBAL: 0x434100 ?ReadTable@LdVec@@QAEXXZ
// GLOBAL: 0x4341f0 ?Clear@LdVec@@QAEXXZ
// std::map header (12 B) + vector (0x10 B) = one of the three registries at DAT_0061f874
struct LdMap {
	int alloc;
	void *head;
	int count;
	void Erase(void *node);   // 0x432ea0 (_Tree::_Erase)
};
// GLOBAL: 0x432ea0 ?Erase@LdMap@@QAEXPAX@Z
struct LdRegistry {
	LdMap map;
	LdVec vec;
};
// DAT_0061f874 is the registry array of script/w2cregistry.h; LdMap / LdVec are this file's views of its members
#include "script/w2cregistry.h"
#define LD_REG(i) (*(LdRegistry *)&DAT_0061f874[i])
static_assert(sizeof(LdRegistry) == sizeof(W2cRegistry), "LdRegistry is the layout of one W2cRegistry");
struct LdTreeNode {
	LdTreeNode *left, *parent, *right;
	const char *key;   // +0xc: _Myval (the pair) of the MSVC 7.1 _Tree node
	void *value;       // +0x10
	char color;        // +0x14: _Color
	char isNil;   // +0x15
	void *val;
};
// MSVC 7.1 _Tree::_Erase (0x432ea0, library code at a game address): frees the subtree under `node` without rebalancing (right
// subtree recursively, then down the left spine) until the nil sentinel. The registries keep the MSVC 7.1 node layout above.
void LdMap::Erase(void *node)
{
	LdTreeNode *n = static_cast<LdTreeNode *>(node);
	while (!n->isNil) {
		Erase(n->right);
		LdTreeNode *left = n->left;
		operator_delete(n);
		n = left;
	}
}
typedef std::vector<ScriptReference *> LdTypeList;
LdTypeList *TFB_GetTypeList();   // 0x43c810 TFB_GetTypeList
// GLOBAL: 0x43c810 TFB_GetTypeList

namespace Script {
}

// Script pool: one block for the node table and every op object. The file's header `count` is the byte total of the 32-bit build; ops and the node
// table hold pointers, so the 64-bit pool is bigger: OpContext::Load measures it (MeasureScriptPool below: the same Clone calls into a scratch buffer,
// nothing parsed) and allocates that.
#include <stdio.h>
extern "C" FILE *port_log;
// The exact pool bytes of this script: node table (nodeCount + 1 pointers), one OpBehavior per local behavior, the three section
// header objects, and the size of every op the node loop clones. Mirrors the allocation order of Load; reads the node records
// (op, flags, length, payload) from the position the tables left off at and restores it.
static size_t MeasureScriptPool(unsigned nodeCount, LdVec &ops, LdVec &locals, size_t *skippedOut)
{
	size_t bytes = nodeCount * sizeof(OpCode *) + sizeof(OpCode *);
	LdTypeList *types = TFB_GetTypeList();
	char subName[0x100], symName[0x100], typeName[0x100];
	LdEntry *entry = locals.first;
	for (unsigned i = 0; i + 1 < locals.size(); entry++, i++) {   // the Resolve loop of Load: a behavior local is an OpBehavior in the pool
		TFB_SplitTypedName(entry->name, symName, typeName, subName);
		unsigned typeIdx = 0;
		while (typeIdx < types->size() && _stricmp(subName, (*types)[typeIdx]->GetAltName()) != 0)
			typeIdx++;
		if (typeIdx < types->size() && _stricmp(typeName, TFBType_Behavior()->GetAltName()) == 0)
			bytes += sizeof(OpBehavior);
	}
	if (nodeCount == 0)
		return bytes;
	char *savedPool = g_scriptPool;
	alignas(16) static char scratch[4096];
	int pos = g_tfbReadPos;
	unsigned sections = 0;
	size_t skipped = 0;   // the ops of disabled nodes (flags & 0x40): not cloned, but the file's count includes them (log only)
	for (unsigned n = 0; n < nodeCount; n++) {
		unsigned char op = g_tfbReadBuf[pos++];
		unsigned flags = g_tfbReadBuf[pos] | g_tfbReadBuf[pos + 1] << 8 | g_tfbReadBuf[pos + 2] << 16 | (unsigned)g_tfbReadBuf[pos + 3] << 24;
		pos += 4;
		unsigned char len = g_tfbReadBuf[pos++];
		int next = pos + len;
		if (op == 0xff) {
			if (sections == 0)
				bytes += sizeof(OpPreScript);
			else if (sections == 1)
				bytes += sizeof(OpStartup);
			else if (sections == 2)
				bytes += sizeof(OpShutdown);
			sections++;
		} else {
			size_t *acc = (flags & 0x40) ? &skipped : &bytes;
			unsigned key = ((LdEntry *)((char *)ops.first + op * 0x10c))->bits;
			int sym = 0;
			while (sym < (int)g_symRegistryY->size() && (*g_symRegistryY)[sym].key != key)
				sym++;
			if (sym >= (int)g_symRegistryY->size())
				sym = -1;
			if (sym >= 0x3fc0d) {
				*acc += sizeof(OpBehavior);   // a local behavior as the prototype: OpBehavior::Clone
			} else if (sym >= 0) {
				ScriptReference *proto = (ScriptReference *)((*g_symRegistryY)[sym].obj & ~(uintptr_t)1);
				g_scriptPool = scratch;
				((OpCode *)proto)->Clone();
				*acc += g_scriptPool - scratch;
			}
		}
		pos = next;
	}
	g_scriptPool = savedPool;
	*skippedOut = skipped;
	return bytes;
}
// 0x4349e0 Script::OpContext::Load
bool OpContext::Load(unsigned arg, unsigned char **buf)
{
	g_tfbLoadArg = arg;
	g_tfbParseEnd = 0;
	g_tfbReadPos = 0;
	g_tfbReadBuf = *buf;
	m_nodes = 0;
	char subName[0x100];
	char symName[0x100];
	char typeName[0x100];
	TFBRead_pstr(typeName);   // the script header name ("TFB Script" = GetAltName) goes through the type-name buffer
	if (strcmp(typeName, GetAltName()) != 0)
		return false;
	unsigned count;
	TFBRead_u32(&count);
	if (count == 0)
		return false;
	OpContext *savedScript = g_curScript;
	int savedPC = g_scriptPC;
	g_curScript = this;
	LdVec ops;
	ops.ReadTable();
	LD_REG(0).vec.ReadTable();
	LD_REG(1).vec.ReadTable();
	char *poolBase = 0;
	size_t poolMeasured = 0;
	unsigned nodeCount;
	TFBRead_u32(&nodeCount);
	if (nodeCount < 3)
		nodeCount = 0;
	{
		size_t skipped;
		size_t measured = MeasureScriptPool(nodeCount, ops, LD_REG(1).vec, &skipped);
		if (port_log)
			fprintf(port_log, "scriptpool: measured %u bytes + %u of disabled nodes, file count*4 = %u (%s)\n", (unsigned)measured, (unsigned)skipped, count * 4, measured + skipped == count * 4 ? "equal" : "differs");
		g_scriptPool = (char *)MemAlloc(measured);
		poolBase = g_scriptPool;
		poolMeasured = measured;
	}
	char *pool = g_scriptPool;
	g_scriptPool = pool + nodeCount * sizeof(OpCode *) + sizeof(OpCode *);
	m_nodes = (OpCode **)pool;
	m_locals = (ScriptReference **)MemAlloc(LD_REG(1).vec.size() * sizeof(ScriptReference *) - sizeof(ScriptReference *));
	m_localCount = 0;
	LdTypeList *types = TFB_GetTypeList();
	unsigned i = 0;
	LdEntry *entry = LD_REG(1).vec.first;
	for (; i < LD_REG(1).vec.size() - 1; entry++, i++) {
		TFB_SplitTypedName(entry->name, symName, typeName, subName);
		unsigned typeIdx = 0;
		while (typeIdx < types->size() && _stricmp(subName, (*types)[typeIdx]->GetAltName()) != 0)
			typeIdx++;
		if (typeIdx >= types->size())
			continue;
		if (_stricmp(typeName, TFBType_Behavior()->GetAltName()) == 0) {
			((ScriptReference *)TFBType_Behavior())->ResolveSymbol(this, symName, 0);
			continue;
		}
		ScriptReference *valueType = (*types)[typeIdx] == TFBType_Value() ? TFBType_Int() : (*types)[typeIdx];
		if (_stricmp(typeName, TFBType_User()->GetAltName()) == 0)
			((ScriptReference *)TFBType_User())->ResolveSymbol(this, symName, valueType);
		else
			((ScriptReference *)TFBType_Set())->ResolveSymbol(this, symName, valueType);
	}
	g_scriptPC = 0;
	if (nodeCount != 0) {
		count = 0;   // the game reuses the slot of `count` for the header counter
		int localOff = -3 * (int)sizeof(ScriptReference *);
		do {
			unsigned char op;
			unsigned char flagBytes[4];
			unsigned char len;
			unsigned flags;
			op = g_tfbReadBuf[g_tfbReadPos++];
			flagBytes[0] = g_tfbReadBuf[g_tfbReadPos++];
			flagBytes[1] = g_tfbReadBuf[g_tfbReadPos++];
			flagBytes[2] = g_tfbReadBuf[g_tfbReadPos++];
			flagBytes[3] = g_tfbReadBuf[g_tfbReadPos++];
			flags = (((flagBytes[3] << 8 | flagBytes[2]) << 8 | flagBytes[1]) << 8) | flagBytes[0];
			len = g_tfbReadBuf[g_tfbReadPos++];
			g_tfbParseEnd = g_tfbReadPos + len;
			OpCode *node;
			if (op == 0xff) {
				ScriptReference *obj;
				switch (count) {
				case 0:
					obj = new (g_scriptPool) OpPreScript("PRESCRIPT");
					g_scriptPool += sizeof(OpPreScript);
					break;
				case 1:
					obj = new (g_scriptPool) OpStartup("STARTUP");
					g_scriptPool += sizeof(OpStartup);
					break;
				case 2:
					obj = new (g_scriptPool) OpShutdown("SHUTDOWN");
					g_scriptPool += sizeof(OpShutdown);
					break;
				case 3:
					((OpBehavior *)m_nodes[0])->m_startPC = g_scriptPC;
				default:
					obj = *(ScriptReference **)((char *)m_locals + localOff);
					break;
				}
				((OpBehavior *)obj)->m_startPC = g_scriptPC;
				count++;
				localOff += (int)sizeof(ScriptReference *);
				node = (OpCode *)obj;
			} else if (flags & 0x40) {
				node = 0;   // no node for this slot; the null check below skips the parse
			} else {
				unsigned key = ((LdEntry *)((char *)ops.first + op * 0x10c))->bits;
				int sym = 0;
				while (sym < (int)g_symRegistryY->size() && (*g_symRegistryY)[sym].key != key)
					sym++;
				if (sym >= (int)g_symRegistryY->size())
					sym = -1;
				ScriptReference *proto;
				if (sym >= 0x3fc0d)
					proto = m_locals[sym - 0x3fc0d];
				else
					proto = (ScriptReference *)((*g_symRegistryY)[sym].obj & ~(uintptr_t)1);
				node = ((OpCode *)proto)->Clone();
			}
			if (node != 0) {
				node->m_flags = flags;
				g_tfbParseOp = node;
				if (g_tfbPending != 0)
					operator_delete(g_tfbPending);
				g_tfbPending = 0;
				g_tfbPending2 = 0;
				g_tfbPending3 = 0;
				g_scriptLastType = 0;
				node->Parse();
			}
			m_nodes[g_scriptPC] = node;
			g_tfbReadPos = g_tfbParseEnd;
			g_tfbParseEnd = 0;
			g_scriptPC++;
		} while ((unsigned)g_scriptPC < nodeCount);
	}
	if (m_nodes != 0)
		m_nodes[g_scriptPC] = 0;
	if (port_log && g_scriptPool - poolBase != (ptrdiff_t)poolMeasured)
		fprintf(port_log, "scriptpool: USED %d bytes of the measured %u (%s)\n", (int)(g_scriptPool - poolBase), (unsigned)poolMeasured, g_scriptPool - poolBase > (ptrdiff_t)poolMeasured ? "OVERRUN" : "less");
	g_scriptPC = savedPC;
	g_curScript = savedScript;
	{
		LdMap *map = &LD_REG(1).map;
		LdTreeNode *n = (LdTreeNode *)((LdTreeNode *)map->head)->parent;
		while (!n->isNil) {
			map->Erase(n->right);
			LdTreeNode *l = n->left;
			operator_delete(n);
			n = l;
		}
		((LdTreeNode *)map->head)->parent = (LdTreeNode *)map->head;
		map->count = 0;
		((LdTreeNode *)map->head)->left = (LdTreeNode *)map->head;
		((LdTreeNode *)map->head)->right = (LdTreeNode *)map->head;
	}
	LD_REG(1).vec.Clear();
	LD_REG(0).vec.Clear();
	return m_nodes != 0;
}

// the symbol table of the game (W2cSymTable of script/w2cregistry.h: Clear 0x4341f0, Read 0x434100); LdVec is this file's view of it
inline void LdVec::Clear() { ((W2cSymTable *)this)->Clear(); }
inline void LdVec::ReadTable() { ((W2cSymTable *)this)->Read(); }

#include "script/folded.h"
#include "script/ops_message.h"
// FOLDED: 0x431840 Script::OpBehavior::GetAltName
char const * Script::OpBehavior::GetAltName() { return OpBehavior_GetAltName(); }
// FOLDED: 0x4318f0 Script::OpBehavior::GetType
Script::ScriptReference * Script::OpBehavior::GetType() { return TFBType_Behavior(); }
// FOLDED: 0x43ba80 Script::OpBehavior::GetName
char const * Script::OpBehavior::GetName() { return m_name; }   // same function as UserReference::GetName in the game
// FOLDED: 0x431850 Script::OpPreScript::Parse
int Script::OpPreScript::Parse() { return Script::OpBehavior::Parse(); }
// FOLDED: 0x43e6f0 Script::OpPreScript::GetKind
int Script::OpPreScript::GetKind() { return 0; }
// FOLDED: 0x431850 Script::OpShutdown::Parse
int Script::OpShutdown::Parse() { return Script::OpBehavior::Parse(); }
// FOLDED: 0x43e6f0 Script::OpShutdown::GetKind
int Script::OpShutdown::GetKind() { return 0; }
// FOLDED: 0x431850 Script::OpStartup::Parse
int Script::OpStartup::Parse() { return Script::OpBehavior::Parse(); }
// FOLDED: 0x43e6f0 Script::OpStartup::GetKind
int Script::OpStartup::GetKind() { return 0; }
// FOLDED: 0x431870 Script::OpBehavior::GetBaseType
Script::ScriptReference *Script::OpBehavior::GetBaseType() { return TFBType_OpCode(); }   // OpCode::GetType
