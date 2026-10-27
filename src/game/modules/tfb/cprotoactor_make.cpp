#include "modules/tfb/cprotoactor.h"
namespace RWS { namespace TFB {
// 0x42f1c0
CAttributeHandler *(*g_keepMakeNew_CProtoActor)(const CAttributePacket &) = &CProtoActor::MakeNew;
}}
