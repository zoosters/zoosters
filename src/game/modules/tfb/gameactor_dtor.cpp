// Stand-in classes with raw offsets (own TU).
#include "modules/tfb/gameactor.h"
#include "framework/toolkits/world/clumphelper.h"
#ifdef ZOOSTERS_DEBUGUI
void port_debugui_actor_life(const void *actor, int alive);   // port/debugui/debugui_panels.cpp
#endif

namespace RWS { namespace TFB {
// GLOBAL: 0x42da00 RWS::TFB::GameActor::~GameActor
GameActor::~GameActor()
{
#ifdef ZOOSTERS_DEBUGUI
	port_debugui_actor_life(this, 0);
#endif
	if (m_clump)
		RWS::ClumpHelper::Destroy(m_clump);
}
} }
