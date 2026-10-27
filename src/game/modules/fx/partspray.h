#ifndef ZOOSTERS_MODULES_FX_PARTSPRAY_H
#define ZOOSTERS_MODULES_FX_PARTSPRAY_H
#include "script/referenceset.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "framework/toolkits/world/smartptrs/catomicptr.h"
#include "framework/toolkits/math/maths.h"
#include "modules/tfb/ctfbcommandstub.h"
#include "modules/fx/particles.h"
#include "modules/fx/gameparticle.h"
namespace RWS { namespace FX {

typedef GameParticle::Pool TexPartBuffer;

// CAtomicPtr plus its inline system-command handler (out-of-line copy 0x406f20).
class SprayAtomicPtr : public CAtomicPtr {
public:
};

// Function-local statics of Script::_ParticleShield::Expose{Active,Inactive,Removed}Set; members are GameParticles. Active ones tick,
// inactive ones wait for a player to come near.
int FUN_0043e6f0(const char *msg);

// The particle emitter behaviour. PS2 base order: AE
class CFXPartSpray : public CSystemCommands, public CAttributeHandler, public CEventHandler, public LinearAllocationPolicy, public TFB::CTFBCommand {
public:
	RWS_MAKENEWCLASS(CFXPartSpray);
	CFXPartSpray(const CAttributePacket &packet);
	~CFXPartSpray();
	virtual void HandleEvents(CMsg &msg);
	virtual void HandleAttributes(const CAttributePacket &packet);
	RwTexture *GetTexture(const char *name);
	static RpMaterial *MatchTextureName(RpMaterial *material, void *data);
	friend class GameParticle;                         // the constructor and ApplySettings read the block m_facing..m_toSizeJitter
	friend struct CFXPartSprayLayout;                  // MSVC checks access in offsetof
protected:
	GameParticle *m_host;                              // 0x2c
	SprayAtomicPtr m_model;                            // 0x30
	float m_offsetHeading;                             // 0x34
	float m_offsetDeflection;                          // 0x38
	CEventId m_frameEvent;                             // 0x3c
	FrameTrans m_placement;                               // 0x44 (0x74 bytes)
	CEventId m_rateEvent;                              // 0xb8
	CEventId m_enableEvent;                            // 0xc0
	unsigned short m_drawOrder;                        // 0xc8
	// mirrored into GameParticle (+0x54..+0xd3) as one block
	float m_facing;                                    // 0xcc
	float m_tilt;                                      // 0xd0
	float m_roll;                                      // 0xd4
	unsigned m_emitRate;                               // 0xd8
	unsigned m_emitRateJitter;                         // 0xdc
	float m_life;                                      // 0xe0
	float m_lifeJitter;                                // 0xe4
	float m_activationRange;                           // 0xe8
	float m_deactivationRange;                         // 0xec
	RwV3d m_emitBox;                                   // 0xf0
	float m_tiltRangeX;                                // 0xfc
	float m_tiltRangeZ;                                // 0x100
	float m_visualRadius;                              // 0x104 (20.0f at construction; copied to GameParticle::m_visualRadius)
	unsigned m_options;                                // 0x108 (bit 0: ignore cut scenes, as GameParticle::m_options)
	float m_fromSpeed;                                 // 0x10c
	float m_fromSpeedJitter;                           // 0x110
	float m_fromGravity;                               // 0x114
	float m_fromDrag;                                  // 0x118
	RwRGBA m_fromColour;                               // 0x11c
	unsigned char m_fromRedJitter;                     // 0x120
	unsigned char m_fromGreenJitter;
	unsigned char m_fromBlueJitter;
	unsigned char m_fromAlphaJitter;
	float m_fromSize;                                  // 0x124
	float m_fromSizeJitter;                            // 0x128
	float m_toSpeed;                                   // 0x12c
	float m_toSpeedJitter;                             // 0x130
	float m_toGravity;                                 // 0x134
	float m_toDrag;                                    // 0x138
	RwRGBA m_toColour;                                 // 0x13c
	unsigned char m_toRedJitter;                       // 0x140
	unsigned char m_toGreenJitter;
	unsigned char m_toBlueJitter;
	unsigned char m_toAlphaJitter;
	float m_toSize;                                    // 0x144
	float m_toSizeJitter;                              // 0x148
};
ZOOSTERS_ASSERT_SIZE(CFXPartSpray, 0x14c);
struct CFXPartSprayLayout {
	ZOOSTERS_ASSERT_OFFSET(CFXPartSpray, m_host, 0x2c);
	ZOOSTERS_ASSERT_OFFSET(CFXPartSpray, m_frameEvent, 0x3c);
	ZOOSTERS_ASSERT_OFFSET(CFXPartSpray, m_placement, 0x44);
	ZOOSTERS_ASSERT_OFFSET(CFXPartSpray, m_drawOrder, 0xc8);
	ZOOSTERS_ASSERT_OFFSET(CFXPartSpray, m_facing, 0xcc);
	ZOOSTERS_ASSERT_OFFSET(CFXPartSpray, m_options, 0x108);
	ZOOSTERS_ASSERT_OFFSET(CFXPartSpray, m_toSizeJitter, 0x148);
};
}}
#endif
