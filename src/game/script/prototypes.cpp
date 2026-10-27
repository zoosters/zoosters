// Type prototype getters of the Placement-based script types (light, rumble, prop, particle, sprite).
#include "script/types/scriptlight.h"
#include "script/types/scriptparticle.h"
#include "script/types/scriptsprite.h"
#include "script/types/scriptcontroller.h"
#include "script/types/scriptprop.h"

using namespace Script;

// 0x43c490
ScriptReference *TFBType_Light()
{
	static ScriptLight s;
	return &s;
}

// 0x439770
ScriptReference *TFBType_Rumble()
{
	static RumbleProto s;
	return &s;
}

// 0x43fc00
ScriptReference *TFBType_Prop()
{
	static PropProto s(PROTOTYPE);
	return &s;
}

// 0x43b4f0
ScriptReference *TFBType_Particle()
{
	static ScriptParticle s(PROTOTYPE);
	ZOOSTERS_PROTO_REGISTER_AFTER(s.ScriptParticle::vf10());
	return &s;
}

// 0x43e330
ScriptReference *TFBType_Sprite()
{
	static ScriptSprite s(PROTOTYPE);
	ZOOSTERS_PROTO_REGISTER_AFTER(s.ScriptSprite::vf10());
	return &s;
}
