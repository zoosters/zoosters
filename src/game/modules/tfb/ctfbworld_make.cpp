// The factory function ("Create", 0x41c370) the compiler makes from RWS_MAKENEWCLASS; this pointer keeps it referenced.
#include "modules/tfb/ctfbworld.h"

namespace RWS { namespace TFB {

// 0x41c370
CAttributeHandler *(*g_keepMakeNew_CTFBWorld)(const CAttributePacket &) = &CTFBWorld::MakeNew;

}}
