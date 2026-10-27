#include "script/value.h"
using namespace Script;

struct ValueOpRegistrar {
	void Register();   // 0x430c00 (RegisterValueOps)
};
// GLOBAL: 0x430c00 ?Register@ValueOpRegistrar@@QAEXXZ

// Near miss: the game's guarded block has one unwind state and an uninitialised 1-byte local; ours adds a second state (~ScriptReference of the base) and zero-inits it.
// Idea: a base class without a real destructor whose vftable still has ~ScriptReference in slot 3.
// GLOBAL: 0x5ca5c8 ValueTypeProto::vftable
class ValueTypeProto : public ScriptReference {
public:
	const char *GetName();       // 0: 0x402b00
	ScriptReference *GetType();  // 4: 0x402b70
	virtual int GetMax();        // 11: default 0x402b10
	virtual int GetValueType();  // 12
	__forceinline ValueTypeProto()
	{
	}
};

// GLOBAL: 0x61f518 ?s@?1??TFBType_Value@@YAPAVScriptReference@Script@@XZ@4VValueTypeProto@@A
// GLOBAL: 0x61f51c ?$S1@?1??TFBType_Value@@YAPAVScriptReference@Script@@XZ@4IA
// 0x42f900 TFBType_Value: guard test/set, vptr store before RegisterValueOps (0x430c00, which never reads its ecx; checked disassembly), atexit, return &s identical; diffs are the registrar temp byte zero-init (game leaves it uninitialised, unread), EH state number 1 vs 0 and one extra unwind funclet; no behaviour difference
ScriptReference *TFBType_Value()
{
	static ValueTypeProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(ValueOpRegistrar().Register());   // RegisterValueOps calls TFBType_Value again (zoosters/base.h)
	return &s;
}

void RegisterValueOps();   // 0x430c00
inline void ValueOpRegistrar::Register() { RegisterValueOps(); }

#include "script/folded.h"
// FOLDED: 0x402b00 ValueTypeProto::GetName
char const * ValueTypeProto::GetName() { return ValueInterface_GetName(); }
// FOLDED: 0x402b10 ValueTypeProto::GetMax
int ValueTypeProto::GetMax() { return ValueInterface_GetMax(); }
// FOLDED: 0x402b70 ValueTypeProto::GetType
Script::ScriptReference * ValueTypeProto::GetType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 ValueTypeProto::GetValueType
int ValueTypeProto::GetValueType() { return 0; }
