// The XDK pad API (xinput_emul.cpp): handles are port + 1, ports are the player input contexts, a pad "device" is that context's binding map.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_XINPUT_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_XINPUT_H

#include "framework/startup/win32/input/pads.h"

void XInitDevices(int preallocCount, int preallocTypes);
int XInputOpen(int deviceType, int port, int slot, int pollingParams);
void XInputGetCapabilities(int device, XInputCapabilities *caps);
int XGetDevices(int deviceType);
int XGetDeviceChanges(int deviceType, unsigned *insertions, unsigned *removals);
int XInputGetState(int device, PadState *state);
int XInputSetState(int device, int *feedback);   // feedback = &PadState::m_feedbackStatus

#endif
