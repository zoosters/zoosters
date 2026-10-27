#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rpplugins.h"
#include "rw/rtanim.h"
#include "framework/core/rwresources.h"
#include "framework/core/memoryhandler.h"
#include <string.h>

// 0x44b700
int RWS::CRenderwareResource::IsHandled(const char *type)
{
	if (type[0] == 'r' && type[1] == 'w' && type[2] == 'I' && type[3] == 'D') {
		const char *p = type + 5;
		if (!rwstrcmp(p, "WORLD") || !rwstrcmp(p, "TEXDICTIONARY") || !rwstrcmp(p, "CLUMP") ||
		    !rwstrcmp(p, "ATOMIC") || !rwstrcmp(p, "HANIMANIMATION") || !rwstrcmp(p, "2DMAESTRO") ||
		    !rwstrcmp(p, "PITEXDICTIONARY") || !rwstrcmp(p, "2DFONT"))
			return 1;
	}
	return 0;
}

// 0x44b5d0
int RWS::CRenderwareResource::UnLoad(const char *type, void *resource)
{
	int ret = 0;
	const char *p = type + 5;
	if (!rwstrcmp(p, "WORLD"))
		return RpWorldDestroy((RpWorld *)resource);
	if (!rwstrcmp(p, "TEXDICTIONARY") || !rwstrcmp(p, "PITEXDICTIONARY"))
		return RwTexDictionaryDestroy((RwTexDictionary *)resource);
	if (!rwstrcmp(p, "CLUMP"))
		return RpClumpDestroy((RpClump *)resource);
	if (!rwstrcmp(p, "ATOMIC"))
		return RpAtomicDestroy((RpAtomic *)resource);
	if (!rwstrcmp(p, "HANIMANIMATION"))
		return RtAnimAnimationDestroy((RtAnimAnimation *)resource);
	if (!rwstrcmp(p, "2DFONT"))
		ret = 1;
	return ret;
}

// 0x44b9f0
void *RWS::CTextResource::Load(const char *psName, const char *psType, const char *psResourcePath, RwStream *pStream,
                               unsigned uiStreamSize, unsigned &uiResourceSize)
{
	void *buffer = 0;
	if (uiStreamSize > 0) {
		buffer = MemAllocHint(uiStreamSize, 2);
		RwStreamRead(pStream, buffer, uiStreamSize);
	}
	uiResourceSize = uiStreamSize;
	return buffer;
}

// 0x44ba30
int RWS::CTextResource::UnLoad(const char *psType, void *pResource)
{
	operator delete(pResource);
	return 1;
}

// 0x44f270
void *RWS::CRwSplineResource::Load(const char *name, const char *type, const char *path, RwStream *stream, unsigned size,
                                   unsigned &outSize)
{
	void *res = 0;
	if (size > 0) {
		if (path)
			RwImageSetPath(path);
		RwChunkHeaderInfo info;
		RwStreamReadChunkHeaderInfo(stream, &info);
		if (info.type == 0xc)
			res = RpSplineStreamRead(stream);
	} else {
		int *emptyMarker = (int *)MemAllocHint(4, 2);
		*emptyMarker = -1;
		res = emptyMarker;
	}
	outSize = size;
	return res;
}

// 0x44f300
int RWS::CRwSplineResource::UnLoad(const char *type, void *resource)
{
	int ok = 0;
	if (*(int *)resource == -1) {
		operator delete(resource);
		ok = 1;
	} else if (!rwstrcmp(type, "rwID_SPLINE")) {
		RpSplineDestroy((RpSpline *)resource);
		ok = 1;
	}
	return ok;
}
