#include "modules/tfb/ctfbsound.h"
namespace RWS { namespace TFB {
// 0x419780
CAttributeHandler *(*g_keepMakeNew_ctfbsound)(const CAttributePacket &) = &CTFBSound::MakeNew;
}}
