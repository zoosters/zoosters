// Scalar deleting destructors (27 B: `~X; if (flags & 1) delete this; return this`) of script and particle classes in
// 0x429710..0x44d220 (family shape 294918).
// The classes are minimal stand-ins local to this file: the real hierarchies carry placeholder or partial destructors, so the
// compiler cannot emit the deleting destructor. Each declares the game's real `~X` (address in the GLOBAL line) and writes the
// deleting destructor by hand as a member. Class names without PS2 RTTI are made up.

#include "script/types/actorattachment.h"
#include "script/types/actorturret.h"
#include "script/types/scriptsets.h"
#include "script/types/scriptparticle.h"

void operator_delete(void *p);  // 0x5500f0

// GLOBAL: 0x43cfe0 Script::ScriptSubset::~ScriptSubset
// GLOBAL: 0x42be20 Script::ActorAttachment::AttacherSet::~AttacherSet
// GLOBAL: 0x42be70 Script::ScriptActor::~ScriptActor
// GLOBAL: 0x42ce50 Script::ActorTurret::~ActorTurret
// GLOBAL: 0x42bf20 Script::ActorAttachment::~ActorAttachment
// GLOBAL: 0x42da00 RWS::TFB::GameActor::~GameActor
// GLOBAL: 0x42ce40 Script::ActorTurretPrototype::~ActorTurretPrototype
// GLOBAL: 0x42f8a0 Script::ValueSlide::~ValueSlide
// GLOBAL: 0x435230 Script::ExposePlayerSet::~ExposePlayerSet
// GLOBAL: 0x43a640 Script::ExposeSendersSet::~ExposeSendersSet
// GLOBAL: 0x43b0b0 Script::ScriptParticle::ExposeClones::~ExposeClones
// GLOBAL: 0x43bf50 Script::UserReference::~UserReference
// GLOBAL: 0x43e2e0 Script::SpriteShieldSetMember::~SpriteShieldSetMember
// GLOBAL: 0x44f810 RWS::Particle::ParticleResourceHandler::~ParticleResourceHandler
// GLOBAL: 0x44baa0 RWS::Particle::CPB_BasePart::~CPB_BasePart

namespace Script {

// vtable 0x5cd234: real class Corpus in g/corpus.cpp (deleting dtor 0x429710, ~Corpus 0x426860).


// 0x42ce20
// (folded: vtables 0x5cd328 AttacherSet, 0x5cd380 ExposeWayPoints, 0x5cd3d8 ExposeAttachmentPoints, 0x5cd430 ExposeTurrets
// all hold this copy in slot 3, all with ~ 0x42be20)
ActorAttachment::AttacherSet *ActorAttachment::AttacherSet::scalar_deleting_dtor(unsigned flags)
{
	this->AttacherSet::~AttacherSet();
	if (flags & 1)
		operator_delete(this);
	return this;
}


// vtable 0x5cd480 ("turret", 35 slots, derives from ActorAttachment, owns a pointer member deleted in the dtor): ActorTurret (types/actorturret.h)

// 0x42d620
ActorTurret *ActorTurret::scalar_deleting_dtor(unsigned flags)
{
	this->ActorTurret::~ActorTurret();
	if (flags & 1)
		operator_delete(this);
	return this;
}


// vtable 0x5cdb30 ("turret", 35 slots; its ~ is the 5-byte `jmp 0x42bf20`, i.e. an empty destructor over ActorAttachment's): name made up
class ActorTurretPrototype {
public:
	~ActorTurretPrototype();
	ActorTurretPrototype *scalar_deleting_dtor(unsigned flags);
};

// 0x42df60
ActorTurretPrototype *ActorTurretPrototype::scalar_deleting_dtor(unsigned flags)
{
	this->ActorTurretPrototype::~ActorTurretPrototype();
	if (flags & 1)
		operator_delete(this);
	return this;
}


// 0x437cc0
// (folded: slot 3 of 0x5cef40 ("set", 35 slots), 0x5cf1e8 ScriptActor::ExposeClones, 0x5d0598 ExposePlayerSet, 0x5d05e8
// ExposeNPCsSet, 0x5d0638, 0x5d0690 ExposeDeadSet; all with ~ 0x435230)
ExposePlayerSet *ExposePlayerSet::scalar_deleting_dtor(unsigned flags)
{
	this->ExposePlayerSet::~ExposePlayerSet();
	if (flags & 1)
		operator_delete(this);
	return this;
}


// 0x43b710
// (folded: slot 3 of 0x5d20c8 ScriptParticle::ExposeClones and 0x5d2d50 / 0x5d2da0 / 0x5d2de8 ("active"/"inactive"/"removed"); ~ 0x43b0b0)
ScriptParticle::ExposeClones *ScriptParticle::ExposeClones::scalar_deleting_dtor(unsigned flags)
{
	this->ExposeClones::~ExposeClones();
	if (flags & 1)
		operator_delete(this);
	return this;
}


// vtables 0x5d3708 ("active") and 0x5d3750 ("removed"), 18 slots each, same destructor 0x43e2e0: name made up
class SpriteShieldSetMember {
public:
	~SpriteShieldSetMember();
	SpriteShieldSetMember *scalar_deleting_dtor(unsigned flags);
};

// 0x43e480
SpriteShieldSetMember *SpriteShieldSetMember::scalar_deleting_dtor(unsigned flags)
{
	this->SpriteShieldSetMember::~SpriteShieldSetMember();
	if (flags & 1)
		operator_delete(this);
	return this;
}

}  // namespace Script

namespace RWS {
namespace Particle {

// slot 5 of ~10 particle behaviour vtables (0x5d51b4, 0x5d5280, 0x5d52a8, ...); its ~ is the 5-byte `jmp CResourceHandler::~CResourceHandler`
class ParticleResourceHandler {
public:
	~ParticleResourceHandler();
	ParticleResourceHandler *scalar_deleting_dtor(unsigned flags);
};

// 0x44a820
ParticleResourceHandler *ParticleResourceHandler::scalar_deleting_dtor(unsigned flags)
{
	this->ParticleResourceHandler::~ParticleResourceHandler();
	if (flags & 1)
		operator_delete(this);
	return this;
}

}  // namespace Particle
}  // namespace RWS


// FOLDED: 0x42ce40 Script::ActorTurretPrototype::~ActorTurretPrototype
Script::ActorTurretPrototype::~ActorTurretPrototype() {}   // the game folded it into GnThis_429000_441000::AttachmentW3a_Dtor_thunk
// FOLDED: 0x44f810 RWS::Particle::ParticleResourceHandler::~ParticleResourceHandler
RWS::Particle::ParticleResourceHandler::~ParticleResourceHandler() {}   // the game folded it into GnThis_rest::CResourceHandler_Dtor_thunk
// FOLDED: 0x43e2e0 Script::SpriteShieldSetMember::~SpriteShieldSetMember
Script::SpriteShieldSetMember::~SpriteShieldSetMember() {}   // the game folded it into Script::RefSetDtor43e::~RefSetDtor43e
