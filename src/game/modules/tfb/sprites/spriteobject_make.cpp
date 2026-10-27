#include "modules/tfb/sprites/spriteobject.h"
namespace RWS { namespace TFB {
// 0x417610
CAttributeHandler *(*g_keepMakeNew_spriteobject)(const CAttributePacket &) = &SpriteObject::MakeNew;
}}
