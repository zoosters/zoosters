#include "modules/tfb/levelhub.h"
#include "tfb/level/level.h"

// GLOBAL: 0x617fe8 g_soundStreamPath
// GLOBAL: 0x617fec g_musicStreamPath
// GLOBAL: 0x602ea0 g_globalSymbolCount
bool MainLoop_IsStarted();

namespace RWS { namespace TFB {

// GLOBAL: 0x5ccad0 RWS::TFB::LevelHub::vftable
// 0x41a050
// 0x419840
LevelHub::LevelHub(const CAttributePacket &)
{
	m_initMode = MainLoop_IsStarted() ? INIT_NEVER : INIT_NEW_ONLY;
	g_musicStreamPath = 0;
	g_soundStreamPath = 0;
}

// 0x4198b0
LevelHub::~LevelHub()
{
	LevelDestroyWorlds(true);
}

// GLOBAL: 0x5ccae4 RWS::TFB::GlobalScriptNames::vftable
// 0x41a000
// 0x41a030
GlobalScriptNames::~GlobalScriptNames()
{
	g_globalSymbolCount = -1;
}

// (vehicle for the compiler-made vtable and scalar deleting destructor of GlobalScriptNames: its constructor is inlined in the game)
void g_useGlobalScriptNames(const CAttributePacket &attr)
{
	new GlobalScriptNames(attr);
}

}}

// 0x419820
const char *GetSoundStreamPath()
{
	return g_soundStreamPath;
}

// 0x419830
const char *GetMusicStreamPath()
{
	return g_musicStreamPath;
}
