#ifndef ZOOSTERS_MODULES_FX_GAMELIGHT_H
#define ZOOSTERS_MODULES_FX_GAMELIGHT_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "script/actor.h"
#include "script/types/scriptlight.h"

namespace RWS { namespace FX {

class CFXColorLight;

// The game's light is Placement-shaped but its constructor is not Placement's: the inline Placement constructor stores the
// +0x3c word before the vptr under cl, the game after it. Hence this stand-in base.
// GLOBAL: 0x5ca710 RWS::FX::LightOwnerSet::vftable
class LightOwnerSet : public Script::ReferenceSet {
public:
	LightOwnerSet() {}
	void *m_owner;
};
ZOOSTERS_ASSERT_SIZE(LightOwnerSet, 0x1c);

// The real light is a Script::ScriptLight; the stand-in above only steers cl 13.10's constructor codegen.
class LightBase : public Script::ScriptLight {
public:
	LightBase() {}
};
ZOOSTERS_ASSERT_SIZE(LightBase, 0x40);

// GLOBAL: 0x5cb1c0 RWS::FX::GameLight::vftable
// Slots 17-24 (position, facing, tilt) are called through the game's vtable and are not declared virtual here.
class GameLight : public LightBase {
public:
	// The game's caller sees the constructor as nothrow (no unwind funclet for the new); the definition is compiled as potentially throwing.
#ifdef ZOOSTERS_GAMELIGHT_DEFINITION
	explicit GameLight(CFXColorLight *owner);
#else
	explicit GameLight(CFXColorLight *owner) throw();
#endif
	// Overrides of the game vtable (the other slots are ScriptLight's / Placement's)
	int AdjustRefCount(int delta);                                // slot 2, deletes the light when the count reaches 0
	void *GetMembers(int *count);                                 // slot 7, the colour member
	Vec3 *GetVector(Script::ScriptReference *owner);              // slot 17, position
	void SetVector(const Vec3 *v, Script::ScriptReference *owner);   // slot 18, position
	float GetFacing();                                            // slot 19
	bool SetFacing(float v, int flag);                            // slot 20
	float GetTilt();                                              // slot 21
	bool SetTilt(float v, Script::ScriptReference *owner);        // slot 22
	void ApplyColor();

	Script::SetCursorTail m_unk40;       // +0x40 unused
	float m_facing;             // +0x4c yaw, degrees
	float m_tilt;               // +0x50 pitch, degrees
	union {
		int32_t m_color;            // +0x54 the colour as the script sees it
		uint8_t m_colorBytes[4];    // r, g, b, a
	};
	int m_refCount;             // +0x58
	CFXColorLight *m_colorLight;   // +0x5c the owning light (0 once it is destroyed)
};
ZOOSTERS_ASSERT_SIZE(GameLight, 0x60);
ZOOSTERS_ASSERT_OFFSET(GameLight, m_unk40, 0x40);
ZOOSTERS_ASSERT_OFFSET(GameLight, m_facing, 0x4c);
ZOOSTERS_ASSERT_OFFSET(GameLight, m_tilt, 0x50);
ZOOSTERS_ASSERT_OFFSET(GameLight, m_color, 0x54);
ZOOSTERS_ASSERT_OFFSET(GameLight, m_refCount, 0x58);
ZOOSTERS_ASSERT_OFFSET(GameLight, m_colorLight, 0x5c);

}}
#endif
