// Script::ScriptVertAnim (PS2 RTTI): the vertex animation script object; only the member names are known so far.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTVERTANIM_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTVERTANIM_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

class ScriptVertAnim : public ScriptReference {
public:
	// The three members of the anim type's member table (SurfaceAnimW::GetMembers 0x43c220 = ScriptSurfaceAnim::GetMembers): names
	// 0x438d30 / 0x438d40 are the ones ScriptSound / ScriptController give their ExposePercentComplete / ExposeTimeRemaining.
	// GLOBAL: 0x5d3188 Script::ScriptVertAnim::ExposePercentComplete::vftable
	class ExposePercentComplete : public Measurement {
	public:
		const char *GetName();          // 0: 0x438d30 "percent complete"
	};
	// GLOBAL: 0x5d3140 Script::ScriptVertAnim::ExposeTimeRemaining::vftable
	class ExposeTimeRemaining : public Measurement {
	public:
		const char *GetName();          // 0: 0x438d40 "time remaining"
	};
	// GLOBAL: 0x5d30f8 Script::ScriptVertAnim::ExposePlaybackRate::vftable
	class ExposePlaybackRate : public Measurement {
	public:
		const char *GetName();          // 0: 0x43c290 "playback rate"
	};
};

}  // namespace Script

#endif
