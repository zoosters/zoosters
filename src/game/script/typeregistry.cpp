// the script type registry's std::map<const char *, X *, cstrless> helpers (tree members at 0x432ea0 .. 0x434300)
#include <map>
#include <vector>
#include <string.h>

struct W2cEntry;
struct w2cless { // std::binary_function is gone in C++17
	typedef const char *first_argument_type;
	typedef const char *second_argument_type;
	typedef bool result_type;
	bool operator()(const char *x, const char *y) const { return strcmp(x, y) < 0; }
};
typedef std::map<const char *, W2cEntry *, w2cless> W2cMap;

// 0x432ea0
// 0x434270
// 0x433c70
void w2cMapCtor(W2cMap *m) { new (m) W2cMap; }
void w2cMapDtor(W2cMap &m) { m.~W2cMap(); }
void w2cMapClear(W2cMap &m) { m.clear(); }
void w2cMapErase(W2cMap &m, W2cMap::iterator a, W2cMap::iterator b) { m.erase(a, b); }

// the registry object (0x1c B): the name map and the symbol vector (3 of them, built by eh_vector_constructor_iterator in TFB_ResolveSymbol)
#include "script/w2cregistry.h"
// 0x4342a0
// 0x434300
void w2cRegCtor(W2cRegistry *r) { new (r) W2cRegistry; }
void w2cRegDtor(W2cRegistry *r) { r->~W2cRegistry(); }
