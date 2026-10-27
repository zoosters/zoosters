#define _ALLOW_KEYWORD_MACROS   // (modern STL headers refuse a keyword macro otherwise)
#define protected public
#include "modules/fx/partspray.h"
#include "modules/fx/particles.h"
#include "framework/toolkits/world/framehelper.h"

namespace RWS { namespace MainLoop { namespace Logic { extern unsigned Rate; } } }   // GLOBAL: 0x606c3c RWS::MainLoop::Logic::Rate
RwFrame *RwFrameCreate(void);

// GLOBAL: 0x44eff0 ?RwFrameCreate_thunk@FX@RWS@@YAPAURwFrame@@XZ
// GLOBAL: 0x40cad0 ??4CPB_TexPart@Particle@RWS@@QAEAAV012@ABV012@@Z

namespace RWS { namespace FX {

// 0x40d3e0
void GameParticle::Reset()
{
	if (!TFB_GetActiveParticleSet()->Remove(this)) {
		if (!TFB_GetRemovedParticleSet()->Remove(this))
			return;
		m_spawnSource = this;
	}
	m_link.Release();
	m_buf->Resize(0, 0);
	GameParticle *src = m_spawnSource;
	if (src != this) {
		if (src->m_20.Remove(this))
			src->AdjustRefCount(-1);
	} else {
		TFB_GetInactiveParticleSet()->Insert(this, 3.4028235e38f);
		ApplySettings();
	}
}

// 0x40d470
void GameParticle::Remove()
{
	if (!TFB_GetActiveParticleSet()->Remove(this)) {
		if (!TFB_GetInactiveParticleSet()->Remove(this))
			return;
	}
	m_link.Release();
	m_buf->Resize(0, 0);
	GameParticle *src = m_spawnSource;
	if (src != this) {
		if (src->m_20.Remove(this))
			src->AdjustRefCount(-1);
	} else {
		m_spawnSource = 0;
		TFB_GetRemovedParticleSet()->Insert(this, 3.4028235e38f);
		ApplySettings();
	}
}

// 0x40d120
void GameParticle::ApplySettings()
{
	m_visible = 1;
	memcpy(&m_facing, &m_owner->m_facing, 0x80);
	TexPartBuffer *buf = m_buf;
	Particle::CPB_TexPart *p = buf->Processor();
	*p = *p;
	buf->Processor()->ForgetTables();
	p->SetRate(m_outputRate);
	p->SetRateJitter(m_outputRateSpread);
	p->SetLife(static_cast<RwUInt32>(RWS::MainLoop::Logic::Rate * m_lifetime));
	p->SetLifeJitter(static_cast<RwUInt32>(RWS::MainLoop::Logic::Rate * m_lifetimeSpread));
	p->SetBoxX(m_emitterSpread.x);
	p->SetBoxY(m_emitterSpread.y);
	p->SetBoxZ(m_emitterSpread.z);
	p->SetTiltX(m_angularSpreadX);
	p->SetTiltZ(m_angularSpreadZ);
	p->SetSpeed(Particle::CPB_BasePart::STAGE_FROM, m_primarySVelocity);
	p->SetSpeedJitter(Particle::CPB_BasePart::STAGE_FROM, m_primarySVelocitySpread);
	p->SetFall(Particle::CPB_BasePart::STAGE_FROM, m_primarySGravity);
	p->SetResistance(Particle::CPB_BasePart::STAGE_FROM, m_primarySDrag);
	RwRGBA col;
	col.red = m_primarySColor.red;
	col.green = m_primarySColor.green;
	col.blue = m_primarySColor.blue;
	col.alpha = m_primarySColor.alpha;
	p->SetColor(Particle::CPB_BasePart::STAGE_FROM, col);
	// NOTE: the game gives all four channels the red spread (the green / blue / alpha spreads are never applied); kept.
	p->SetRedJitter(Particle::CPB_BasePart::STAGE_FROM, m_primarySColorSpread.red);
	p->SetGreenJitter(Particle::CPB_BasePart::STAGE_FROM, m_primarySColorSpread.red);
	p->SetBlueJitter(Particle::CPB_BasePart::STAGE_FROM, m_primarySColorSpread.red);
	p->SetAlphaJitter(Particle::CPB_BasePart::STAGE_FROM, m_primarySColorSpread.red);
	p->SetExtent(Particle::CPB_BasePart::STAGE_FROM, m_primarySSize);
	p->SetExtentJitter(Particle::CPB_BasePart::STAGE_FROM, m_primarySSizeSpread);
	p->SetSpeed(Particle::CPB_BasePart::STAGE_TO, m_primaryEVelocity);
	p->SetSpeedJitter(Particle::CPB_BasePart::STAGE_TO, m_primaryEVelocitySpread);
	p->SetFall(Particle::CPB_BasePart::STAGE_TO, m_primaryEGravity);
	p->SetResistance(Particle::CPB_BasePart::STAGE_TO, m_primaryEDrag);
	RwRGBA peCol;
	peCol.red = m_primaryEColor.red;
	peCol.green = m_primaryEColor.green;
	peCol.blue = m_primaryEColor.blue;
	peCol.alpha = m_primaryEColor.alpha;
	p->SetColor(Particle::CPB_BasePart::STAGE_TO, peCol);
	p->SetRedJitter(Particle::CPB_BasePart::STAGE_TO, m_primaryEColorSpread.red);
	p->SetGreenJitter(Particle::CPB_BasePart::STAGE_TO, m_primaryEColorSpread.red);
	p->SetBlueJitter(Particle::CPB_BasePart::STAGE_TO, m_primaryEColorSpread.red);
	p->SetAlphaJitter(Particle::CPB_BasePart::STAGE_TO, m_primaryEColorSpread.red);
	p->SetExtent(Particle::CPB_BasePart::STAGE_TO, m_primaryESize);
	p->SetExtentJitter(Particle::CPB_BasePart::STAGE_TO, m_primaryESizeSpread);
}

// 0x40d500
intptr_t GameParticle::Spawn(intptr_t pos, int flag)
{
	GameParticle *clone = new GameParticle(m_owner);
	if (clone) {
		clone->m_poolCapacity = m_poolCapacity;
		memcpy(&clone->m_facing, &m_facing, 0x80);
		clone->m_spawnSource = this;
		GetName();
		clone->m_ownsFrame = 1;
		clone->m_frame = RwFrameCreate_thunk();
		RwFrameSetIdentity(clone->m_frame);
		RwV3d t = *(const RwV3d *)pos;
		RwFrameTranslate(clone->m_frame, &t, rwCOMBINEPOSTCONCAT);
		clone->m_buf->m_drawMode = m_buf->m_drawMode;
		clone->m_buf->m_blendSrc = m_buf->m_blendSrc;
		clone->m_buf->m_blendDst = m_buf->m_blendDst;
		clone->m_buf->m_stencilWrite = m_buf->m_stencilWrite;
		TexPartBuffer *cloneBuf = clone->m_buf;
		cloneBuf->Processor()->operator=(*m_buf->Processor());
		cloneBuf->Processor()->ForgetTables();
		clone->m_buf->Resize(clone->m_poolCapacity, 0);
		TFB_GetActiveParticleSet()->Insert(clone, 3.4028235e38f);
	}
	return (intptr_t)clone;
}
}}
