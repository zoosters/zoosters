// Scalar deleting destructors of classes whose layout is not declared yet: a stand-in with only a virtual destructor, NOVTABLE.
// The unannotated factory function makes cl emit the vtable and the compiler-made destructor. Once the class has its real
// declaration, move the address line there (the name must be the emitted class's).
#include "script/script.h"

// folded copy shared by the two Script classes with vtables 0x5ca608 / 0x5ca710 (~X = 0x402b20 stores 0x5ca608, clears a ReferenceSet).
// NOVTABLE: Script::ReferenceSetDeleter
namespace Script {
class ReferenceSetDeleter : public ScriptReference {
public:
	~ReferenceSetDeleter();
};
// 0x402be0
// GLOBAL: 0x402b20 ??1ReferenceSetDeleter@Script@@UAE@XZ
ScriptReference *g_makeRefSetOwner() { return new ReferenceSetDeleter; }

// vtable 0x5cd1a8: real class Script::ClumpCloneRef in g/corpus.cpp.
}

namespace RWS {
// vtable 0x5cd16c: real class RWS::MainTickHandler in g/corpus.cpp.

// vtable 0x5cb348 CPB_TexPart: real class in rws/particles*.cpp (task fx-particles; ~CPB_TexPart 0x44caa0, deleting dtor 0x40a320).
}

// FOLDED: 0x402b20 Script::ReferenceSetDeleter::~ReferenceSetDeleter
Script::ReferenceSetDeleter::~ReferenceSetDeleter() {}   // the game folded it into Script::ReferenceSet::~ReferenceSet
