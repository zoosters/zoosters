// Makes cl emit the vtables and scalar deleting destructors of CAttributeHandler and CEventHandler, via their implicit constructors.
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
namespace RWS {

// GLOBAL: 0x550170 ??2@YAPAXI@Z
// GLOBAL: 0x5ca2e8 RWS::CAttributeHandler::vftable
// 0x401020
CAttributeHandler *g_makeAttributeHandler() { return new CAttributeHandler; }

// NOVTABLE: RWS::EventHandlerStandIn
class EventHandlerStandIn : public CEventHandler {
public:
	virtual void HandleEvents(CMsg &) {}
};

// GLOBAL: 0x5ca2f8 RWS::CEventHandler::vftable
// 0x401050
CEventHandler *g_makeEventHandler() { return new EventHandlerStandIn; }

}
