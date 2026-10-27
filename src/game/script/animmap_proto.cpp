// AnimMapMeasurement prototype getter (function-local static whose constructor registers the object with TFB_ResolveSymbol, using the actor type)
// and the ExposeAnimIndex name lookup.
#include "script/script.h"

#include "script/runtime.h"
#include "script/types/animmap.h"

namespace Script {
// GLOBAL: 0x43b3d0 ??1AnimMapProto@Script@@UAE@XZ
// GLOBAL: 0x5d02f8 Script::AnimMapProto::vftable
class AnimMapProto : public AnimMapMeasurement {
public:
	__forceinline AnimMapProto() { }
	static void Register() { ((ScriptReference *)::TFBType_AnimMap())->ResolveSymbol(::TFBType_Actor(), 0, 0); }
};
}

using namespace Script;

// 0x4384c0
typedef Script::AnimMapMeasurement *AnimMapTypePtr;
AnimMapTypePtr TFBType_AnimMap()
{
	static AnimMapProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(AnimMapProto::Register());
	return (AnimMapTypePtr)&s;
}

// AnimMapMeasurement's overrides in the game's vtable 0x5d02f8 (shared bodies, ICF)
#include "script/folded.h"
// FOLDED: 0x436a50 Script::AnimMapMeasurement::GetBaseType
ScriptReference *Script::AnimMapMeasurement::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::AnimMapMeasurement::GetKind
int Script::AnimMapMeasurement::GetKind() { return 0; }
// FOLDED: 0x4386a0 Script::AnimMapMeasurement::GetMax
int Script::AnimMapMeasurement::GetMax() { return AnimMapMemberCount(); }
// FOLDED: 0x43e6f0 Script::AnimMapMeasurement::GetValueType
int Script::AnimMapMeasurement::GetValueType() { return 0; }
// FOLDED: 0x438630 Script::AnimMapMeasurement::SetValue
void Script::AnimMapMeasurement::SetValue(int value, ScriptReference *owner) { ((Script::ExposeAnimIndex *)this)->Script::ExposeAnimIndex::SetValue(value, owner); }   // cross-class fold: ExposeAnimIndex::SetValue's body (walks the member table from `this`)
