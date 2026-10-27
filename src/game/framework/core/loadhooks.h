// TFB's additions to stream loading: the loading-bar update after each chunk or upload, and the pump before each chunk.
#ifndef ZOOSTERS_FRAMEWORK_CORE_LOADHOOKS_H
#define ZOOSTERS_FRAMEWORK_CORE_LOADHOOKS_H

void TFBLoadingProgress();
// __stdcall: the platform layer sets it.
extern void(__stdcall *TFBStreamPump)(int);

#endif
