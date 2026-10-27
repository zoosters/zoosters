// The script symbol registry: a pointer (0x61f878) to the one std::vector of slots (script object | tag bit 0, name key) that
// ResolveSymbol fills and everyone else reads. The game build uses the MSVC 7.1 std::vector, the normal build zoosters::Vec71
// (same layout).
#ifndef ZOOSTERS_SCRIPT_SYMBOLREGISTRY_H
#define ZOOSTERS_SCRIPT_SYMBOLREGISTRY_H
#include <stddef.h>
#include "zoosters/vec71.h"

struct SymE8 {
	uintptr_t obj;   // script object pointer | tag bit 0 (mask with ~(uintptr_t)1)
	unsigned key;    // name hash
};
typedef SymE8 SymbolSlot;
typedef zoosters::Vec71<SymE8> SymbolVec;
extern SymbolVec *g_symRegistryY;   // GLOBAL: 0x61f878 g_symRegistryY (the GLOBAL line is in each user .cpp)

#endif
