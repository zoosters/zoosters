// Prototype getters (static T s; return &s;) of ops_value.h classes whose default constructor stores m_flags = 1.
// One TU per op header: they conflict with each other. ZOOSTERS_REAL_DTOR gives the atexit of the destructor stub.
#include "script/ops_value.h"
using namespace Script;

// GLOBAL: 0x5cdcf0 Script::ValueInc::vftable
// 0x42ff50
ScriptReference *OpProto_ValueInc()
{
	static ValueInc s;
	return &s;
}

// GLOBAL: 0x5cdd38 Script::ValueDec::vftable
// 0x42ff90
ScriptReference *OpProto_ValueDec()
{
	static ValueDec s;
	return &s;
}

// GLOBAL: 0x5cdd80 Script::ValueSet::vftable
// 0x42ffd0
ScriptReference *OpProto_ValueSet()
{
	static ValueSet s;
	return &s;
}
