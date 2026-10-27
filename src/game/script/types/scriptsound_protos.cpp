// sound prototype getters (0x43e120, 0x43db70) and the member registration helper (0x43e1b0).
#include "script/script.h"
#include "script/prototype_virtuals.h"
#include "script/types/scriptsound.h"
#include "script/script.h"

namespace Script {

class SoundProtoW : public ScriptSoundObj {
public:
	SoundProtoW() {}
	int GetKind();                       // 6: 0x43e6f0 (returns 0)
	void *GetMembers(int *count);        // 7: 0x43dc80 (the default: the type prototype has no members; ScriptSoundObj's 0x43e0a0 is the object's)
};

class SoundObjProtoW : public ScriptSoundObj {
public:
	inline SoundObjProtoW();
};

struct SoundRegHelper {
	SoundRegHelper() {}
	void Run();                                 // 0x43e1b0
};

__forceinline SoundObjProtoW::SoundObjProtoW()
{
}

}

using namespace Script;

ScriptReference *TFBType_Sound();    // 0x43db70
ScriptReference *ScriptSound_AllSetProto();    // 0x43e120
ScriptReference *TFBMember_Sound_StreamCount();    // 0x43e170
ScriptReference *OpProto_MediaPlay();    // 0x43dc00
ScriptReference *OpProto_MediaStop();    // 0x43dc40
int ScriptSound_GetEnvironmentType();                 // 0x43dfc0
int ScriptSound_GetEnvironmentLevel();                 // 0x43e030

// 0x43e120
ScriptReference *ScriptSound_AllSetProto()
{
	static SoundProtoW s;
	return &s;
}

// 0x43db70
ScriptReference *TFBType_Sound()
{
	static SoundObjProtoW s;
	ZOOSTERS_PROTO_REGISTER_AFTER(SoundRegHelper().Run());
	return (ScriptReference *)&s;
}

// 0x43e1b0
void SoundRegHelper::Run()
{
	((ScriptReference *)ScriptSound_AllSetProto())->ResolveSymbol(TFBType_Sound(), " ALL ", 0);
	((ScriptReference *)TFBMember_Sound_StreamCount())->ResolveSymbol(TFBType_Sound(), 0, 0);
	ScriptSound_GetEnvironmentType();
	ScriptSound_GetEnvironmentLevel();
	((ScriptReference *)OpProto_MediaPlay())->ResolveSymbol(TFBType_Sound(), 0, 0);
	((ScriptReference *)OpProto_MediaStop())->ResolveSymbol(TFBType_Sound(), 0, 0);
}

// FOLDED: 0x43e6f0 Script::SoundProtoW::GetKind
int Script::SoundProtoW::GetKind() { return 0; }
// FOLDED: 0x43dc80 Script::SoundProtoW::GetMembers
void *Script::SoundProtoW::GetMembers(int *count) { *count = 0; return 0; }   // CAttributeHandler::GetAttrPostUpdateFunc
