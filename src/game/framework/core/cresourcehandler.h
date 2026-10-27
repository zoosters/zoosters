// Base of the per-type resource loaders; each links itself into a list at construction.
#ifndef ZOOSTERS_FRAMEWORK_CORE_CRESOURCEHANDLER_H
#define ZOOSTERS_FRAMEWORK_CORE_CRESOURCEHANDLER_H

#include "zoosters/base.h"

struct RwStream;

namespace RWS {

class CResourceHandler {
public:
	virtual void Open(void) {}
	virtual void Close(void) {}
	virtual int IsHandled(const char *psType) = 0;
	virtual void *Load(const char *psName, const char *psType, const char *psResourcePath, RwStream *pStream,
	                   unsigned uiStreamSize, unsigned &uiResourceSize) = 0;
	virtual int UnLoad(const char *pStrType, void *pResource) = 0;
	CResourceHandler(void);
	virtual ~CResourceHandler(void);
	static CResourceHandler *FindHandler(const char *const psType);
	static void OpenAll(void);
	static void CloseAll(void);

private:
	CResourceHandler *m_pNext;
	static CResourceHandler *g_pHead;
};
ZOOSTERS_ASSERT_SIZE(CResourceHandler, 8);

}  // namespace RWS

#endif
