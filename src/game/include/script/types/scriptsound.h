// Script::ScriptSound (PS2 RTTI; base ScriptReference), the sound script type, and Script::ScriptSoundObj, the script object of
// one CTFBSound. ScriptSound carries the Expose* members of the audio settings (effects / music volume, environment type and
// level, stream count) and of one playing sound (volume, pitch, percent complete, time remaining: those take a ScriptSoundObj
// as the owner). GetName 0x418ec0 is shared with the sound script object and the sound prototype (SoundProtoW, vtable 0x5d34fc:
// 0xc bytes, vptr + two zeroed words). No vtable of ScriptSound itself is identified, so its size is not asserted.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTSOUND_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTSOUND_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

class ScriptSound : public ScriptReference {
public:
	const char *GetName();                    // 0: 0x418ec0 (shared with ScriptSoundObj / SoundProtoW)
	// GLOBAL: 0x5d33d8 Script::ScriptSound::ExposeVolume::vftable
	class ExposeVolume : public Measurement { // "volume", vtable 0x5d33d8
	public:
		const char *GetName();                            // 0: 0x43dbe0
		int GetInt(ScriptReference *owner);               // 13: 0x4189b0
		void SetValue(int value, ScriptReference *owner); // 16: 0x4189d0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x42fbc0
	};
	// GLOBAL: 0x5d3428 Script::ScriptSound::ExposePitch::vftable
	class ExposePitch : public Measurement { // vtable 0x5d3428
	public:
		int GetInt(ScriptReference *owner);               // 13: 0x418a20
		void SetValue(int value, ScriptReference *owner); // 16: 0x418a40
		char const *GetName();                                  // 0: 0x43dbf0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b2b0
	};
	// GLOBAL: 0x5d3470 Script::ScriptSound::ExposePercentComplete::vftable
	class ExposePercentComplete : public Measurement { // "percent complete", vtable 0x5d3470
	public:
		int GetInt(ScriptReference *owner); // 13: 0x418750
		const char *GetName();                                  // 0: 0x438d30
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x42fbc0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d34b8 Script::ScriptSound::ExposeTimeRemaining::vftable
	class ExposeTimeRemaining : public Measurement { // "time remaining", vtable 0x5d34b8
	public:
		float GetFloat(ScriptReference *owner); // 14: 0x418860
		char const *GetName();                                  // 0: 0x438d40
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
	class ExposeStreamCount : public Measurement { // "active stream count", vtable 0x5d3530
	public:
		const char *GetName();                            // 0: 0x43dc90
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetKind();                                    // 6: 0x43e6f0
		int GetMax();                                     // 11: 0x42fbc0
		int GetInt(ScriptReference *owner);               // 13: 0x418600
		void SetValue(int value, ScriptReference *owner); // 16: 0x4025a0
	};
	class ExposeEffectsVolume : public Measurement { // "effects volume", vtable 0x5d3588
	public:
		ExposeEffectsVolume();                            // 0x43dca0
		const char *GetName();                            // 0: 0x43dd00
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetMax();                                     // 11: 0x42fbc0
		int GetInt(ScriptReference *owner);               // 13: 0x418610
		void SetValue(int value, ScriptReference *owner); // 16: 0x418620
	};
	class ExposeMusicVolume : public Measurement { // "music volume", vtable 0x5d35e0
	public:
		ExposeMusicVolume();                              // 0x43dd10
		const char *GetName();                            // 0: 0x43dd70
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetMax();                                     // 11: 0x42fbc0
		int GetInt(ScriptReference *owner);               // 13: 0x418650
		void SetValue(int value, ScriptReference *owner); // 16: 0x418660
	};
	class ExposeEnvironmentType : public Measurement { // "environment reverb type", vtable 0x5d3638
	public:
		ExposeEnvironmentType();                          // 0x43dd80
		const char *GetName();                            // 0: 0x43ddd0
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetInt(ScriptReference *owner);               // 13: 0x4186f0
		void SetValue(int value, ScriptReference *owner); // 16: 0x418700
	};
	class ExposeEnvironmentLevel : public Measurement { // "environment reverb level", vtable 0x5d3698
	public:
		ExposeEnvironmentLevel();                         // 0x43dde0
		const char *GetName();                            // 0: 0x43de30
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetMax();                                     // 11: 0x42fbc0
		int GetInt(ScriptReference *owner);               // 13: 0x418690
		void SetValue(int value, ScriptReference *owner); // 16: 0x4186b0
	};
};

} // namespace Script


namespace RWS { namespace TFB { class SoundLink; class CTFBSound; } }

namespace Script {

// The separately allocated script object behind CTFBSound (vtable 0x5cca48, 13 slots; allocated by CTFBSound's constructor 0x418ee0,
// 0xc bytes). Scripts reach the sound through it (Play, StopVoices) and its Expose members (ScriptSound::ExposeVolume ...).
class ScriptSoundObj : public ScriptReference {
public:
	RWS::TFB::SoundLink *m_link;  // the SoundLink of the CTFBSound that owns this object (CTFBSound + 0x1c); 0 once the sound is gone
	int m_refCount;
	__forceinline ScriptSoundObj() : m_link(0), m_refCount(0) {}   // inline in CTFBSound's constructor
	inline RWS::TFB::CTFBSound *GetSound();   // the sound this object belongs to (0 when there is none); defined in modules/tfb/ctfbsound.h
	int AdjustRefCount(int delta);          // 2: 0x418720 (the last reference deletes the object, except the type prototype)
	const char *GetName();                  // 0: 0x418ec0
	ScriptReference *GetType();             // 4: 0x418ed0
	ScriptReference *GetBaseType();         // 5: 0x418b70
	void *GetMembers(int *count);           // 7: 0x43e0a0
	virtual void Play(ScriptReference *ref); // 0x419150 (thiscall, ret 4) vtable slot 11: start a voice for ref unless the sound is "throw07_pressthecircle"
	virtual void StopVoices(ScriptReference *ref); // 0x419240 (thiscall, ret 4) vtable slot 12: stop this sound's voices for ref (0 = all)
};

}   // namespace Script

Script::ScriptReference *TFBType_Sound();   // 0x43db70: the prototype getter of the ScriptSound environment type

#endif
