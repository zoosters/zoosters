// The factory functions ("Create", 0x419f40, 0x419fa0) the compiler makes from RWS_MAKENEWCLASS; these tables keep them referenced.
#include "modules/tfb/levelhub.h"

namespace RWS { namespace TFB {

// 0x419f40
CAttributeHandler *(*g_keepMakeNew_levelhub)(const CAttributePacket &) = &LevelHub::MakeNew;
// 0x419fa0
CAttributeHandler *(*g_keepMakeNew_globalscriptnames)(const CAttributePacket &) = &GlobalScriptNames::MakeNew;

}}
