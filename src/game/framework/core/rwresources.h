// Resource classes: RenderWare data (rwID_* types), raw text, splines.
#pragma once

struct RwStream;

namespace RWS {

struct CRenderwareResource {
	int IsHandled(const char *type);
	int UnLoad(const char *type, void *resource);
};

class CTextResource {
public:
	void *Load(const char *psName, const char *psType, const char *psResourcePath, RwStream *pStream,
	           unsigned uiStreamSize, unsigned &uiResourceSize);
	int UnLoad(const char *psType, void *pResource);
};

class CRwSplineResource {
public:
	void *Load(const char *name, const char *type, const char *path, RwStream *stream, unsigned size, unsigned &outSize);
	int UnLoad(const char *type, void *resource);
};

}   // namespace RWS
