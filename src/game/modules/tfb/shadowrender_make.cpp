#include "modules/tfb/shadowrender.h"
// 0x4444e0
RWS::CAttributeHandler *(*g_keepMakeNew_ShadowRender)(const RWS::CAttributePacket &) = &ShadowRender::MakeNew;
