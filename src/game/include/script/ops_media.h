// Script media ops. Address comments: the game function in that slot. Members past the base are sized from CloneToPool (m_XX = offset).
#ifndef ZOOSTERS_SCRIPT_OPS_MEDIA_H
#define ZOOSTERS_SCRIPT_OPS_MEDIA_H

#include "script/opcode.h"
#include "script/runtime.h"

namespace Script {


// vtable 0x5d3330, "play media", 12 B
class MediaPlay : public OpCode {
public:
	const char *GetName();                      // 0: 0x43da80
	const char *GetAltName();                   // 1: 0x43da90
	OpCode *Clone();                            // 12: 0x43de40
	int Execute();                              // 13: 0x43daa0
	int Parse();                                // 14: 0x43db40
	__declspec(noinline) static MediaPlay *CloneToPool(const MediaPlay *src);
	ScriptRef m_media;
};
ZOOSTERS_ASSERT_SIZE(MediaPlay, 0xc);

// vtable 0x5d3384, "stop media", 12 B
class MediaStop : public OpCode {
public:
	const char *GetName();                      // 0: 0x43dae0
	const char *GetAltName();                   // 1: 0x43daf0
	OpCode *Clone();                            // 12: 0x43de90
	int Execute();                              // 13: 0x43db00
	int Parse();                                // 14: 0x43db40
	__declspec(noinline) static MediaStop *CloneToPool(const MediaStop *src);
	ScriptRef m_media;
};
ZOOSTERS_ASSERT_SIZE(MediaStop, 0xc);

}  // namespace Script

// Script globals of the running actor context (0x621f14 current actor "self", 0x621f18 the other actor)
extern Script::ScriptReference *g_scriptSelf;    // 0x621f14
extern Script::ScriptReference *g_scriptOwner;   // 0x621f18

// Expected-type getter of the media Parse functions: a function-local static type prototype
Script::ScriptReference *TFBType_Media();

#endif
