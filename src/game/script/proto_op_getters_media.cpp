// Prototype getters of ops_media.h classes; see proto_op_getters.cpp.
#include "script/ops_media.h"
using namespace Script;

// GLOBAL: 0x5d3330 Script::MediaPlay::vftable
// 0x43dc00
ScriptReference *OpProto_MediaPlay()
{
	static MediaPlay s;
	return &s;
}

// GLOBAL: 0x5d3384 Script::MediaStop::vftable
// 0x43dc40
ScriptReference *OpProto_MediaStop()
{
	static MediaStop s;
	return &s;
}
