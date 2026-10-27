// Stream chunk handlers: embedded asset (0x716), asset in its own file (0x715), unload (0x70f).
#include <string.h>
#include "framework/core/cresourcemanager.h"
#include "framework/core/cresourcehandler.h"
#include "framework/core/memoryhandler.h"
#include "framework/core/streamhandler.h"
#include "rw/rwcore.h"

namespace RWS {

namespace {

// The strings point into the buffer the header was read into.
struct AssetHeader {
	const char *name;
	RWSGUID id;
	const char *type;
	const char *fileName;
	const char *location;
	unsigned global;   // non-zero: lock the resource so that level purges keep it
};

unsigned ReadWord(const unsigned char *&cursor)
{
	unsigned value;
	memcpy(&value, cursor, 4);
	cursor += 4;
	return value;
}

// The length covers the text, its NUL and the padding.
const char *ReadString(const unsigned char *&cursor)
{
	unsigned length = ReadWord(cursor);
	const char *text = reinterpret_cast<const char *>(cursor);
	cursor += length;
	return text;
}

void ParseAssetHeader(const unsigned char *data, AssetHeader &header)
{
	const unsigned char *cursor = data;
	header.name = ReadString(cursor);
	memcpy(header.id.m_Data, cursor, sizeof(header.id.m_Data));
	cursor += sizeof(header.id.m_Data);
	header.type = ReadString(cursor);
	header.fileName = ReadString(cursor);
	header.location = ReadString(cursor);
	header.global = ReadWord(cursor);
}

}  // namespace

namespace anon {

// 0x5532d0
void UnLoadAsset(RwChunkHeaderInfo &chunk, RwStream *stream)
{
	unsigned char *buffer = static_cast<unsigned char *>(MemAllocHint(chunk.length, 2));
	RwStreamRead(stream, buffer, chunk.length);
	RWSGUID id;
	id.Clear();
	if (buffer)
		memcpy(id.m_Data, buffer, chunk.length < sizeof(id.m_Data) ? chunk.length : sizeof(id.m_Data));
	if (const void *data = CResourceManager::FindById(&id))
		CResourceManager::Remove(data);
	operator delete(buffer);
}

// 0x553320
void LoadAsset(RwChunkHeaderInfo &chunk, RwStream *stream)
{
	unsigned char *buffer = static_cast<unsigned char *>(MemAllocHint(chunk.length, 2));
	RwStreamRead(stream, buffer, chunk.length);
	AssetHeader asset;
	ParseAssetHeader(buffer, asset);
	if (!CResourceManager::FindById(&asset.id)) {
		void *data = CResourceManager::Load(asset.name, &asset.id, asset.type, asset.fileName, asset.location);
		if (asset.global)
			CResourceManager::Lock(data);
	}
	operator delete(buffer);
}

// 0x5533c0
void LoadEmbeddedAsset(RwChunkHeaderInfo &chunk, RwStream *stream)
{
	unsigned headerSize = 0;
	RwStreamRead(stream, &headerSize, 4);
	unsigned char *buffer = static_cast<unsigned char *>(MemAllocHint(headerSize, 2));
	RwStreamRead(stream, buffer, headerSize);
	// must be the last 4-byte read before the payload (the shim re-aligns readers by it)
	unsigned dataSize = 0;
	RwStreamRead(stream, &dataSize, 4);

	AssetHeader asset;
	ParseAssetHeader(buffer, asset);
	if (CResourceManager::FindById(&asset.id)) {
		RwStreamSkip(stream, dataSize);
	} else if (dataSize != 0) {
		void *data = CResourceManager::UpLoad(asset.name, &asset.id, asset.type, asset.location, stream, dataSize);
		if (!data && !CResourceHandler::FindHandler(asset.type))
			RwStreamSkip(stream, dataSize);   // keep the stream in step
		else if (asset.global)
			CResourceManager::Lock(data);
	}

	int trailer = static_cast<int>(chunk.length - dataSize - headerSize - 8);
	if (trailer > 0)
		RwStreamSkip(stream, trailer);
	operator delete(buffer);
}

}  // namespace anon

// 0x5534c0
void CResourceManager::RegisterStreamChunkHandlers()
{
	CStreamHandler::RegisterChunkHandler(0x70f, anon::UnLoadAsset);
	CStreamHandler::RegisterChunkHandler(0x715, anon::LoadAsset);
	CStreamHandler::RegisterChunkHandler(0x716, anon::LoadEmbeddedAsset);
}

// 0x553500
void CResourceManager::UnRegisterStreamChunkHandlers()
{
	CStreamHandler::UnRegisterChunkHandler(0x70f);
	CStreamHandler::UnRegisterChunkHandler(0x715);
	CStreamHandler::UnRegisterChunkHandler(0x716);
}

}  // namespace RWS
