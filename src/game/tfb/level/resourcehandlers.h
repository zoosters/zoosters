// The game's per-type resource handlers (RWS::CResourceHandler subclasses). The overrides forward to the game's bodies in resourcehandlers.cpp.
#ifndef ZOOSTERS_TFB_LEVEL_RESOURCEHANDLERS_H
#define ZOOSTERS_TFB_LEVEL_RESOURCEHANDLERS_H

#include "framework/core/cresourcehandler.h"

// The compiler's vtables forward to the game's handler bodies (plain __stdcall functions and Studio resource class members).
#define ZOOSTERS_RES_HANDLER(NAME) \
	class NAME : public RWS::CResourceHandler { \
	public: \
		NAME(); \
		virtual int IsHandled(const char *); \
		virtual void *Load(const char *, const char *, const char *, RwStream *, unsigned, unsigned &); \
		virtual int UnLoad(const char *, void *); \
		virtual ~NAME() {} \
	}

ZOOSTERS_RES_HANDLER(DMorphResourceHandler);
ZOOSTERS_RES_HANDLER(RenderwareResourceHandler);
ZOOSTERS_RES_HANDLER(TextResourceHandler);
ZOOSTERS_RES_HANDLER(SplineResourceHandler);
ZOOSTERS_RES_HANDLER(KFsetResourceHandlerG);
ZOOSTERS_RES_HANDLER(ScriptResourceHandlerG);
ZOOSTERS_RES_HANDLER(TextStringDictResourceHandler);

#endif
