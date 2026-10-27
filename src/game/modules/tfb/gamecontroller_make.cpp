#include "modules/tfb/gamecontroller.h"
namespace RWS { namespace TFB {
// 0x403ed0
CAttributeHandler *(*g_keepMakeNew)(const CAttributePacket &) = &GameController::MakeNew;
}}
