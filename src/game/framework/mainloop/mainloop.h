// Main loop: Logic, Render and the start/stop/reset handlers.
#ifndef ZOOSTERS_FRAMEWORK_MAINLOOP_MAINLOOP_H
#define ZOOSTERS_FRAMEWORK_MAINLOOP_MAINLOOP_H

#include "rw/rwcore.h"

namespace RWS {
namespace MainLoop {

void Open(int width, int height, int zdepth);
void Close();
void Poll();

namespace Logic {
ZOOSTERS_NOINLINE void Running();
void SetEnabled(int enabled);
void Poll();
void Open();
void Close();
void RegisterStreamChunkHandlers();
void UnRegisterStreamChunkHandlers();
extern unsigned Rate;                    // GLOBAL: 0x606c3c RWS::MainLoop::Logic::Rate
}

namespace Render {
RwCamera *GetCamera();
unsigned GetFrameCount();
void SetCamera(RwCamera *camera);
void Open();
void Close();
ZOOSTERS_NOINLINE void SetEnabled(int enabled);
void Poll();
}

namespace anon {
void NewEntityCreatedCB();
void StartSystem(RwChunkHeaderInfo &, RwStream *);
void StopSystem(RwChunkHeaderInfo &, RwStream *);
void Reset(RwChunkHeaderInfo &, RwStream *);
void Init(RwChunkHeaderInfo &, RwStream *);
}

}  // namespace MainLoop
}  // namespace RWS

bool MainLoop_IsStarted();
int MainLoop_LogicIsEnabled();
int MainLoop_RenderIsEnabled();

#endif
