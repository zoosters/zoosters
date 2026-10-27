// vtable TU: the scaffold constructor makes cl emit the vtables, the scalar deleting destructor and the thunk
#include "modules/tfb/gamecontroller.h"
#include "framework/core/ceventhandler.h"
extern RWS::TFB::GameController *g_gameController;   // GLOBAL: 0x61798c g_gameController (the one controller behaviour, 0 while there is none)
namespace RWS { namespace TFB {
// GLOBAL: 0x5ca5a0 RWS::TFB::GameController::vftable_for_CEventHandler
// GLOBAL: 0x5ca590 RWS::TFB::GameController::vftable_for_CAttributeHandler
// 0x403f40
// 0x402af0
// GLOBAL: 0x558bd3 ??_M@YGXPAXIHP6EX0@Z@Z
// GLOBAL: 0x558c33 ??_L@YGXPAXIHP6EX0@Z1@Z
// GLOBAL: 0x550250 ??0CEventId@RWS@@QAE@XZ
// GLOBAL: 0x464ef0 ??1CEventId@RWS@@QAE@XZ
// GLOBAL: 0x550760 ?UnLinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@@Z
// GLOBAL: 0x5506d0 ?UnRegisterMsg@CEventHandler@RWS@@SAXAAVCEventId@2@@Z
// GLOBAL: 0x550440 ?RegisterMsg@CEventHandler@RWS@@SAXAAVCEventId@2@PBD1@Z
// GLOBAL: 0x550ac0 ?LinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@PBDG@Z
// 0x402600
GameController::GameController(const CAttributePacket &) : m_blockMask(0), m_blockUntil(0)
{
	g_gameController = this;
	const char *names[20] = {
		"INQ_ACTN_START", "INQ_ACTN_SELECT", "INQ_ACTN_TRICK", "INQ_ACTN_FIRE", "INQ_ACTN_JUMP",
		"INQ_ACTN_DODGE", "INQ_SHOULDER_LEFT", "INQ_SHOULDER_RIGHT", "INQ_ACTN_SHOULDER_L2", "INQ_ACTN_SHOULDER_R2",
		"INQ_ACTN_PAD_L", "INQ_ACTN_PAD_R", "INQ_ACTN_PAD_U", "INQ_ACTN_PAD_D", "INQ_ACTN_TURN",
		"INQ_ACTN_FWD", "INQ_LPAD_BUTTON", "INQ_CAMERA_ROTATE", "INQ_CAMERA_PITCH", "INQ_RPAD_BUTTON"
	};
	for (int i = 0; i < 20; i++)
		RegisterMsg(m_events[i], names[i], "return RwReal");
	LinkMsg(RWS::iMsgStartSystem);
	LinkMsg(RWS::iMsgStopSystem);
	LinkMsg(RWS::iMsgRunningPostTick);
}

// 0x402790
GameController::~GameController()
{
	g_gameController = 0;
	UnLinkMsg(RWS::iMsgRunningPostTick);
	UnLinkMsg(RWS::iMsgStartSystem);
	UnLinkMsg(RWS::iMsgStopSystem);
	for (int i = 0; i < 20; i++)
		UnRegisterMsg(m_events[i]);
}
}}

namespace RWS { namespace TFB {
// 0x445eb0
// (shared with MemCard::HandleAttributes: the game has one folded copy)
void GameController::HandleAttributes(const CAttributePacket &attr)
{
	CAttributeHandler::HandleAttributes(attr);
}
}}
