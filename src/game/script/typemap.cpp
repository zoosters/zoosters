#include <map>
#include <string.h>

struct BsEntry;
struct bsless { // std::binary_function is gone in C++17
	typedef const char *first_argument_type;
	typedef const char *second_argument_type;
	typedef bool result_type;
	bool operator()(const char *x, const char *y) const { return strcmp(x, y) < 0; }
};
typedef std::map<const char *, BsEntry *, bsless> BsMap;

// 0x4335f0
void bsMapErase(BsMap &registry, BsMap::iterator where) { registry.erase(where); }
