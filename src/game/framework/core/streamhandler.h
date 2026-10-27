// Reads a .stream chunk by chunk and hands each to the handler registered for its type.
#ifndef ZOOSTERS_FRAMEWORK_CORE_STREAMHANDLER_H
#define ZOOSTERS_FRAMEWORK_CORE_STREAMHANDLER_H

#include "zoosters/base.h"
#include "rw/rwstream.h"
// script.h defines the placement operator new
#include "script/script.h"
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
#endif
#pragma warning(disable : 4291)
#include <map>

namespace RWS {

typedef void (*StreamChunkHandler)(RwChunkHeaderInfo &rChunkHeader, RwStream *pStream);

class CStreamHandler {
public:
	static CStreamHandler *Instance();
	static void RegisterChunkHandler(unsigned ChunkType, StreamChunkHandler ChunkHandler);
	static void UnRegisterChunkHandler(unsigned ChunkType);
	static bool ProcessStream(RwStream *pStream);
	static void RegisterStreamChunkHandlers();
	static void UnRegisterStreamChunkHandlers();

private:
	static bool IsAtEnd(RwStream *stream);

	std::map<unsigned, StreamChunkHandler> m_handlers;
	RwChunkHeaderInfo m_header;   // handlers get a reference to it
};

}  // namespace RWS

#endif
