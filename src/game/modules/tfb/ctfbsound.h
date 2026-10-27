// Console RTTI: CAttributeHandler@0, LinearAllocationPolicy@28 and Script::ScriptBuddy@32; on PC the ScriptBuddy is a separately allocated
// script object at +0x1c (SoundLink).
#ifndef ZOOSTERS_MODULES_TFB_CTFBSOUND_H
#define ZOOSTERS_MODULES_TFB_CTFBSOUND_H

#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "rw/rwaudio.h"
#include "script/types/scriptsound.h"

namespace TFB { class KFSet; }

namespace RWS { namespace TFB {

struct Voice;

// Non-polymorphic base at +0x1c (4 B): its inline constructor allocates the script object (the game stores the CTFBSound vtable after the
// allocation; same as CameraData's CameraLink).
class SoundLink {
public:
	__forceinline SoundLink();    // defined in modules/tfb/ctfbsound.cpp (inlined into CTFBSound's constructor)
protected:
	::Script::ScriptSoundObj *m_obj;                     // +0x1c: the script object (attribute command 0 re-links it)
};

class CTFBSound : public CAttributeHandler, public SoundLink, public LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(CTFBSound);
	CTFBSound(const CAttributePacket &attr);
	~CTFBSound();
	virtual void HandleAttributes(const CAttributePacket &attr);
	// This sound's voice for `ref` when it loops or does not allow several instances, else `new Voice(this, ref)` (the constructor starts it)
	Voice *FindOrCreateVoice(::Script::ScriptReference *ref);

	// Attribute commands of class "CTFBSound" (jump table 0x419738, ids 0..16):
	const char *m_name;                                  // +0x20 cmd 0: pooled lower-case sound name (the wave's or the stream segment's name)
	const char *m_subtitle;                              // +0x24 cmd 16: localised string by index (0x44f580); shown (0x414720) while a voice plays
	RwaWave *m_wave;                                     // +0x28 wave found by name (RwsAudio::FindWave 0x44afb0); not for streamed sounds
	RwaWave *m_waveCopy;                                 // +0x2c an own wave, destroyed with the sound; nothing in the game sets it
	unsigned char m_resolved;                            // +0x30 the name has been resolved (script object link, wave lookup)
	char m_pad31[3];
	int m_volumePct, m_pitchPct;                         // +0x34, +0x38 cmd 2, 3 (default 100; x 0.01 in Voice::RandomizeAndSync)
	float m_maxDistance;                                 // +0x3c cmd 4: value x 12 (feet -> inches); the distance range of 3D sounds
	float m_pan;                                         // +0x40 cmd 5: int x 0.01; fixed pan (Voice::Update, Voice::Play)
	unsigned char m_streamed;                            // +0x44 cmd 6: plays through a mixer stream slot instead of a wave
	unsigned char m_loop;                                // +0x45 cmd 7: loops (the stream segment, the voice's loop flag)
	unsigned char m_music;                               // +0x46 cmd 14: volume follows the music setting (else effects); no pan or positioning
	// +0x47 cmd 8: 3D sound. The voice follows its target and its volume falls off with the distance (1 .. m_maxDistance for samples,
	// Voice::Update's range^2 curve
	// for streams); selects the four-channel gain layout of SetStreamGains
	unsigned char m_is3D;
	unsigned char m_doppler;                             // +0x48 cmd 9: the target's velocity is pushed too (Voice::Update3DPosition)
	unsigned char m_positional;                          // +0x49 cmd 10: pan from the direction of the target (Voice::Update)
	unsigned char m_multiInstance;                       // +0x4a cmd 13: every play starts a new voice (FindOrCreateVoice)
	char m_pad4b;
	int m_pitchVarPct, m_volumeVarPct;                   // +0x4c, +0x50 cmd 11, 12: random spread in percent (Voice::RandomizeAndSync)
	::TFB::KFSet *m_mouthAnim;                           // +0x54 resource of the system class's command 0 (the mouth animation played on the target's model)
};
ZOOSTERS_ASSERT_SIZE(CTFBSound, 0x58);
ZOOSTERS_ASSERT_OFFSET(CTFBSound, m_name, 0x20);
ZOOSTERS_ASSERT_OFFSET(CTFBSound, m_volumePct, 0x34);
ZOOSTERS_ASSERT_OFFSET(CTFBSound, m_streamed, 0x44);
ZOOSTERS_ASSERT_OFFSET(CTFBSound, m_mouthAnim, 0x54);

}}   // namespace RWS::TFB

// The SoundLink sits at +0x1c of its CTFBSound: stepping back from it reaches the sound (null stays null).
inline RWS::TFB::CTFBSound *Script::ScriptSoundObj::GetSound()
{
	return static_cast<RWS::TFB::CTFBSound *>(m_link);
}

#endif
