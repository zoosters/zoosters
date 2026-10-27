// Stand-in for TFB::CTFBCommand (PS2: base at +52 of CFXColorLight): on PC a plain object with one pointer and no vtable.
// Its constructor (0x413360, takes the attribute packet) is called by CFXColorLight's and CFXPartSpray's.
#ifndef ZOOSTERS_MODULES_TFB_CTFBCOMMANDSTUB_H
#define ZOOSTERS_MODULES_TFB_CTFBCOMMANDSTUB_H
#include "framework/core/cattributehandler.h"
namespace RWS { namespace TFB {
class CTFBCommand {
public:
	CTFBCommand(const CAttributePacket &attr) : m_0(0) { HandleAttributes(attr); }
	void HandleAttributes(const CAttributePacket &attr);
	const char *m_0;   // the interned command name
};
}}
#endif
