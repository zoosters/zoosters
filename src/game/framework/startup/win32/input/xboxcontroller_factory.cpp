// RWS_MAKENEWCLASS factory of CXboxStndController, in its own unit so the constructor is a call.
#include "framework/startup/win32/input/xboxcontroller.h"
// 0x4063a0
RWS::CAttributeHandler *(*g_keepMakeNew_CXboxStndController)(const RWS::CAttributePacket &) = &CXboxStndController::MakeNew;
