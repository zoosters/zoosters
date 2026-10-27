#include <stdio.h>
#include "framework/startup/win32/cfgobj.h"
#include "framework/core/streamhandler.h"
#include "framework/mainloop/mainloop.h"
#include "framework/startup/startup.h"
#include "rw/rtanim.h"
#include "rw/rttoolkits.h"
#include "rw/rwd3d8.h"

char *GetVideoDir();

typedef RwBool (*PluginAttachFn)(void);
extern const PluginAttachFn g_pluginList[19];   // GLOBAL: 0x5d5cf0 g_pluginList

namespace RWS {
namespace StartUp {

// 0x4627b0
bool AttachPlugins()
{
	if (!RtAnimInitialize())
		return false;
	bool ok = true;
	for (unsigned i = 0; i < 19; i++) {
		if (!g_pluginList[i]())
			ok = false;
	}
	RtCompressedKeyFrameRegister();
	return ok;
}

}  // namespace StartUp
}  // namespace RWS

struct ImageFormat {
	RwImageCallBackRead read;
	RwImageCallBackWrite write;
	const char *name;
};
extern const ImageFormat g_imageFormats[4];   // GLOBAL: 0x5d5d6c g_imageFormats

namespace RWS {
namespace StartUp {

// 0x462880
int RegisterImageLoaders()
{
	bool ok = true;
	for (unsigned i = 0; i < 4; i++) {
		if (!RwImageRegisterImageFormat(g_imageFormats[i].name, g_imageFormats[i].read, g_imageFormats[i].write))
			ok = false;
	}
	return ok;
}

// 0x4627e0
bool LoadGameDatabaseFile(const char *name, bool existsOnly)
{
	char path[0x100];
	RwFreeListPurgeAllFreeLists();
	sprintf(path, "%s%s", GetVideoDir(), name);
	if (existsOnly)
		return RwFexist(path) ? true : false;
	RwStream *stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
	if (stream) {
		bool ok = RWS::CStreamHandler::ProcessStream(stream);
		RwStreamClose(stream, (void *)name);
		RwFreeListPurgeAllFreeLists();
		return ok;
	}
	return false;
}

}  // namespace StartUp
}  // namespace RWS

extern RwMemoryFunctions g_defMemFuncs;       // GLOBAL: 0x606c90 g_defMemFuncs
void Fold_Nop();
namespace RWS { namespace RwsAudio { int RegisterAudioObjects(); } }
// GLOBAL: 0x461f70 ?Open@MainLoop@RWS@@YAXHHH@Z

namespace RWS {
namespace StartUp {

// 0x462aa0
int Open(void *installer, RwMemoryFunctions *memfuncs, int arena, VideoModeRequest *mode, int zDepth, void *displayID)
{
	int result = 0;
	if (!memfuncs)
		memfuncs = &g_defMemFuncs;
	Fold_Nop();
	if (RwEngineInit(memfuncs, 0, arena)) {
		RWS::RwsAudio::RegisterAudioObjects();
		if (RtFSManagerOpen(-1) != 0 && RWS::psInstallFileSystem() && AttachPlugins()) {
			RwEngineOpenParams openParams = { displayID };
			if (RwEngineOpen(&openParams)) {
				if (SelectVideoMode(mode->width, mode->height, mode->depth, mode->flags, zDepth)) {
					if (g_audioDevice->GetInt("Settings\\Display\\FSAA", 0) == 1)
						RwD3D8EngineSetMultiSamplingLevels(2);
					int started = RwEngineStart();
					if (!started) {
						RwD3D8EngineSetMultiSamplingLevels(started);
						started = RwEngineStart();
					}
					if (started) {
						if (RegisterImageLoaders()) {
							RWS::MainLoop::Open(mode->width, mode->height, zDepth);
							result = 1;
						} else {
							RwEngineStop();
						}
					}
				}
				if (!result)
					RwEngineClose();
			}
		}
		if (!result)
			RwEngineTerm();
	}
	return result;
}

// 0x462bc0
void Close()
{
	RWS::MainLoop::Close();
	RwEngineStop();
	RwEngineClose();
	RwEngineTerm();
	RtFSManagerClose();
	Fold_Nop();
	Fold_Nop();
}

}  // namespace StartUp
}  // namespace RWS
