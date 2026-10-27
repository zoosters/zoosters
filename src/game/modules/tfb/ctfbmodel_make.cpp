#include "modules/tfb/ctfbmodel.h"
// 0x445620
RWS::CAttributeHandler *(*g_keepMakeNew_CTFBModel)(const RWS::CAttributePacket &) = &CTFBModel::MakeNew;
