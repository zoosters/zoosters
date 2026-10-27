#include "modules/tfb/memcard.h"
// 0x447580
RWS::CAttributeHandler *(*g_keepMakeNew_MemCard)(const RWS::CAttributePacket &) = &MemCard::MakeNew;
