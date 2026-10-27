#include "script/ops_media.h"
#include "script/ops_camera.h"
#include "script/types/scriptsound.h"

// GLOBAL: 0x621f18 g_scriptOwner

namespace Script {

// 0x43da80
const char *MediaPlay::GetName()
{
	return "play media";
}

// 0x43da90
const char *MediaPlay::GetAltName()
{
	return "play sound";
}

// 0x43de40
OpCode *MediaPlay::Clone()
{
	return CloneToPool(this);
}

// 0x43de50
MediaPlay *MediaPlay::CloneToPool(const MediaPlay *src)
{
	void *mem = g_scriptPool;
	MediaPlay *r = new (mem) MediaPlay(*src);
	g_scriptPool += sizeof(MediaPlay);
	return r;
}

// 0x43dae0
const char *MediaStop::GetName()
{
	return "stop media";
}

// 0x43daf0
const char *MediaStop::GetAltName()
{
	return "stop sound";
}

// 0x43de90
OpCode *MediaStop::Clone()
{
	return CloneToPool(this);
}

// 0x43dea0
MediaStop *MediaStop::CloneToPool(const MediaStop *src)
{
	void *mem = g_scriptPool;
	MediaStop *r = new (mem) MediaStop(*src);
	g_scriptPool += sizeof(MediaStop);
	return r;
}

// 0x43daa0
int MediaPlay::Execute()
{
	ScriptReference *owner;
	ScriptReference *set;
	ScriptSoundObj *media = (ScriptSoundObj *)m_media.Resolve(&owner, &set, true);
	if (media != 0)
		media->Play(g_scriptOwner);
	return 1;
}

// 0x43db00
int MediaStop::Execute()
{
	ScriptReference *owner;
	ScriptReference *set;
	ScriptSoundObj *media = (ScriptSoundObj *)m_media.Resolve(&owner, &set, true);
	if (media != 0)
		media->StopVoices(g_scriptSelf);
	return 1;
}

// 0x43db40
// Shared with MediaStop::Parse: the game's linker folded the two into one body (the ops share OpCode and the media reference at +8),
// so the body is written once over the op and its reference and both members call it.
static int ParseMediaOp(OpCode *op, ScriptRef *media, int size)
{
	op->GetName();
	ScriptRefReader refs(TFBType_Media());
	refs.Read(media);
	return size;
}
int MediaPlay::Parse() { return ParseMediaOp(this, &m_media, sizeof(MediaPlay)); }
// FOLDED: 0x43db40 Script::MediaStop::Parse
int MediaStop::Parse() { return ParseMediaOp(this, &m_media, sizeof(MediaStop)); }

// Defined here, not in ops_camera.cpp (see there): a constructor in another translation unit keeps CloneToPool's
// EH frame.
// 0x438f40
CameraUse::CameraUse(const CameraUse &src)
	: OpCode(src), m_camera(src.m_camera), m_type(src.m_type), m_transition(src.m_transition), m_param(src.m_param), m_transitionParam(src.m_transitionParam)
{
}

}  // namespace Script
