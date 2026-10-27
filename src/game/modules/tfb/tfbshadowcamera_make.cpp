#include "modules/tfb/tfbshadowcamera.h"
namespace RWS { namespace TFB {
// 0x422750
CAttributeHandler *(*g_keepMakeNew_TFBShadowCamera)(const CAttributePacket &) = &TFBShadowCamera::MakeNew;
}}
