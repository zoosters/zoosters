// Singleton: constructing it loads the control settings and the DirectInput manager; destroying it frees both.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_CONTROLSMANAGER_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_CONTROLSMANAGER_H

#include "script/script.h"
#include "framework/core/memoryhandler.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"

class ControlsManager : public RWS::CEventHandler, public RWS::LinearAllocationPolicy {
public:
	ControlsManager();
	~ControlsManager();
	virtual void HandleEvents(RWS::CMsg &msg) {}        // game 0x410380, a shared empty function
	static void *operator new(size_t size, RwsMemHint type);   // MemAllocHint(size, type)
	static void operator delete(void *p, RwsMemHint) {}
	static void operator delete(void *p) { RWS::LinearAllocationPolicy::operator delete(p); }
	void Poll();
};
ZOOSTERS_ASSERT_SIZE(ControlsManager, 8);

ControlsManager *GetControlsManager();
extern ControlsManager *g_controlsManager;           

#endif
