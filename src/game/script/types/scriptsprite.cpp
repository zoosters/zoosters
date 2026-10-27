// Script::ScriptSprite: the member accessors of the sprite script object (Game.exe 0x413ae0-0x416800): names, getters and setters
// of the Expose* members, the ExposeClones set, and the vtable emitters for their classes. The sprite is the `owner` argument.
#include "script/membertable.h"
#include <float.h>
#include "modules/tfb/sprites/spritemanager.h"
#include "zoosters/memberrow.h"

using RWS::TFB::GameSprite;

static inline GameSprite *Sprite(Script::ScriptReference *owner)
{
	return static_cast<GameSprite *>(owner);
}

namespace Script {

// GLOBAL: 0x617e80 g_spriteManager
// GLOBAL: 0x617e84 g_spriteGeneration
// GLOBAL: 0x5cc098 Script::ScriptSprite::ExposeClones::vftable
// 0x416540
// (vehicle for the compiler-made vtable and scalar deleting destructor of ExposeClones: its constructor is inlined in the game)
ScriptReference *g_makeExposeClones() { return new ScriptSprite::ExposeClones; }

// 0x4164a0
ScriptSprite::ExposeClones::~ExposeClones() {}

// GLOBAL: 0x5cc3c0 Script::ScriptSprite::ExposePriority::vftable
// 0x416560
ScriptSprite::ExposePriority::ExposePriority() {}

// GLOBAL: 0x5cc410 Script::ScriptSprite::ExposePixelExtents::vftable
// 0x416590
ScriptSprite::ExposePixelExtents::ExposePixelExtents() {}


// 0x415c80
const char *ScriptSprite::GetName()
{
	return "sprite";
}

// 0x416420
const char *ScriptSprite::ExposeVisible::GetName()
{
	return "visible";
}

// 0x416430
const char *ScriptSprite::ExposeData::GetName()
{
	return "data";
}

// 0x416440
const char *ScriptSprite::ExposeLocation::GetName()
{
	return "location";
}

// 0x416450
const char *ScriptSprite::ExposeRotation::GetName()
{
	return "rotation";
}

// 0x416460
const char *ScriptSprite::ExposeJustification::GetName()
{
	return "justification";
}

// 0x416470
const char *ScriptSprite::ExposeTint::GetName()
{
	return "tint";
}

// 0x416480
const char *ScriptSprite::ExposeClones::GetName()
{
	return "clones";
}

// 0x416570
const char *ScriptSprite::ExposePriority::GetName()
{
	return "priority";
}

// 0x4165a0
const char *ScriptSprite::ExposePixelExtents::GetName()
{
	return "pixel extents";
}

// 0x4167e0
const char *ScriptSprite::ExposeScales::ExposeX::GetName()
{
	return "Horz";
}

// 0x4167f0
const char *ScriptSprite::ExposeScales::ExposeY::GetName()
{
	return "Vert";
}


// 0x413ae0
int ScriptSprite::ExposePixelExtents::ExposeX::GetInt(ScriptReference *owner)
{
	if (Sprite(owner)->m_visible)
		return (int)Sprite(owner)->m_width;
	return 0;
}

// 0x413b10
int ScriptSprite::ExposePixelExtents::ExposeY::GetInt(ScriptReference *owner)
{
	if (Sprite(owner)->m_visible)
		return (int)Sprite(owner)->m_height;
	return 0;
}

// 0x413b40
int ScriptSprite::ExposePixelExtents::GetInt(ScriptReference *owner)
{
	if (Sprite(owner)->m_visible) {
		PackedShorts r;
		r.x = (short)(int)Sprite(owner)->m_width;
		r.y = (short)(int)Sprite(owner)->m_height;
		return *(int *)&r;
	}
	return 0;
}

// 0x413b80
Vec3 *ScriptSprite::ExposeLocation::GetVector(ScriptReference *owner)
{
	return LocationVector(owner);
}

Vec3 *ScriptSprite::ExposeLocation::LocationVector(ScriptReference *owner)
{
	return (Vec3 *)&Sprite(owner)->m_posX;   // m_posX, m_posY, m_posZ are one vector
}

// 0x413b90
int ScriptSprite::ExposeLocation::GetInt(ScriptReference *owner)
{
	PackedShorts r;
	r.x = (short)(int)Sprite(owner)->m_posX;
	r.y = (short)(int)Sprite(owner)->m_posZ;
	return *(int *)&r;
}

// 0x413bc0
int ScriptSprite::ExposeLocation::ExposeX::GetInt(ScriptReference *owner)
{
	return (int)Sprite(owner)->m_posX;
}

// 0x413bd0
int ScriptSprite::ExposeLocation::ExposeY::GetInt(ScriptReference *owner)
{
	return (int)Sprite(owner)->m_posZ;
}

// 0x413be0
int ScriptSprite::ExposeScales::GetInt(ScriptReference *owner)
{
	return *(int *)&Sprite(owner)->m_scalePct;
}

// 0x413bf0
int ScriptSprite::ExposeScales::ExposeX::GetInt(ScriptReference *owner)
{
	return Sprite(owner)->m_scalePct.x;
}

// 0x413c00
int ScriptSprite::ExposeScales::ExposeY::GetInt(ScriptReference *owner)
{
	return Sprite(owner)->m_scalePct.y;
}

// 0x413c10
float ScriptSprite::ExposeRotation::GetFloat(ScriptReference *owner)
{
	return Sprite(owner)->m_rotation;
}

// 0x413c20
int ScriptSprite::ExposeTint::GetInt(ScriptReference *owner)
{
	return Sprite(owner)->m_tintPacked;
}

// 0x413c30
int ScriptSprite::ExposeTint::ExposeR::GetInt(ScriptReference *owner)
{
	return Sprite(owner)->m_tint.red;
}

// 0x413c40
int ScriptSprite::ExposeTint::ExposeG::GetInt(ScriptReference *owner)
{
	return Sprite(owner)->m_tint.green;
}

// 0x413c50
int ScriptSprite::ExposeTint::ExposeB::GetInt(ScriptReference *owner)
{
	return Sprite(owner)->m_tint.blue;
}

// 0x413c60
int ScriptSprite::ExposeTint::ExposeA::GetInt(ScriptReference *owner)
{
	return Sprite(owner)->m_tint.alpha;
}

// 0x413c70
int ScriptSprite::ExposeJustification::GetInt(ScriptReference *owner)
{
	return Sprite(owner)->m_justification;
}

// 0x413c80
// Visible = in the active set (priority key >= 0), switched on, and (when it inherits visibility) not hidden by its parent's flag.
int ScriptSprite::ExposeVisible::GetInt(ScriptReference *owner)
{
	if (Sprite(owner)->m_priorityKey >= 0 && Sprite(owner)->m_visible && (!Sprite(owner)->m_parent || !(Sprite(owner)->m_inheritFlags & 8)))
		return 1;
	return 0;
}

// 0x416580
int ScriptSprite::ExposePriority::GetInt(ScriptReference *owner)
{
	return Sprite(owner)->m_priority;
}

// 0x416630
// (folded: the same code is GetFloat of every 16-bit sprite member, 8 vtables 0x5cc4e8..0x5cc6f0)
float ScriptSprite::ExposeLocation::ExposeX::GetFloat(ScriptReference *owner)
{
	return (float)(short)GetInt(owner);
}

// ---- setters (each marks the sprite dirty: InvalidateSprite re-resolves it and its children)

// 0x4147e0
void ScriptSprite::ExposeLocation::ExposeX::SetValue(int value, ScriptReference *owner)
{
	value = (short)value;
	Sprite(owner)->m_posX = (float)value;
	Sprite(owner)->m_posY = FLT_MAX;   // FLT_MAX in y = a screen sprite at (x, z)
	InvalidateSprite(Sprite(owner));
}

// 0x414810
void ScriptSprite::ExposeLocation::ExposeY::SetValue(int value, ScriptReference *owner)
{
	value = (short)value;
	Sprite(owner)->m_posZ = (float)value;
	Sprite(owner)->m_posY = FLT_MAX;
	InvalidateSprite(Sprite(owner));
}

// 0x414890
void ScriptSprite::ExposeScales::ExposeX::SetValue(int value, ScriptReference *owner)
{
	Sprite(owner)->m_scalePct.x = (short)value;
	value = (short)value;
	Sprite(owner)->m_scaleX = (float)value * 0.01f;
	InvalidateSprite(Sprite(owner));
}

// 0x4148c0
void ScriptSprite::ExposeScales::ExposeY::SetValue(int value, ScriptReference *owner)
{
	Sprite(owner)->m_scalePct.y = (short)value;
	value = (short)value;
	Sprite(owner)->m_scaleY = (float)value * 0.01f;
	InvalidateSprite(Sprite(owner));
}

// 0x4148f0
void ScriptSprite::ExposeRotation::SetValue(float value, ScriptReference *owner)
{
	Sprite(owner)->m_rotation = value;
	InvalidateSprite(Sprite(owner));
}

// 0x414910
void ScriptSprite::ExposeTint::SetValue(int value, ScriptReference *owner)
{
	Sprite(owner)->m_tintPacked = value;
	InvalidateSprite(Sprite(owner));
}

// 0x414930
void ScriptSprite::ExposeTint::ExposeR::SetValue(int value, ScriptReference *owner)
{
	Sprite(owner)->m_tint.red = (unsigned char)value;
	InvalidateSprite(Sprite(owner));
}

// 0x414950
void ScriptSprite::ExposeTint::ExposeG::SetValue(int value, ScriptReference *owner)
{
	Sprite(owner)->m_tint.green = (unsigned char)value;
	InvalidateSprite(Sprite(owner));
}

// 0x414970
void ScriptSprite::ExposeTint::ExposeB::SetValue(int value, ScriptReference *owner)
{
	Sprite(owner)->m_tint.blue = (unsigned char)value;
	InvalidateSprite(Sprite(owner));
}

// 0x414990
void ScriptSprite::ExposeTint::ExposeA::SetValue(int value, ScriptReference *owner)
{
	Sprite(owner)->m_tint.alpha = (unsigned char)value;
	InvalidateSprite(Sprite(owner));
}

// 0x4149b0
void ScriptSprite::ExposeJustification::SetValue(int value, ScriptReference *owner)
{
	Sprite(owner)->m_justification = (unsigned char)value;
	Sprite(owner)->m_anchor = value;   // the Hud2DObject keeps the anchor as an int
	InvalidateSprite(Sprite(owner));
}

// 0x414a30
void ScriptSprite::ExposeData::SetValue(int value, ScriptReference *owner)
{
	Sprite(owner)->m_dataValue = value;
	unsigned char type = Sprite(owner)->m_type;
	if (type == 1 || type == 2)   // only a text sprite shows the value
		InvalidateSprite(Sprite(owner));
}

// 0x414a50
void ScriptSprite::ExposePriority::SetValue(int value, ScriptReference *owner)
{
	if (value != Sprite(owner)->m_priority) {
		Sprite(owner)->m_priority = (unsigned char)value;
		if (g_spriteManager)
			g_spriteManager->MarkRenderListDirty();   // the render list is sorted by priority
	}
}

// Setters that take their value as two packed shorts.

// 0x4147a0
void ScriptSprite::ExposeLocation::SetInt(PackedShorts value, ScriptReference *owner)
{
	Sprite(owner)->m_posY = FLT_MAX;
	Sprite(owner)->m_posX = (float)value.x;
	Sprite(owner)->m_posZ = (float)value.y;
	InvalidateSprite(Sprite(owner));
}

// DROPPED 0x414840 Script::ScriptSprite::ExposeScales::SetInt (the game spills value.x (movsx cx) into the owner arg slot BEFORE
// the m_scalePct store and reloads y from memory; ours schedules the movsx after the store; 8 source variants all give the same code)
// 0x414840 Script::ScriptSprite::ExposeScales::SetInt: x = movsx of low half, y re-read as movsx word [+F4] after the 32-bit store at +F2, *0.01f via fild of int, stores +7C/+80, InvalidateSprite equal; only scheduling of movsx/spill differs (a); no (b)
void ScriptSprite::ExposeScales::SetInt(PackedShorts value, ScriptReference *owner)
{
	int x = value.x;
	Sprite(owner)->m_scalePct = value;
	int y = Sprite(owner)->m_scalePct.y;
	Sprite(owner)->m_scaleX = (float)x * 0.01f;
	Sprite(owner)->m_scaleY = (float)y * 0.01f;
	InvalidateSprite(Sprite(owner));
}

static inline void SetDirty()
{
	if (g_spriteManager)
		g_spriteManager->MarkRenderListDirty();
}

// DROPPED 0x4149d0 Script::ScriptSprite::ExposeVisible::SetInt (instruction-identical, but the game has a 6-byte `lea ebx,[ebx+0]`
// alignment pad before the shared `ret 8`; same residual as GameSprite::Remove/Reset)
// 0x4149d0 Script::ScriptSprite::ExposeVisible::SetInt
void ScriptSprite::ExposeVisible::SetInt(int value, ScriptReference *owner)
{
	bool v = value != 0;
	if (v != Sprite(owner)->m_visible) {
		Sprite(owner)->m_visible = v;
		if (v) {
			if (Sprite(owner)->m_renderNext == (GameSprite *)-1) {   // not listed yet: size it, then rebuild the list
				Sprite(owner)->UpdateExtents(1);
				SetDirty();
			}
		} else if (Sprite(owner)->m_renderNext != (GameSprite *)-1) {
			SetDirty();
		}
	}
}

}  // namespace Script

namespace Script {
// GLOBAL: 0x5cc5e0 Script::g_vtPixX
extern char g_vtPixX[];
// GLOBAL: 0x5cc598 Script::g_vtPixY
extern char g_vtPixY[];

// 0x416670
void *ScriptSprite::ExposePixelExtents::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptSprite::ExposePixelExtents::ExposeX, ScriptSprite::ExposePixelExtents::ExposeY> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5cc670 Script::g_vtLocX
extern char g_vtLocX[];
// GLOBAL: 0x5cc628 Script::g_vtLocY
extern char g_vtLocY[];

// 0x416700
void *ScriptSprite::ExposeLocation::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptSprite::ExposeLocation::ExposeX, ScriptSprite::ExposeLocation::ExposeY> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5cc700 Script::g_vtScaX
extern char g_vtScaX[];
// GLOBAL: 0x5cc6b8 Script::g_vtScaY
extern char g_vtScaY[];

// 0x416770
void *ScriptSprite::ExposeScales::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptSprite::ExposeScales::ExposeX, ScriptSprite::ExposeScales::ExposeY> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5cc758 Script::g_vtTintA
extern char g_vtTintA[];
// GLOBAL: 0x5cc7a0 Script::g_vtTintB
extern char g_vtTintB[];
// GLOBAL: 0x5cc7e8 Script::g_vtTintG
extern char g_vtTintG[];
// GLOBAL: 0x5cc830 Script::g_vtTintR
extern char g_vtTintR[];

// 0x416800
void *ScriptSprite::ExposeTint::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptSprite::ExposeTint::ExposeR, ScriptSprite::ExposeTint::ExposeG, ScriptSprite::ExposeTint::ExposeB, ScriptSprite::ExposeTint::ExposeA> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

// The game folds these bodies into one function each (shared with other classes' virtuals), so scriptsprite.h had no definition for them;
// each override gets its own one-line body.
Script::ScriptReference *thunk_FUN_0043e330();   // 0x416490 (script/protothunks.cpp): the sprite type getter

// GetBaseType (slot 5) is the game's 0x416490 itself (the sprite type getter): a linker alias, as for ScriptSprite::GetType.
#if defined(_MSC_VER)
#endif
Script::ScriptReference *Script::ScriptSprite::ExposeClones::Deref(Script::ScriptReference *owner, int flags) { return &static_cast<Script::Placement *>(owner)->m_20; }


int Script::ScriptSprite::ExposePriority::GetMax() { return 255; }

// the game's SetInt members are slot 16 (SetValue(int, owner)); the packed pair travels as the int.
// FOLDED: 0x4149d0 Script::ScriptSprite::ExposeVisible::SetValue
void Script::ScriptSprite::ExposeVisible::SetValue(int value, Script::ScriptReference *owner) { SetInt(value, owner); }
// FOLDED: 0x4147a0 Script::ScriptSprite::ExposeLocation::SetValue
void Script::ScriptSprite::ExposeLocation::SetValue(int value, Script::ScriptReference *owner)
{
	Script::PackedShorts p;
	memcpy(&p, &value, sizeof p);
	SetInt(p, owner);
}
// FOLDED: 0x414840 Script::ScriptSprite::ExposeScales::SetValue
void Script::ScriptSprite::ExposeScales::SetValue(int value, Script::ScriptReference *owner)
{
	Script::PackedShorts p;
	memcpy(&p, &value, sizeof p);
	SetInt(p, owner);
}
// FOLDED: 0x41ce40 Script::ScriptSprite::ExposeData::GetInt (the game's ScriptProp::ExposeAnimationRate::GetInt: the field at the same offset)
int Script::ScriptSprite::ExposeData::GetInt(Script::ScriptReference *owner) { return Sprite(owner)->m_dataValue; }
