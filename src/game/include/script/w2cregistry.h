// Normal build only: the three name registries of the script engine (0x1c bytes each in the game: a MSVC 7.1 std::map header
// {allocator, head, size} with its nil head node, then a 7.1 vector {allocator, first, last, end} of 0x10c-byte symbol entries).
// ONE definition for every translation unit that sees the record (typeresolver.cpp, typeregistry.cpp; the views LdRegistry in
// loadctx.cpp and SymEntryTable in reader.cpp use the same member types), so that its size on x64 (the head and the vector's pointers
// are real pointers) agrees everywhere: the array of three that ResolveSymbol allocates is indexed by sizeof of this record.
#ifndef ZOOSTERS_SCRIPT_W2CREGISTRY_H
#define ZOOSTERS_SCRIPT_W2CREGISTRY_H
#include <stddef.h>
#include "zoosters/vec71.h"
struct SymE10C { int d[0x43]; };
void *MemAlloc(size_t n);          // 0x550170
void operator_delete(void *p);     // 0x5500f0
// normal build: the 0x1c-byte record the other TUs see (typeresolver.cpp W2cRegistry, loadctx.cpp LdRegistry/LdMap/LdVec,
// symboltable.cpp): an MSVC 7.1 std::map header {allocator, head, size} with its nil head node (0x18 B: left, parent, right,
// key, value, color, isnil at +0x15) and a 7.1 vector {allocator, first, last, end}. Constructor 0x4342a0 and destructor
// 0x434300 as the game has them (nodes never get inserted).
// W2cSymTable: the symbol table of one registry (a 7.1 vector of SymE10C entries). Clear 0x4341f0 and Read 0x434100 are its
// members (script/symboltable.cpp); loadctx.cpp's LdVec and the readers (script/reader.cpp) address the same object.
struct W2cSymTable {
	zoosters::Vec71<SymE10C> m_vec;
	void Clear();   // 0x4341f0
	void Read();    // 0x434100 TFBRead_SymbolTable
};
struct W2cNode71 { W2cNode71 *left, *parent, *right; const char *key; void *value; char color; char isNil; };
struct W2cRegistry {
	int m_mapAlloc;
	W2cNode71 *m_head;
	int m_count;
	W2cSymTable m_vec;
	W2cRegistry()
	{
		W2cNode71 *h = (W2cNode71 *)MemAlloc(sizeof(W2cNode71) + 2);
		m_head = h;
		h->isNil = 1;
		h->parent = h;
		h->left = h;
		h->right = h;
		m_count = 0;
	}
	static void *operator new[](size_t n) { return MemAlloc(n); }
	~W2cRegistry()
	{
		m_vec.m_vec.reset();
		erase(m_head->parent);
		operator_delete(m_head);
		m_head = 0;
		m_count = 0;
	}
	void erase(W2cNode71 *n)
	{
		while (!n->isNil) {
			erase(n->right);
			W2cNode71 *l = n->left;
			operator_delete(n);
			n = l;
		}
	}
};
// GLOBAL: 0x61f874 DAT_0061f874
extern W2cRegistry *DAT_0061f874;   // the array of three (names, types, ...): built by ScriptReference::ResolveSymbol 0x434350
static_assert(sizeof(void *) != 4 || (sizeof(W2cRegistry) == 0x1c), "the record is 0x1c bytes");
static_assert(sizeof(void *) != 4 || (sizeof(W2cNode71) == 0x18 && offsetof(W2cNode71, key) == 0x0c && offsetof(W2cNode71, color) == 0x14 && offsetof(W2cNode71, isNil) == 0x15), "the nil head node of the 7.1 map (isnil at +0x15)");
static_assert(sizeof(void *) != 4 || (offsetof(W2cRegistry, m_head) == 4 && offsetof(W2cRegistry, m_count) == 8 && offsetof(W2cRegistry, m_vec) == 0x0c), "the game build's W2cRegistry: std::map header {alloc, head, size} then the vector at +0xc");
#endif
