#include "framework/core/streamhandler.h"
#include "framework/core/loadhooks.h"
#include "framework/core/memoryhandler.h"
#include "rw/rwcore.h"
#include "rw/rttoolkits.h"

// GLOBAL: 0x606cc8 TFBStreamPump
// GLOBAL: 0x460da0 ?TFBLoadingProgress@@YAXXZ

// std::map<unsigned, StreamChunkHandler> members the game emits out of line.
// 0x5525b0
// 0x5525d0
// 0x552630
// 0x552690
// 0x5526d0
// 0x552740
// 0x5527f0
// 0x552ab0
// 0x552b70
// 0x552d20
// 0x552de0
// 0x552e40

namespace RWS {

namespace anon {

// Version number chunk (0x7ff): discarded.
// 0x552580
void VersionNumber(RwChunkHeaderInfo &header, RwStream *stream)
{
	void *buffer = MemAllocHint(header.length, 2);
	RwStreamRead(stream, buffer, header.length);
	operator delete(buffer);
}

}  // namespace anon

// 0x552e70
CStreamHandler *CStreamHandler::Instance()
{
	static CStreamHandler instance;
	return &instance;
}

// 0x552f00
void CStreamHandler::RegisterChunkHandler(unsigned ChunkType, StreamChunkHandler ChunkHandler)
{
	// the first registration wins
	Instance()->m_handlers.insert(std::make_pair(ChunkType, ChunkHandler));
}

// 0x552f50
void CStreamHandler::UnRegisterChunkHandler(unsigned ChunkType)
{
	Instance()->m_handlers.erase(ChunkType);
}

bool CStreamHandler::IsAtEnd(RwStream *stream)
{
	switch (stream->type) {
	case rwSTREAMFILE:
	case rwSTREAMFILENAME:
		return RwFeof(stream->Type.fpFile) != 0;
	case rwSTREAMMEMORY:
		return stream->Type.memory.position == stream->Type.memory.nSize;
	case rwSTREAMCUSTOM:
		return stream->Type.custom.sfnread(stream->Type.custom.data, 0, 0) == 0;
	default:
		return false;
	}
}

// 0x552f70
bool CStreamHandler::ProcessStream(RwStream *pStream)
{
	CStreamHandler *self = Instance();
	while (!IsAtEnd(pStream)) {
		(*TFBStreamPump)(0);
		RwStreamReadChunkHeaderInfo(pStream, &self->m_header);
		std::map<unsigned, StreamChunkHandler>::const_iterator found = self->m_handlers.find(self->m_header.type);
		if (found != self->m_handlers.end())
			found->second(self->m_header, pStream);
		else
			RwStreamSkip(pStream, self->m_header.length);
		TFBLoadingProgress();
	}
	return true;
}

// 0x553040
void CStreamHandler::RegisterStreamChunkHandlers()
{
	RegisterChunkHandler(0x7ff, anon::VersionNumber);
}

// 0x5530a0
void CStreamHandler::UnRegisterStreamChunkHandlers()
{
	UnRegisterChunkHandler(0x7ff);
}

}  // namespace RWS
