// prototype getters of the ScriptSystem family (math ops, shadow, fog, motion blur, memory card): the constructor registers
// the type with the global type list (0x439bc0) by calling the getter itself again. Standalone classes: script/prototype_virtuals.h.
#include "script/types/scriptmathops.h"
#include "script/types/scriptshadow.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptmotionblur.h"
#include "script/types/scriptmemcard.h"

void TFB_RegisterType(Script::ScriptReference *t);   // 0x439bc0 TFB_RegisterType

Script::ScriptReference *TFBType_MathOps();   // 0x43f610
Script::ScriptReference *TFBType_Shadow();   // 0x43f680
Script::ScriptReference *TFBType_Fog();   // 0x43f6f0
Script::ScriptReference *TFBType_MotionBlur();   // 0x43f760
Script::ScriptReference *TFBType_MemCard();   // 0x43f7d0

namespace Script {

class MathOpsProto : public ScriptMathOps {
public:
	MathOpsProto() { }
};

class ShadowProto : public ScriptShadow {
public:
	ShadowProto() { }
};

class FogProto : public ScriptFog {
public:
	FogProto() { }
};

class MotionBlurProto : public ScriptMotionBlur {
public:
	MotionBlurProto() { }
};

class MemCardProto : public ScriptMemCard {
public:
	MemCardProto() { }
};


}

using namespace Script;

// 0x43f610
Script::ScriptReference *TFBType_MathOps()
{
	static MathOpsProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(TFB_RegisterType(::TFBType_MathOps()));   // the constructor registered the type in the game
	return (Script::ScriptReference *)&s;
}

// 0x43f680
Script::ScriptReference *TFBType_Shadow()
{
	static ShadowProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(TFB_RegisterType(::TFBType_Shadow()));
	return (Script::ScriptReference *)&s;
}

// 0x43f6f0
Script::ScriptReference *TFBType_Fog()
{
	static FogProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(TFB_RegisterType(::TFBType_Fog()));
	return (Script::ScriptReference *)&s;
}

// 0x43f760
Script::ScriptReference *TFBType_MotionBlur()
{
	static MotionBlurProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(TFB_RegisterType(::TFBType_MotionBlur()));
	return (Script::ScriptReference *)&s;
}

// 0x43f7d0
Script::ScriptReference *TFBType_MemCard()
{
	static MemCardProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(TFB_RegisterType(::TFBType_MemCard()));
	return (Script::ScriptReference *)&s;
}

