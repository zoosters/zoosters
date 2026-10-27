// the second copy of the reference set destructor (0x43e2e0; identical to 0x402b20 but with its own EH tables).
#include "script/script.h"

namespace Script {
// NOVTABLE: Script::RefSetDtor43e
// GLOBAL: 0x43cca0 Script::RefSetDtor43e::Clear
class RefSetDtor43e : public ScriptReference {
public:
	~RefSetDtor43e();
	void Clear();   // 0x43cca0
};
// 0x43e2e0
RefSetDtor43e::~RefSetDtor43e()
{
	Clear();
}
}

#include "script/referenceset.h"
namespace Script { inline void RefSetDtor43e::Clear() { ((ReferenceSet *)this)->Clear(); } }
