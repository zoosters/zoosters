#include "script/script.h"

// 0x43b3d0: ~ScriptReference (the out-of-line copy: stores the ScriptReference vtable; NamedObject has no destructor code)
namespace Script {
// NOVTABLE: Script::RootRefDtor
// GLOBAL: 0x5ca3ec Script::RootRefDtor::vftable
class RootRefDtor : public ScriptReference {
public:
	virtual ~RootRefDtor();
};

// 0x43b3d0
RootRefDtor::~RootRefDtor()
{
}
}

// 0x43baa0: drops one reference of the script type registry (0x43c870 counts them up)
// GLOBAL: 0x621f28 g_typeInitCount
extern int g_typeInitCount;

namespace Script {
// 0x43baa0
void TypeRegistry_Release()
{
	g_typeInitCount--;
}
}

// 0x43e640: ScriptSystem and friends, virtual slot 6 (returns -1)
namespace Script {
class SysFamilySlot6 {
public:
	virtual int GetKind();
};
}

// 0x43e640
int Script::SysFamilySlot6::GetKind()
{
	return -1;
}
