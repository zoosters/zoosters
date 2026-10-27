#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_FRAMEHELPER_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_FRAMEHELPER_H
#include "rw/rwcore.h"

int FUN_0043e6f0(const char *msg);   // the release build's debug print: empty

RwFrame *RwFrameCreate_thunk(void);   // thunk to 0x4d8d40: the framework's frame create, not RW's

namespace RWS {
namespace FrameHelper {
RwBool FrameDestroy(RwFrame *pFrame);
RwFrame *RemoveChildFrame(RwFrame *pFrame, void *);
}
}
#endif
