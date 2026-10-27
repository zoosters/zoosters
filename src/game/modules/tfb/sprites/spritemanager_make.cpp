#include "modules/tfb/sprites/spritemanager.h"
namespace RWS { namespace TFB {
// 0x4175a0
CAttributeHandler *(*g_keepMakeNew_spritemanager)(const CAttributePacket &) = &SpriteManager::MakeNew;
}}
