#include "modules/fx/partspray.h"
namespace RWS { namespace FX {
// 0x4101b0
CAttributeHandler *(*g_keepMakeNew_CFXPartSpray)(const CAttributePacket &) = &CFXPartSpray::MakeNew;
}}
