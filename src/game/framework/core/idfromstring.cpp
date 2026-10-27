#include "framework/core/idfromstring.h"

namespace RWS {

// 0x553620
unsigned GenerateUIntIdFromString(const char *cptr)
{
	if (!cptr) return 0;
	char *ptr = const_cast<char *>(cptr);
	unsigned id = 0;
	while (*ptr != 0) {
		id = (id << 1) ^ (static_cast<unsigned>(*ptr));
		ptr++;
	}
	return id;
}

}  // namespace RWS
