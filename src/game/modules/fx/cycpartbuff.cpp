// Explicit instantiation emits every member out of line, as the game has them.
#include "modules/fx/particles.h"
#include "framework/toolkits/world/framehelper.h"
#include "tfb/render/tfbatomic.h"

namespace RWS { namespace Particle {

// One PTank atomic of `capacity` particles; every particle size zeroed (= not live).
// 0x40a340
template <class T> ParticlePool<T>::Tank::Tank(RwUInt32 capacity, RwUInt32 dataFlags, RwUInt32 ordinal)
{
	RpPTankLockStruct sizes;
	m_atomic = RpPTankAtomicCreate(capacity, dataFlags | rpPTANKDFLAGSIZE, 0);
	TFBPlugin_FD_SetFlagBits(m_atomic, 0x3000);
	RwFrame *frame = RwFrameCreate_thunk();
	RwUInt32 frameFlags = frame->modelling.flags | 0x20003;
	// the identity matrix, stored in the order the game does
	frame->modelling.at.z = 1.0f;
	frame->modelling.up.y = 1.0f;
	frame->modelling.right.x = 1.0f;
	frame->modelling.up.x = 0.0f;
	frame->modelling.right.z = 0.0f;
	frame->modelling.right.y = 0.0f;
	frame->modelling.at.y = 0.0f;
	frame->modelling.at.x = 0.0f;
	frame->modelling.up.z = 0.0f;
	frame->modelling.pos.z = 0.0f;
	frame->modelling.pos.y = 0.0f;
	frame->modelling.pos.x = 0.0f;
	frame->modelling.flags = frameFlags;
	RpAtomicSetFrame(m_atomic, frame);
	RpPTankExtOf(m_atomic)->instFlags |= 0x800000;
	RpPTankExtView *ext = RpPTankExtOf(m_atomic);
	ext->actPCount = ext->maxPCount;
	m_live = static_cast<bool *>(MemAllocHint(capacity, kParticleMemType));
	memset(m_live, 0, capacity);
	m_ages = static_cast<RwUInt32 *>(MemAllocHint(capacity * 4, kParticleMemType));
	RpPTankAtomicLock(m_atomic, &sizes, rpPTANKLFLAGSIZE, rpPTANKLOCKWRITE);
	for (RwUInt32 i = 0; i < capacity; i++) {
		int *entry = reinterpret_cast<int *>(sizes.data + sizes.stride * i);
		entry[0] = 0;
		entry[1] = 0;
	}
	RpPTankAtomicUnlock(m_atomic);
	m_capacity = capacity;
	m_liveCount = 0;
	m_pins = 0;
	m_ordinal = ordinal;
}

template class ParticlePool<CPB_TexPart>;

// 0x40a460
// 0x40a490
// 0x40ada0
// 0x40adb0
// 0x40adc0
// 0x40add0
// 0x40b0a0
// 0x40b130
// 0x40b1b0
// 0x40b240
// 0x40ade0
// 0x40c030
// 0x40cee0
// 0x40c9a0
// 0x40c100
// 0x40c530
// 0x44cf60
// 0x44d060

// 0x44d240
void CPB_TexPart::SetTexture(RwTexture *texture, ParticlePool<CPB_TexPart> &pool)
{
	if (texture != m_texture) {
		pool.ForEachTank([texture](TankView &view) {
			RpMaterialSetTexture(RpGeometryGetMaterial(RpAtomicGetGeometry(view.m_atomic), 0), texture);
		});
		if (m_texture)
			RwTextureDestroy(m_texture);
		m_texture = texture;
	}
}

// Implicit assignment operators, used by GameParticle::Spawn to copy the template particle.
// 0x40cad0
CPB_TexPart &(CPB_TexPart::*g_keepTexPartAssign)(const CPB_TexPart &) = &CPB_TexPart::operator=;
// 0x40ae40
CPB_BasePart &(CPB_BasePart::*g_keepBasePartAssign)(const CPB_BasePart &) = &CPB_BasePart::operator=;
}}

// Forwarding thunks: ecx (this) passes through the jmp.
struct GnThis_rest {
	ZOOSTERS_NOINLINE void FUN_00424860();
	void CorpusLink_Release_thunk();
};

#include "tfb/corpus/corpus.h"
void GnThis_rest::FUN_00424860() { ((CorpusLink *)this)->Release(); }
// 0x40a4b0
void GnThis_rest::CorpusLink_Release_thunk()
{
	FUN_00424860();
}
