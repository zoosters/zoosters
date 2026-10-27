// Resource handler for "rwaID_WAVEDICT" wave dictionaries. Open / Close are not overridden, as in the game's vtable.
#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_RWAUDRESOURCE_RWAUDRESOURCE_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_RWAUDRESOURCE_RWAUDRESOURCE_H

#include "framework/core/cresourcehandler.h"
#include "rw/rwaudio.h"

namespace RWS {

class CAudioResource : public CResourceHandler {
public:
	CAudioResource();
	virtual int IsHandled(const char *psType);
	// Skips ("LANG_" names of another language): -1, size 0. Else reads the dictionary into RwsAudio.
	virtual void *Load(const char *psName, const char *psType, const char *psResourcePath, RwStream *pStream, unsigned uiStreamSize,
	                   unsigned &uiResourceSize);
	virtual int UnLoad(const char *pStrType, void *pResource);

private:
	// Returns the stream itself when the 0x809 chunk is missing; the game then registers the stream pointer as a dictionary.
	RwaWaveDict *WaveDictStreamRead(RwStream *pRwStream);
};

}   // namespace RWS

#endif
