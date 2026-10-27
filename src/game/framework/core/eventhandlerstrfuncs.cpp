// Stream chunk handler: sends the message named by the payload (a test event).
#include "framework/core/ceventhandler.h"
#include "framework/core/memoryhandler.h"
#include "framework/core/streamhandler.h"
#include "rw/rwcore.h"

namespace RWS {

namespace {

const unsigned CHUNK_TEST_EVENT = 0x712;

// 0x553530
void SendTestEvent(RwChunkHeaderInfo &header, RwStream *stream)
{
	// the extra byte terminates the name if the chunk lacks a NUL
	char *name = static_cast<char *>(MemAllocHint(header.length + 1, 2));
	RwStreamRead(stream, name, header.length);
	name[header.length] = 0;
	CEventId id;
	CEventHandler::RegisterMsg(id, name, 0);
	if (id.pEventId && id.pEventId->Erm.p_next) {
		CMsg msg(id);
		_SendMsg(msg);
	}
	CEventHandler::UnRegisterMsg(id);
	::operator delete(name);
}

} // namespace

// 0x5535f0
void CEventHandler::RegisterStreamChunkHandlers()
{
	CStreamHandler::RegisterChunkHandler(CHUNK_TEST_EVENT, SendTestEvent);
}

// 0x553610
void CEventHandler::UnRegisterStreamChunkHandlers()
{
	CStreamHandler::UnRegisterChunkHandler(CHUNK_TEST_EVENT);
}

} // namespace RWS
