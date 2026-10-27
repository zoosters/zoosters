// PS2 RTTI: LevelHub : CAttributeHandler@0, LinearAllocationPolicy@28; GlobalScriptNames : LevelHub@0 (the same behaviour class, created for
// the game-wide names:
// the script symbols that stay declared across levels).
#ifndef ZOOSTERS_TFB_LEVELHUB_H
#define ZOOSTERS_TFB_LEVELHUB_H

#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"

namespace Script {
// Script variable object as LevelHub command 0 sees it: slot 16 of its vtable (offset 0x40) takes (value, object); only that slot is used,
// the others are fillers
// (no constructor, so no vtable is emitted).
class ScriptVarP : public ScriptReference {
public:
	virtual void vf11();
	virtual void vf12();
	virtual void vf13();
	virtual void vf14();
	virtual void vf15();
	virtual void vf16(int value, ScriptVarP *self);
};
}

namespace RWS { namespace TFB {

// Single vtable 0x5ccad0 = [~ 0x41a050, HandleAttributes 0x419a70, GetAttributeTable and GetAttrPostUpdateFunc of the base, vf4
// 0x410380 (an empty function)]: five slots. Registered size 0x20 (RegisterFactory 0x5c5ac0).
class LevelHub : public CAttributeHandler, public LinearAllocationPolicy {
public:
	// What the hub does with the initial value of a declared integer variable. The constructor picks INIT_NEVER when the main loop already
	// runs (a hub created while a level
	// is loaded in-game) and INIT_NEW_ONLY otherwise; the first attribute packet leaves it at INIT_ALWAYS.
	enum InitMode { INIT_NEVER = -1, INIT_ALWAYS = 0, INIT_NEW_ONLY = 1 };

	RWS_MAKENEWCLASS(LevelHub);
	LevelHub(const CAttributePacket &attr);
	~LevelHub();
	virtual void HandleAttributes(const CAttributePacket &attr);
	virtual void vf4(const CAttributePacket &attr);                // slot 4, empty
	// Declares one typed script variable (attribute command 1: initial value, type id, name). Always returns true.
	bool DeclareVariable(const char *cmdData);

protected:
	int m_initMode;   // +0x1c InitMode (the size assert below pins it: the base is 0x1c bytes)
};
ZOOSTERS_ASSERT_SIZE(LevelHub, 0x20);

// LevelHub with its own vtable 0x5ccae4 (inline constructor) and destructor 0x41a030, which marks the global names as unloaded.
class GlobalScriptNames : public LevelHub {
public:
	RWS_MAKENEWCLASS(GlobalScriptNames);
	GlobalScriptNames(const CAttributePacket &attr) : LevelHub(attr) {}
	~GlobalScriptNames();
};
ZOOSTERS_ASSERT_SIZE(GlobalScriptNames, 0x20);

}}

// The registry size when the global names were loaded (symbols below it are game-wide): the limit the script reset and the level
// loader use. -1 = no global names loaded (GlobalScriptNames::~GlobalScriptNames); HandleAttributes command 3 only sets the stream
// paths while the global names are loaded.
extern int g_globalSymbolCount;
// The language-specific speech stream and the music stream of the level (GetSoundStreamPath / GetMusicStreamPath 0x419820 / 0x419830
// hand them to the audio stream player); 0 until command 3 of a LevelHub packet sets them.
extern const char *g_soundStreamPath;
extern const char *g_musicStreamPath;

#endif
