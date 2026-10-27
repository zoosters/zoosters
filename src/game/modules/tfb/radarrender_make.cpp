#include "modules/tfb/radarrender.h"
// 0x4428d0
RWS::CAttributeHandler *(*g_keepMakeNew_RadarRender)(const RWS::CAttributePacket &) = &RadarRender::MakeNew;
