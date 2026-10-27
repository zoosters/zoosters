// Engine records rwcore.h only forward-declares: the video mode, sub system and RwEngineOpen parameters (RW 3.6 public layouts).
// Kept apart on purpose: defining them in rwcore.h changed cl's code for 0x463640 in an unrelated TU. Include it where the fields are read.
#ifndef ZOOSTERS_RW_RWENGINE_H
#define ZOOSTERS_RW_RWENGINE_H

#include "rw/rwcore.h"

// A video mode of a sub system. Flags: 1 = exclusive (full screen). The game reads width, height, depth and flags
// (RWS::StartUp::SelectVideoMode 0x462bf0).
enum RwVideoModeFlag { rwVIDEOMODEEXCLUSIVE = 0x1, rwVIDEOMODEINTERLACE = 0x4 };  // 0x4 (interlace) doubles the nominal height: RWS::CameraHelper::SetFieldOfView 0x44ea40
struct RwVideoMode {
	RwInt32 width;
	RwInt32 height;
	RwInt32 depth;
	RwUInt32 flags;
	RwInt32 refRate;
	RwInt32 format;
};
ZOOSTERS_ASSERT_SIZE(RwVideoMode, 0x18);

// RwEngineGetSubSystemInfo: the name of a display adapter (80 characters).
struct RwSubSystemInfo {
	RwChar name[80];
};

// RwEngineOpen's argument: the window of the display (RWS::StartUp::Open 0x462aa0 passes the game window).
struct RwEngineOpenParams {
	void *displayID;
};

#endif
