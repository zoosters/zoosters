#ifndef ZOOSTERS_MODULES_FX_GAMEPARTICLE_H
#define ZOOSTERS_MODULES_FX_GAMEPARTICLE_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "script/types/scriptparticle.h"
#include "tfb/corpus/corpus.h"
#include "modules/fx/particles.h"

namespace RWS { namespace FX {

class CFXPartSpray;

// GLOBAL: 0x5cb420 RWS::FX::GameParticle::vftable
class GameParticle : public Script::ScriptParticle {
public:
	typedef Particle::ParticlePool<Particle::CPB_TexPart> Pool;

	explicit GameParticle(CFXPartSpray *owner);
	~GameParticle();                                              // slot 3

	// Overrides of the game vtable (the other slots are ScriptParticle's / Placement's)
	int AdjustRefCount(int delta);                                // slot 2, deletes the particle when the count reaches 0
	Vec3 *GetVector(Script::ScriptReference *owner);              // slot 17, position
	void SetVector(const Vec3 *v, Script::ScriptReference *owner);   // slot 18, position
	float GetFacing();                                            // slot 19
	bool SetFacing(float v, int flag);                            // slot 20
	float GetTilt();                                              // slot 21
	bool SetTilt(float v, Script::ScriptReference *owner);        // slot 22
	float GetRoll();                                              // slot 23
	bool SetRoll(float v, Script::ScriptReference *owner);        // slot 24
	void Remove();                                                // slot 27
	void Reset();                                                 // slot 28
	char Control(bool on);                                        // slot 29, true while the particle is its own spawn source or a clone of one
	intptr_t Spawn(intptr_t pos, int flag);                               // slot 30, PTR32: pos is a const RwV3d *, the result the new GameParticle *
	Script::ScriptReference *GetSpawnSource();                    // slot 31
	void SetAttachTarget(Script::ScriptReference *target);        // slot 39

	void ApplySettings();   // pushes the settings into the buffer's base processor
	float GetActivationDistance();

	// The spray mirrors its attributes into these; +0x54..+0xd3 are copied from the spray by ApplySettings and Spawn.
	Script::SetCursorTail m_unk40;                                   // +0x40 unused
	uint8_t m_visible;                                      // +0x4c rewritten by the visibility pass
	uint8_t m_pad4d[3];
	uint32_t m_poolCapacity;                                // +0x50 size of the particle buffer when activated
	float m_facing;                                         // +0x54 degrees
	float m_tilt;                                           // +0x58
	float m_roll;                                           // +0x5c
	uint32_t m_outputRate;                                  // +0x60
	uint32_t m_outputRateSpread;                            // +0x64
	float m_lifetime;                                       // +0x68 seconds
	float m_lifetimeSpread;                                 // +0x6c
	float m_activationRange;                                // +0x70 metres (x 12 = game units)
	float m_deactivationRange;                              // +0x74
	RwV3d m_emitterSpread;                                  // +0x78
	float m_angularSpreadX;                                 // +0x84 degrees
	float m_angularSpreadZ;                                 // +0x88
	float m_visualRadius;                                   // +0x8c
	uint32_t m_options;                                     // +0x90 bit 0: ignore cut scenes
	// Start (S) and end (E) state of the primary stage. Colours are RGBA bytes in channel order.
	float m_primarySVelocity;                               // +0x94
	float m_primarySVelocitySpread;                         // +0x98
	float m_primarySGravity;                                // +0x9c
	float m_primarySDrag;                                   // +0xa0
	union {
		RwRGBA m_primarySColor;                             // +0xa4
		int32_t m_primarySColorWord;
	};
	union {
		RwRGBA m_primarySColorSpread;                       // +0xa8
		int32_t m_primarySColorSpreadWord;
	};
	float m_primarySSize;                                   // +0xac
	float m_primarySSizeSpread;                             // +0xb0
	float m_primaryEVelocity;                               // +0xb4
	float m_primaryEVelocitySpread;                         // +0xb8
	float m_primaryEGravity;                                // +0xbc
	float m_primaryEDrag;                                   // +0xc0
	union {
		RwRGBA m_primaryEColor;                             // +0xc4
		int32_t m_primaryEColorWord;
	};
	union {
		RwRGBA m_primaryEColorSpread;                       // +0xc8
		int32_t m_primaryEColorSpreadWord;
	};
	float m_primaryESize;                                   // +0xcc
	float m_primaryESizeSpread;                             // +0xd0


	GameParticle *m_spawnSource;                                  // +0xd4 itself for the original, the original for a clone, 0 once removed
	int32_t m_refCount;                                           // +0xd8
	CFXPartSpray *m_owner;                                        // +0xdc the spray (cleared by its destructor)
	Pool *m_buf;                                                  // +0xe0 the particle buffer (owned)
	RwFrame *m_frame;                                             // +0xe4 the frame the particles are emitted from
	uint8_t m_ownsFrame;                                          // +0xe8 set when the spray / Spawn created m_frame itself (no atomic): the destructor frees it
	uint8_t m_pade9[3];
	CorpusLink m_link;                                            // +0xec the actor this particle rides on
};
ZOOSTERS_ASSERT_SIZE(GameParticle, 0x110);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_unk40, 0x40);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_visible, 0x4c);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_poolCapacity, 0x50);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_facing, 0x54);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_outputRate, 0x60);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_lifetime, 0x68);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_activationRange, 0x70);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_emitterSpread, 0x78);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_angularSpreadX, 0x84);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_visualRadius, 0x8c);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_options, 0x90);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_primarySVelocity, 0x94);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_primarySColor, 0xa4);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_primarySColorSpread, 0xa8);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_primarySSize, 0xac);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_primaryEVelocity, 0xb4);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_primaryEColor, 0xc4);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_primaryEColorSpread, 0xc8);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_primaryESize, 0xcc);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_primaryESizeSpread, 0xd0);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_spawnSource, 0xd4);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_refCount, 0xd8);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_owner, 0xdc);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_buf, 0xe0);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_frame, 0xe4);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_ownsFrame, 0xe8);
ZOOSTERS_ASSERT_OFFSET(GameParticle, m_link, 0xec);

void UpdateGameParticles();

}}

#endif
