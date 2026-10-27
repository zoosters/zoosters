// The main loop's message ids, and the SendMsg forms that test each module's copy of the invalid id.
#ifndef ZOOSTERS_FRAMEWORK_MAINLOOP_MAINLOOPMSG_H
#define ZOOSTERS_FRAMEWORK_MAINLOOP_MAINLOOPMSG_H

#include "framework/core/cattributehandler.h"
#include "framework/core/ceventhandler.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/cresourcemanager.h"
#include "rw/rwstream.h"

#ifndef MSG_INVALID
#error define MSG_INVALID before including framework/mainloop/mainloopmsg.h
#endif

namespace RWS {

// Every module of the game has its own copy of the invalid id: the .cpp defines MSG_INVALID (and annotates it) first.
extern CEventId MSG_INVALID;

inline bool SendMsg(const CEventId &id)
{
	if (id.pEventId != MSG_INVALID.pEventId && id.pEventId->Erm.p_next) {
		CMsg m;
		m.Id = id.pEventId;
		m.pData = 0;
		return _SendMsg(m) != 0;
	}
	return false;
}

inline bool SendMsg(CMsg &m)
{
	if (m.Id != MSG_INVALID.pEventId && m.Id->Erm.p_next)
		return _SendMsg(m) != 0;
	return false;
}

extern CEventId iMsgSetRunningMode;
extern CEventId iMsgSetPausedMode;
extern CEventId iMsgSetFrozenMode;
extern CEventId iMsgRunningPreTick;
extern CEventId iMsgRunningTick;
extern CEventId iMsgRunningPostTick;
extern CEventId iMsgPausedPreTick;
extern CEventId iMsgPausedTick;
extern CEventId iMsgPausedPostTick;
extern CEventId iMsgDeleteEntity;
extern CEventId iMsgStartSystem;
extern CEventId iMsgStopSystem;
extern CEventId iMsgDeleteEventHandler;
extern CEventId iMsgDoRender;
extern CEventId iMsgDoRenderDirectorsCamera;
extern CEventId iMsgPreShowRaster;

}  // namespace RWS

#endif
