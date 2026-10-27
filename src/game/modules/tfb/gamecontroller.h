#ifndef ZOOSTERS_MODULES_TFB_GAMECONTROLLER_H
#define ZOOSTERS_MODULES_TFB_GAMECONTROLLER_H
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
namespace RWS { namespace TFB {
// PS2 base order: CEventHandler@0, CAttributeHandler@8, LinearAllocationPolicy@36 (no CSystemCommands)
class GameController : public CEventHandler, public CAttributeHandler, public LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(GameController);
	GameController(const CAttributePacket &attr);
	~GameController();
	virtual void HandleEvents(CMsg &msg);
	virtual void HandleAttributes(const CAttributePacket &attr);
public:
	int m_blockMask;        // +0x24: inputs 0..5 ignored until released (set by 0x4028a0; starts at +0x24: without CSystemCommands no 4 byte pad)
	int m_blockUntil;       // +0x28: time (ms, GetTimeMs) from which HandleEvents re-checks the blocked inputs
	CEventId m_events[20];  // +0x2c: 20 registered-message handles (UnRegisterMsg loop in the dtor)
};
ZOOSTERS_ASSERT_SIZE(GameController, 0xcc);
}}
#endif
