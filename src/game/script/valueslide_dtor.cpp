// ~ValueSlide 0x42f8a0 in its own TU with a local stand-in view of the class (the shared ValueSlide in
// script/ops_value.h has no destructor: giving it one changes every copy constructor / CloneToPool of the file group).
#include "script/script.h"
#include "script/referenceset.h"

namespace Script {
// GLOBAL: 0x5ca608 Script::ValueSlideCurve::vftable
// GLOBAL: 0x5ca3ec Script::ScriptReference::vftable
class ValueSlideCurve : public ReferenceSet {
public:
	ValueSlideCurve() {}
	ValueSlideCurve(const ValueSlideCurve &o);
	void Insert(void *obj, float key) { ReferenceSet::Insert((ScriptReference *)obj, key); }
};

// novtable: the game dtor 0x42f8a0 stores only ScriptReference::vftable (dis +0x4b); a vtable of its own (no game address) must not be stored
class __declspec(novtable) ValueSlideDV : public ScriptReference {
public:
	~ValueSlideDV();
	char m_data[0x28];
	ValueSlideCurve m_curve;    // +0x2c (ValueSlide::m_slides in script/ops_value.h)
};

// GLOBAL: 0x43cca0 ?Clear@ValueSlideCurve@Script@@QAEXXZ
// 0x42f8a0
ValueSlideDV::~ValueSlideDV()
{
}
}
