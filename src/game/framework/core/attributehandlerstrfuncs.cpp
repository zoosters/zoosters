// Stream chunk handlers: create, update and delete entities from attribute packets.
#include "framework/core/cattributehandler.h"
#include "framework/core/ceventhandler.h"
#include "framework/core/memoryhandler.h"
#include "framework/core/streamhandler.h"
#include "rw/rwcore.h"

namespace RWS {

namespace {

enum {
	CHUNK_CREATE_ENTITY = 0x704,
	CHUNK_UPDATE_ENTITY_ATTRIBUTES = 0x705,
	CHUNK_DELETE_ENTITY = 0x70d,
	CHUNK_DELETE_ALL_ENTITIES = 0x70e,
	CHUNK_PROFILER_MESSAGES = 0x71a,
	CHUNK_CREATE_ENTITY_2 = 0x71b,
};

// Freed with operator delete.
char *ReadPayload(RwChunkHeaderInfo &header, RwStream *stream)
{
	char *buffer = static_cast<char *>(MemAllocHint(header.length, 2));
	RwStreamRead(stream, buffer, header.length);
	return buffer;
}

// 0x5531a0
void CreateEntity(RwChunkHeaderInfo &header, RwStream *stream)
{
	char *buffer = ReadPayload(header, stream);
	unsigned isGlobal = *reinterpret_cast<const unsigned *>(buffer);
	const CAttributePacket *packet = reinterpret_cast<const CAttributePacket *>(buffer + 4);
	CAttributeHandler *handler = packet->CreateEntity();
	if (handler && isGlobal)
		handler->Lock();
	operator delete(buffer);
}

// 0x553140
void UpdateEntityAttributes(RwChunkHeaderInfo &header, RwStream *stream)
{
	char *buffer = ReadPayload(header, stream);
	const CAttributePacket *packet = reinterpret_cast<const CAttributePacket *>(buffer);
	if (CAttributeHandler *handler = CAttributeHandler::Find(packet->GetInstanceId()))
		handler->HandleAttributes(*packet);
	operator delete(buffer);
}

// 0x5530c0
void DeleteEntity(RwChunkHeaderInfo &header, RwStream *stream)
{
	char *buffer = ReadPayload(header, stream);
	const CAttributePacket *packet = reinterpret_cast<const CAttributePacket *>(buffer);
	if (CAttributeHandler *handler = CAttributeHandler::Find(packet->GetInstanceId()))
		delete handler;
	operator delete(buffer);
}

// 0x553120
// The payload is not consumed.
void DeleteAllEntities(RwChunkHeaderInfo &header, RwStream *stream)
{
	CEventHandler::DeleteEventHandlers(iMsgDeleteEventHandler);
	CAttributeHandler::Purge();
}

} // namespace

// 0x5531f0
void HandleProfilerMessages(RwChunkHeaderInfo &header, RwStream *stream)
{
	RwUInt32 words[2];
	RwStreamRead(stream, words, sizeof(words));
}

// 0x553230
void CAttributeHandler::RegisterStreamChunkHandlers()
{
	CStreamHandler::RegisterChunkHandler(CHUNK_CREATE_ENTITY, CreateEntity);
	CStreamHandler::RegisterChunkHandler(CHUNK_UPDATE_ENTITY_ATTRIBUTES, UpdateEntityAttributes);
	CStreamHandler::RegisterChunkHandler(CHUNK_DELETE_ENTITY, DeleteEntity);
	CStreamHandler::RegisterChunkHandler(CHUNK_DELETE_ALL_ENTITIES, DeleteAllEntities);
	CStreamHandler::RegisterChunkHandler(CHUNK_PROFILER_MESSAGES, HandleProfilerMessages);
	CStreamHandler::RegisterChunkHandler(CHUNK_CREATE_ENTITY_2, CreateEntity);
}

// 0x553290
void CAttributeHandler::UnRegisterStreamChunkHandlers()
{
	CStreamHandler::UnRegisterChunkHandler(CHUNK_UPDATE_ENTITY_ATTRIBUTES);
	CStreamHandler::UnRegisterChunkHandler(CHUNK_CREATE_ENTITY);
	CStreamHandler::UnRegisterChunkHandler(CHUNK_DELETE_ENTITY);
	CStreamHandler::UnRegisterChunkHandler(CHUNK_DELETE_ALL_ENTITIES);
	CStreamHandler::UnRegisterChunkHandler(CHUNK_PROFILER_MESSAGES);
	CStreamHandler::UnRegisterChunkHandler(CHUNK_CREATE_ENTITY_2);
}

} // namespace RWS
