#include "modules/tfb/worldobject.h"
// 0x41d370
RWS::CAttributeHandler *(*g_keepMakeNew_WorldObject)(const RWS::CAttributePacket &) = &WorldObject::MakeNew;
