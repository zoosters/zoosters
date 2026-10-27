// Engine start and stop, video mode, file systems, loading screen.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_STARTUP_H
#define ZOOSTERS_FRAMEWORK_STARTUP_STARTUP_H

#include "rw/rwcore.h"
#include "rw/rwengine.h"

// What WinMain asks StartUp::Open for. flags: 1 = exclusive. SelectVideoMode may change the size.
struct VideoModeRequest {
	RwInt32 width;
	RwInt32 height;
	RwInt32 depth;
	RwUInt32 flags;
};
ZOOSTERS_ASSERT_SIZE(VideoModeRequest, 0x10);

namespace RWS {

int psInstallFileSystem();   // one RtWinFSystem per drive, or the UNC root

namespace StartUp {

bool AttachPlugins();
int RegisterImageLoaders();
// Best match by score: depth, size, exclusive flag.
int SelectVideoMode(int &screenWidth, int &screenHeight, int bitDepth, unsigned flags, unsigned zDepth);
int Open(void *installer, RwMemoryFunctions *memfuncs, int arena, VideoModeRequest *mode, int zDepth, void *displayID);
void Close();
// The path is the video directory plus `name`; existsOnly only tests the file.
bool LoadGameDatabaseFile(const char *name, bool existsOnly);

}  // namespace StartUp
}  // namespace RWS

// loadscreen.cpp
bool LoadScreen_LoadImage(const char *name);   // Images/<language>/<name> into the loading raster (0 = none)
void LoadScreen_Draw(RwCamera *camera);
void LoadScreen_Show(RwCamera *camera, const char *name);   // draws twice, once per buffer
void LoadScreen_DestroyRaster();
// framework/loadprogress.cpp
void LoadScreen_Begin(unsigned duration);
void LoadScreen_FreeTextures();

#endif
