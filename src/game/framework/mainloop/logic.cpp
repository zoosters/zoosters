// GLOBAL: 0x62a310 RWS::ERROR_NOT_A_VALID_MESSAGE_ID_LG
#define MSG_INVALID ERROR_NOT_A_VALID_MESSAGE_ID_LG
#include "framework/mainloop/mainloopmsg.h"
#include "tfb/level/level.h"
#include "framework/core/streamhandler.h"
#include "framework/mainloop/mainloop.h"

namespace RWS {

namespace MainLoop {
namespace Logic {
namespace anon {
// file-local in the original
int frameNumber = 0;       // GLOBAL: 0x62a30c RWS::MainLoop::Logic::anon::frameNumber
CMsg msgPreTick;        // GLOBAL: 0x62a364 RWS::MainLoop::Logic::anon::msgPreTick
CMsg msgTick;           // GLOBAL: 0x62a36c RWS::MainLoop::Logic::anon::msgTick
CMsg msgPostTick;       // GLOBAL: 0x62a374 RWS::MainLoop::Logic::anon::msgPostTick
int isEnabled = 1;          // GLOBAL: 0x606c40 RWS::MainLoop::Logic::anon::isEnabled
}
using namespace anon;

// 0x461800
void Running()
{
	SendMsg(iMsgSetRunningMode);
	msgPreTick.Id = iMsgRunningPreTick.pEventId;
	msgTick.Id = iMsgRunningTick.pEventId;
	msgPostTick.Id = iMsgRunningPostTick.pEventId;
}

// 0x461860
void SetEnabled(int enabled)
{
	isEnabled = enabled;
}

// 0x461880
void Poll()
{
	if (!(isEnabled && RWS::CLevel::sm_pActiveWorld))
		return;
	SendMsg(msgPreTick);
	msgTick.pData = (void *)(intptr_t)frameNumber;
	frameNumber++;
	SendMsg(msgTick);
	SendMsg(msgPostTick);
	CEventHandler::DeleteEventHandlers(iMsgDeleteEntity);
}

// 0x461920
void Open()
{
	CEventHandler::RegisterMsg(iMsgSetRunningMode, "iMsgSetRunningMode", 0);
	CEventHandler::RegisterMsg(iMsgSetPausedMode, "iMsgSetPausedMode", 0);
	CEventHandler::RegisterMsg(iMsgSetFrozenMode, "iMsgSetFrozenMode", 0);
	CEventHandler::RegisterMsg(iMsgRunningPreTick, "iMsgRunningPreTick", 0);
	CEventHandler::RegisterMsg(iMsgRunningTick, "iMsgRunningTick", "RwUInt32");
	CEventHandler::RegisterMsg(iMsgRunningPostTick, "iMsgRunningPostTick", 0);
	CEventHandler::RegisterMsg(iMsgPausedPreTick, "iMsgPausedPreTick", 0);
	CEventHandler::RegisterMsg(iMsgPausedTick, "iMsgPausedTick", "RwUInt32");
	CEventHandler::RegisterMsg(iMsgPausedPostTick, "iMsgPausedPostTick", 0);
}

// 0x4619d0
void Close()
{
	CEventHandler::UnRegisterMsg(iMsgSetRunningMode);
	CEventHandler::UnRegisterMsg(iMsgSetPausedMode);
	CEventHandler::UnRegisterMsg(iMsgSetFrozenMode);
	CEventHandler::UnRegisterMsg(iMsgRunningPreTick);
	CEventHandler::UnRegisterMsg(iMsgRunningTick);
	CEventHandler::UnRegisterMsg(iMsgRunningPostTick);
	CEventHandler::UnRegisterMsg(iMsgPausedPreTick);
	CEventHandler::UnRegisterMsg(iMsgPausedTick);
	CEventHandler::UnRegisterMsg(iMsgPausedPostTick);
}

namespace anon {

// 0x461a80
void SetRunningMode(RwChunkHeaderInfo &, RwStream *)
{
	Running();
}

// 0x461a30
void SetFrozenMode(RwChunkHeaderInfo &, RwStream *)
{
	SendMsg(iMsgSetFrozenMode);
	msgPreTick.Id = MSG_INVALID.pEventId;
	msgTick.Id = MSG_INVALID.pEventId;
	msgPostTick.Id = MSG_INVALID.pEventId;
}

} // namespace anon

// 0x461a90
void RegisterStreamChunkHandlers()
{
	CStreamHandler::RegisterChunkHandler(0x706, SetFrozenMode);
	CStreamHandler::RegisterChunkHandler(0x707, SetRunningMode);
}

// 0x461ac0
void UnRegisterStreamChunkHandlers()
{
	CStreamHandler::UnRegisterChunkHandler(0x706);
	CStreamHandler::UnRegisterChunkHandler(0x707);
}

} // namespace Logic
} // namespace MainLoop
} // namespace RWS

// GLOBAL: 0x62972c RWS::CLevel::sm_pActiveWorld
// GLOBAL: 0x62a324 RWS::iMsgSetPausedMode
// GLOBAL: 0x62a34c RWS::iMsgPausedPreTick
// GLOBAL: 0x62a354 RWS::iMsgPausedTick
// GLOBAL: 0x62a35c RWS::iMsgPausedPostTick

// 0x461870
int MainLoop_LogicIsEnabled()
{
	return RWS::MainLoop::Logic::anon::isEnabled;
}
