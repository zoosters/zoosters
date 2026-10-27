// Registry of loaded resources, found by GUID or name; loaded and unloaded through the CResourceHandler of their type.
#ifndef ZOOSTERS_FRAMEWORK_CORE_CRESOURCEMANAGER_H
#define ZOOSTERS_FRAMEWORK_CORE_CRESOURCEMANAGER_H

#include "framework/core/rwsguid.h"

struct RwStream;

namespace RWS {

class CResourceIterator;

class CResourceManager {
public:
	static void Add(const char *_pResName, const RWSGUID *_pResId, const char *_pResType, const void *_pResData,
	                unsigned _ResDataSize);
	static const void *Remove(const void *_pResData);
	static const void *FindById(const RWSGUID *_pResId, const char **_pResType = 0, unsigned *_ResDataSize = 0,
	                            const char **_pResName = 0);
	static const void *FindByName(const char *const _pResName, RWSGUID *_pResId = 0, const char **_pResType = 0,
	                              unsigned *_ResDataSize = 0);
	static void Lock(const void *ptr);
	static void *Load(const char *pStrName, const RWSGUID *pResId, const char *pStrType, const char *pStrFName,
	                  const char *pStrLoc);
	static void *UpLoad(const char *pStrName, const RWSGUID *pResId, const char *pStrType, const char *pStrLoc,
	                    RwStream *pStream, unsigned nEmbDataSize);
	static bool Purge(void);
	static bool PurgeAll(void);
	static void Open(void);
	static void Close(void);
	static void RegisterStreamChunkHandlers();
	static void UnRegisterStreamChunkHandlers();
protected:
	static bool PurgeItem(CResourceIterator &Ittr);
};

// Port: keep the names across StrPool_Destroy.
char *SaveResourceNames();
void RestoreResourceNames(char *names);

}  // namespace RWS

#endif
