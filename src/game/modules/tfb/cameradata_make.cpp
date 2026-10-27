#include "modules/tfb/cameradata.h"
namespace RWS { namespace TFB {
// 0x41def0
CAttributeHandler *(*g_keepMakeNew_cameradata)(const CAttributePacket &) = &CameraData::MakeNew;
}}
