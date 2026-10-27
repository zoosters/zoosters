#ifndef ZOOSTERS_TFB_SPRITES_GAMESPRITE_H
#define ZOOSTERS_TFB_SPRITES_GAMESPRITE_H

#include "script/types/scriptsprite.h"
#include "script/referenceset.h"
#include "tfb/hud/hud2d.h"
#include "modules/tfb/sprites/spriteextra.h"

// GLOBAL: 0x5cbf50 RWS::TFB::GameSprite::vftable
namespace RWS { namespace TFB {

class SpriteObject;

class SpriteManager;

class GameSprite : public Script::ScriptSprite, public Hud2DObject {
public:
	GameSprite(SpriteObject *owner);   // made by SpriteObject
	GameSprite(const GameSprite *source);   // copy for Spawn: the looks, not the links
	GameSprite(const char *name, const char *text);   // made by the scripts (SpriteManager::CreateTextSprite)
	~GameSprite();

	// Overrides of the Placement slots
	virtual int AdjustRefCount(int delta);                             // slot 2, the sprite deletes itself at 0 references
	virtual void SetVector(const Vec3 *v, Script::ScriptReference *owner);   // slot 18, ignores this: stores into owner
	virtual float GetFacing();                                         // slot 19
	virtual bool SetFacing(float v, int flag);                         // slot 20
	virtual float GetTilt();                                           // slot 21
	virtual bool SetTilt(float v, Script::ScriptReference *owner);     // slot 22
	virtual float GetRoll();                                           // slot 23
	virtual bool SetRoll(float v, Script::ScriptReference *owner);     // slot 24
	virtual void Remove();                                             // slot 27
	virtual void Reset();                                              // slot 28
	virtual char Control(bool);                                        // slot 29
	virtual intptr_t Spawn(intptr_t position, int);                              // slot 30, position is a Vec3 * (PTR32)
	virtual Script::ScriptReference *GetSpawnSource();                 // slot 31
	virtual int IsSolidCollider();                                     // slot 32, false; declared to keep the slot order
	virtual int IsMeshCollider();                                      // slot 33
	virtual int IsMeshCollidee();                                      // slot 34
	virtual bool PlayAnimation(int index);                             // slot 35
	virtual int AccessBehavior(int index, int value);                  // slot 36, byte table of the model state; -1 = count

	void SetFont(const char *name);
	void SetParent(GameSprite *parent);   // swap m_parent (counted in m_childRefs, released through AdjustRefCount)
	void SetModel(CTFBModel *model);
	float ProjectToScreen(RwCamera *camera);
	void UpdateExtents(char reset);
	char Resolve(RwCamera *camera, char resolveOnly);

	int m_refCount;                 // +0xdc references (AdjustRefCount); 0 = free
	const char *m_name;             // +0xe0 script object name (FindSpriteByName compares it)
	SpriteObject *m_owner;          // +0xe4 the SpriteObject behaviour that made it (0 for a script-made sprite)
	GameSprite *m_spawnSource;      // +0xe8 the original of a spawned clone; this for an original; 0 once removed
	char *m_textBuf;                // +0xec text storage (m_format points here)
	unsigned char m_alwaysVisible;  // +0xf0 set: Resolve forces the sprite visible
	char m_padf1;
	Script::PackedShorts m_scalePct;   // +0xf2 ExposeScales in percent (x, y)
	unsigned char m_visible;        // +0xf6 ExposeVisible (extents read 0 while clear); a byte, not a bool: the cursor stores a copy of a byte flag
	char m_padf7;
	SpriteExtra *m_extra;           // +0xf8 model state of a type 3 sprite
	unsigned char m_scaleByDistance;   // +0xfc set: the resolved scale is multiplied by the distance scale
	char m_padfd[3];
	float m_fixedWidth;             // +0x100 fixed box size (attribute commands 13 / 14; default 32)
	float m_fixedHeight;            // +0x104
	int m_priorityKey;              // +0x108 signed priority = the key in the active set (< 0: hidden)
	const char *m_parentName;       // +0x10c name of the parent sprite (attribute command 21)
	unsigned m_inheritFlags;        // +0x110 what the sprite takes from its parent: 1 position (+ anchor offset), 2 scale, 4 colour, 8 visibility, 0x10 rotation, 0x20 priority (added), 0x40 alpha
	int m_parentAnchor;             // +0x114 anchor kind of the parent
	int m_resolveGeneration;        // +0x118 generation stamp of the last resolve (g_spriteGeneration)
	GameSprite *m_renderNext;       // +0x11c next in the manager's render list (-1: not listed)
	GameSprite *m_parent;           // +0x120
	int m_childRefs;                // +0x124 sprites whose m_parent is this one
};
ZOOSTERS_ASSERT_SIZE(GameSprite, 0x128);
ZOOSTERS_ASSERT_OFFSET(GameSprite, m_type, 0x4c);
ZOOSTERS_ASSERT_OFFSET(GameSprite, m_dataValue, 0x50);
ZOOSTERS_ASSERT_OFFSET(GameSprite, m_refCount, 0xdc);
ZOOSTERS_ASSERT_OFFSET(GameSprite, m_scalePct, 0xf2);
ZOOSTERS_ASSERT_OFFSET(GameSprite, m_visible, 0xf6);
ZOOSTERS_ASSERT_OFFSET(GameSprite, m_extra, 0xf8);
ZOOSTERS_ASSERT_OFFSET(GameSprite, m_fixedWidth, 0x100);
ZOOSTERS_ASSERT_OFFSET(GameSprite, m_priorityKey, 0x108);
ZOOSTERS_ASSERT_OFFSET(GameSprite, m_parent, 0x120);

}}

void StopVoicesForObject(Script::ScriptReference *obj, int mode);

#endif
