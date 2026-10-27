#include "framework/toolkits/rwaudresource/rwaudresource.h"
#include "framework/toolkits/audio/audio.h"
#include "rw/rwcore.h"

// GLOBAL: 0x62dc30 g_wdScratch
// GLOBAL: 0x62dc34 g_wdScratchSize
// GLOBAL: 0x5d52a8 RWS::CAudioResource::vftable
extern void *g_wdScratch;       // the upload buffer RWA reads wave data through (RwaWaveDictSetStreamIOBuffer)
extern int g_wdScratchSize;

RwStream *RwStreamSkip(RwStream *s, unsigned size);   // 0x4efa90
const char *GetLanguagePrefix();   // "LANG_"
int IsCurrentLanguageName(const char *name);

namespace RWS {

// rwaID_WAVEDICT chunk id; WAVEUPLOADBUFFSIZE.
static const int kWaveDictChunkId = 0x809;
static const int kWaveUploadBufferSize = 0x40000;

// 0x44b9d0
CAudioResource::CAudioResource() {}

// 0x44b9a0
int CAudioResource::IsHandled(const char *psType)
{
	return 0 == rwstrcmp(psType, "rwaID_WAVEDICT");
}

// 0x44b980
int CAudioResource::UnLoad(const char *, void *pResource)
{
	RwsAudio::RemoveDictionary(static_cast<RwaWaveDict *>(pResource));
	return 1;
}

// 0x44b830
RwaWaveDict *CAudioResource::WaveDictStreamRead(RwStream *pRwStream)
{
	void *uploadBuffer = RwCalloc(kWaveUploadBufferSize + 1, 1, 0x10000);
	if (uploadBuffer) {
		g_wdScratch = uploadBuffer;
		g_wdScratchSize = kWaveUploadBufferSize;
	} else {
		g_wdScratch = 0;
		g_wdScratchSize = 0;
	}
	RwaWaveDict *dict;
	if (!RwStreamFindChunk(pRwStream, kWaveDictChunkId, 0, 0))
		dict = (RwaWaveDict *)pRwStream;
	else
		dict = _rwaWaveDictStreamRead(0, pRwStream, RwsAudio::GetOutputObject(), 0, 0, 0);
	RwFree(uploadBuffer);
	return dict;
}

// 0x44b8d0
void *CAudioResource::Load(const char *psName, const char *psType, const char *, RwStream *pStream,
                           unsigned uiStreamSize, unsigned &uiResourceSize)
{
	if (uiStreamSize) {
		if (!rwstrcmp(psType, "rwaID_WAVEDICT")) {
			// through the address of the function pointer: cl then loads the slot address before the argument calls, as the game does
			int (**strncmpSlot)(const RwChar *, const RwChar *, size_t) = &RwEngineGlobals()->stringFuncs.vecStrncmp;
			if (!(*strncmpSlot)(psName, GetLanguagePrefix(), 5) && !(char)IsCurrentLanguageName(psName + 5)) {
				RwStreamSkip(pStream, uiStreamSize);
				uiResourceSize = 0;
				return (void *)-1;
			}
			RwaWaveDict *dict = WaveDictStreamRead(pStream);
			if (dict)
				RwsAudio::AddDictionary(dict);
			return dict;
		}
	}
	return 0;
}

}   // namespace RWS
