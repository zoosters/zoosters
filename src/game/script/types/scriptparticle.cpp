// Script::ScriptParticle member accessors (the Expose* classes, 0x40aa70-0x40c030): the getters read the emitter settings, the setters
// store the value in the settings and push it into the live particle buffer's base processor (CPB_BasePart), so a script can change a
// running emitter. A colour setter stores the RGBA word (or one channel) and re-sends the whole colour with its channels.
#include "script/membertable.h"
#include "modules/fx/gameparticle.h"
#include "script/types/scriptparticle.h"
#include "zoosters/memberrow.h"

namespace RWS { namespace MainLoop { namespace Logic { extern unsigned Rate; } } }   // GLOBAL: 0x606c3c RWS::MainLoop::Logic::Rate

namespace Script {

typedef RWS::FX::GameParticle GameParticle;
typedef RWS::Particle::CPB_TexPart CPB_TexPart;
static const RWS::Particle::CPB_BasePart::Stage STAGE_FROM = RWS::Particle::CPB_BasePart::STAGE_FROM;
static const RWS::Particle::CPB_BasePart::Stage STAGE_TO = RWS::Particle::CPB_BasePart::STAGE_TO;

static GameParticle *Emitter(ScriptReference *owner)
{
	return static_cast<GameParticle *>(owner);
}

// 0x40aa70
int ScriptParticle::ExposeOutputRate::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_outputRate;
}

// 0x40aa80
int ScriptParticle::ExposeOutputRateSpread::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_outputRateSpread;
}

// 0x40aa90
float ScriptParticle::ExposeLifetime::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_lifetime;
}

// 0x40aaa0
float ScriptParticle::ExposeLifetimeSpread::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_lifetimeSpread;
}

// 0x40aab0
float ScriptParticle::ExposeActivationRange::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_activationRange;
}

// 0x40aac0
void ScriptParticle::ExposeActivationRange::SetValue(float value, ScriptReference *owner)
{
	Emitter(owner)->m_activationRange = value;
}

// 0x40aad0
float ScriptParticle::ExposeDeactivationRange::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_deactivationRange;
}

// 0x40aae0
void ScriptParticle::ExposeDeactivationRange::SetValue(float value, ScriptReference *owner)
{
	Emitter(owner)->m_deactivationRange = value;
}

// 0x40aaf0
float ScriptParticle::ExposePrimarySVelocity::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySVelocity;
}

// 0x40ab00
float ScriptParticle::ExposePrimarySVelocitySpread::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySVelocitySpread;
}

// 0x40ab10
float ScriptParticle::ExposePrimarySGravity::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySGravity;
}

// 0x40ab20
float ScriptParticle::ExposePrimarySDrag::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySDrag;
}

// 0x40ab30
int ScriptParticle::ExposePrimarySColor::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySColorWord;
}

// 0x40ab40
int ScriptParticle::ExposePrimarySColor::ExposeR::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySColor.red;
}

// 0x40ab50
int ScriptParticle::ExposePrimarySColor::ExposeG::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySColor.green;
}

// 0x40ab60
int ScriptParticle::ExposePrimarySColor::ExposeB::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySColor.blue;
}

// 0x40ab70
int ScriptParticle::ExposePrimarySColor::ExposeA::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySColor.alpha;
}

// 0x40ab80
int ScriptParticle::ExposePrimarySColorSpread::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySColorSpreadWord;
}

// 0x40ab90
int ScriptParticle::ExposePrimarySColorSpread::ExposeR::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySColorSpread.red;
}

// 0x40aba0
int ScriptParticle::ExposePrimarySColorSpread::ExposeG::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySColorSpread.green;
}

// 0x40abb0
int ScriptParticle::ExposePrimarySColorSpread::ExposeB::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySColorSpread.blue;
}

// 0x40abc0
int ScriptParticle::ExposePrimarySColorSpread::ExposeA::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySColorSpread.alpha;
}

// 0x40abd0
float ScriptParticle::ExposePrimarySSize::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySSize;
}

// 0x40abe0
float ScriptParticle::ExposePrimarySSizeSpread::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primarySSizeSpread;
}

// 0x40abf0
float ScriptParticle::ExposePrimaryEVelocity::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEVelocity;
}

// 0x40ac00
float ScriptParticle::ExposePrimaryEVelocitySpread::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEVelocitySpread;
}

// 0x40ac10
float ScriptParticle::ExposePrimaryEGravity::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEGravity;
}

// 0x40ac20
float ScriptParticle::ExposePrimaryEDrag::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEDrag;
}

// 0x40ac30
int ScriptParticle::ExposePrimaryEColor::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEColorWord;
}

// 0x40ac40
int ScriptParticle::ExposePrimaryEColor::ExposeR::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEColor.red;
}

// 0x40ac50
int ScriptParticle::ExposePrimaryEColor::ExposeG::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEColor.green;
}

// 0x40ac60
int ScriptParticle::ExposePrimaryEColor::ExposeB::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEColor.blue;
}

// 0x40ac70
int ScriptParticle::ExposePrimaryEColor::ExposeA::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEColor.alpha;
}

// 0x40ac80
int ScriptParticle::ExposePrimaryEColorSpread::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEColorSpreadWord;
}

// 0x40ac90
int ScriptParticle::ExposePrimaryEColorSpread::ExposeR::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEColorSpread.red;
}

// 0x40aca0
int ScriptParticle::ExposePrimaryEColorSpread::ExposeG::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEColorSpread.green;
}

// 0x40acb0
int ScriptParticle::ExposePrimaryEColorSpread::ExposeB::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEColorSpread.blue;
}

// 0x40acc0
int ScriptParticle::ExposePrimaryEColorSpread::ExposeA::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryEColorSpread.alpha;
}

// 0x40acd0
float ScriptParticle::ExposePrimaryESize::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryESize;
}

// 0x40acf0
float ScriptParticle::ExposeEmitterSpreadX::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_emitterSpread.x;
}

// 0x40ad00
float ScriptParticle::ExposeEmitterSpreadY::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_emitterSpread.y;
}

// 0x40ad10
float ScriptParticle::ExposeEmitterSpreadZ::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_emitterSpread.z;
}

// 0x40ad20
float ScriptParticle::ExposeAngularSpreadZ::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_angularSpreadZ;
}

// 0x40ad30
float ScriptParticle::ExposeVisualRadius::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_visualRadius;
}

// 0x40ad40
void ScriptParticle::ExposeVisualRadius::SetValue(float value, ScriptReference *owner)
{
	Emitter(owner)->m_visualRadius = value;
}

// 0x40b6e0
void ScriptParticle::ExposePrimarySVelocity::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySVelocity = value;
	p->m_buf->Processor()->SetSpeed(STAGE_FROM, value);
}

// 0x40b710
void ScriptParticle::ExposePrimarySVelocitySpread::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySVelocitySpread = value;
	p->m_buf->Processor()->SetSpeedJitter(STAGE_FROM, value);
}

// 0x40b740
void ScriptParticle::ExposePrimarySGravity::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySGravity = value;
	p->m_buf->Processor()->SetFall(STAGE_FROM, value);
}

// 0x40b770
void ScriptParticle::ExposePrimarySDrag::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySDrag = value;
	p->m_buf->Processor()->SetResistance(STAGE_FROM, value);
}

// 0x40ba30
void ScriptParticle::ExposePrimarySColorSpread::ExposeR::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	unsigned char c = (unsigned char)value;
	p->m_primarySColorSpread.red = c;
	p->m_buf->Processor()->SetRedJitter(STAGE_FROM, c);
}

// 0x40ba50
void ScriptParticle::ExposePrimarySColorSpread::ExposeG::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	unsigned char c = (unsigned char)value;
	p->m_primarySColorSpread.green = c;
	p->m_buf->Processor()->SetGreenJitter(STAGE_FROM, c);
}

// 0x40ba70
void ScriptParticle::ExposePrimarySColorSpread::ExposeB::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	unsigned char c = (unsigned char)value;
	p->m_primarySColorSpread.blue = c;
	p->m_buf->Processor()->SetBlueJitter(STAGE_FROM, c);
}

// 0x40ba90
void ScriptParticle::ExposePrimarySColorSpread::ExposeA::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	unsigned char c = (unsigned char)value;
	p->m_primarySColorSpread.alpha = c;
	p->m_buf->Processor()->SetAlphaJitter(STAGE_FROM, c);
}

// 0x40bab0
void ScriptParticle::ExposePrimarySSize::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySSize = value;
	p->m_buf->Processor()->SetExtent(STAGE_FROM, value);
}

// 0x40bae0
void ScriptParticle::ExposePrimarySSizeSpread::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySSizeSpread = value;
	p->m_buf->Processor()->SetExtentJitter(STAGE_FROM, value);
}

// 0x40bb10
void ScriptParticle::ExposePrimaryEVelocity::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryEVelocity = value;
	p->m_buf->Processor()->SetSpeed(STAGE_TO, value);
}

// 0x40bb40
void ScriptParticle::ExposePrimaryEVelocitySpread::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryEVelocitySpread = value;
	p->m_buf->Processor()->SetSpeedJitter(STAGE_TO, value);
}

// 0x40bb70
void ScriptParticle::ExposePrimaryEGravity::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryEGravity = value;
	p->m_buf->Processor()->SetFall(STAGE_TO, value);
}

// 0x40bba0
void ScriptParticle::ExposePrimaryEDrag::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryEDrag = value;
	p->m_buf->Processor()->SetResistance(STAGE_TO, value);
}

// 0x40be60
void ScriptParticle::ExposePrimaryEColorSpread::ExposeR::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	unsigned char c = (unsigned char)value;
	p->m_primaryEColorSpread.red = c;
	p->m_buf->Processor()->SetRedJitter(STAGE_TO, c);
}

// 0x40be80
void ScriptParticle::ExposePrimaryEColorSpread::ExposeG::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	unsigned char c = (unsigned char)value;
	p->m_primaryEColorSpread.green = c;
	p->m_buf->Processor()->SetGreenJitter(STAGE_TO, c);
}

// 0x40bea0
void ScriptParticle::ExposePrimaryEColorSpread::ExposeB::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	unsigned char c = (unsigned char)value;
	p->m_primaryEColorSpread.blue = c;
	p->m_buf->Processor()->SetBlueJitter(STAGE_TO, c);
}

// 0x40bec0
void ScriptParticle::ExposePrimaryEColorSpread::ExposeA::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	unsigned char c = (unsigned char)value;
	p->m_primaryEColorSpread.alpha = c;
	p->m_buf->Processor()->SetAlphaJitter(STAGE_TO, c);
}

// 0x40bee0
void ScriptParticle::ExposePrimaryESize::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryESize = value;
	p->m_buf->Processor()->SetExtent(STAGE_TO, value);
}

// 0x40bf10
void ScriptParticle::ExposePrimaryESizeSpread::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryESizeSpread = value;
	p->m_buf->Processor()->SetExtentJitter(STAGE_TO, value);
}

// 0x40bf40
void ScriptParticle::ExposeEmitterSpreadX::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_emitterSpread.x = value;
	p->m_buf->Processor()->SetBoxX(value);
}

// 0x40bf70
void ScriptParticle::ExposeEmitterSpreadY::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_emitterSpread.y = value;
	p->m_buf->Processor()->SetBoxY(value);
}

// 0x40bfa0
void ScriptParticle::ExposeEmitterSpreadZ::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_emitterSpread.z = value;
	p->m_buf->Processor()->SetBoxZ(value);
}

// 0x40bfd0
void ScriptParticle::ExposeAngularSpreadX::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_angularSpreadX = value;
	p->m_buf->Processor()->SetTiltX(value);
}

// 0x40c000
void ScriptParticle::ExposeAngularSpreadZ::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_angularSpreadZ = value;
	p->m_buf->Processor()->SetTiltZ(value);
}

// 0x40ad60
int ScriptParticle::ExposeIgnoreCutScenes::GetInt(ScriptReference *owner)
{
	return Emitter(owner)->m_options & 1;
}

// 0x40ad70
void ScriptParticle::ExposeIgnoreCutScenes::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	if (value)
		p->m_options |= 1;
	else
		p->m_options &= ~1;
}

// 0x40ace0
float ScriptParticle::ExposePrimaryESizeSpread::GetFloat(ScriptReference *owner)
{
	return Emitter(owner)->m_primaryESizeSpread;
}

// 0x40b5a0
void ScriptParticle::ExposeOutputRate::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_outputRate = value;
	p->m_buf->Processor()->SetRate(value);
}

// 0x40b5f0
void ScriptParticle::ExposeOutputRateSpread::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_outputRateSpread = value;
	p->m_buf->Processor()->SetRateJitter(value);
}

// 0x40b640
void ScriptParticle::ExposeLifetime::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_lifetime = value;
	p->m_buf->Processor()->SetLife(static_cast<RwUInt32>(RWS::MainLoop::Logic::Rate * value));
}

// 0x40b690
void ScriptParticle::ExposeLifetimeSpread::SetValue(float value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_lifetimeSpread = value;
	p->m_buf->Processor()->SetLifeJitter(static_cast<RwUInt32>(RWS::MainLoop::Logic::Rate * value));
}

// 0x40b9d0
void ScriptParticle::ExposePrimarySColorSpread::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySColorSpreadWord = value;
	p->m_buf->Processor()->SetRedJitter(STAGE_FROM, p->m_primarySColorSpread.red);
	p->m_buf->Processor()->SetGreenJitter(STAGE_FROM, p->m_primarySColorSpread.green);
	p->m_buf->Processor()->SetBlueJitter(STAGE_FROM, p->m_primarySColorSpread.blue);
	p->m_buf->Processor()->SetAlphaJitter(STAGE_FROM, p->m_primarySColorSpread.alpha);
}

// 0x40be00
void ScriptParticle::ExposePrimaryEColorSpread::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryEColorSpreadWord = value;
	p->m_buf->Processor()->SetRedJitter(STAGE_TO, p->m_primaryEColorSpread.red);
	p->m_buf->Processor()->SetGreenJitter(STAGE_TO, p->m_primaryEColorSpread.green);
	p->m_buf->Processor()->SetBlueJitter(STAGE_TO, p->m_primaryEColorSpread.blue);
	p->m_buf->Processor()->SetAlphaJitter(STAGE_TO, p->m_primaryEColorSpread.alpha);
}

// 0x40b7a0
void ScriptParticle::ExposePrimarySColor::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySColorWord = value;
	RwRGBA col;
	col.red = p->m_primarySColor.red;
	col.green = p->m_primarySColor.green;
	col.blue = p->m_primarySColor.blue;
	col.alpha = p->m_primarySColor.alpha;
	CPB_TexPart *base = p->m_buf->Processor();
	base->SetColor(STAGE_FROM, col);
}

// 0x40b810
void ScriptParticle::ExposePrimarySColor::ExposeR::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySColor.red = (unsigned char)value;
	RwRGBA col;
	col.red = p->m_primarySColor.red;
	col.green = p->m_primarySColor.green;
	col.blue = p->m_primarySColor.blue;
	col.alpha = p->m_primarySColor.alpha;
	CPB_TexPart *base = p->m_buf->Processor();
	base->SetColor(STAGE_FROM, col);
}

// 0x40b880
void ScriptParticle::ExposePrimarySColor::ExposeG::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySColor.green = (unsigned char)value;
	RwRGBA col;
	col.red = p->m_primarySColor.red;
	col.green = p->m_primarySColor.green;
	col.blue = p->m_primarySColor.blue;
	col.alpha = p->m_primarySColor.alpha;
	CPB_TexPart *base = p->m_buf->Processor();
	base->SetColor(STAGE_FROM, col);
}

// 0x40b8f0
void ScriptParticle::ExposePrimarySColor::ExposeB::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySColor.blue = (unsigned char)value;
	RwRGBA col;
	col.red = p->m_primarySColor.red;
	col.green = p->m_primarySColor.green;
	col.blue = p->m_primarySColor.blue;
	col.alpha = p->m_primarySColor.alpha;
	CPB_TexPart *base = p->m_buf->Processor();
	base->SetColor(STAGE_FROM, col);
}

// 0x40b960
void ScriptParticle::ExposePrimarySColor::ExposeA::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primarySColor.alpha = (unsigned char)value;
	RwRGBA col;
	col.red = p->m_primarySColor.red;
	col.green = p->m_primarySColor.green;
	col.blue = p->m_primarySColor.blue;
	col.alpha = p->m_primarySColor.alpha;
	CPB_TexPart *base = p->m_buf->Processor();
	base->SetColor(STAGE_FROM, col);
}

// 0x40bbd0
void ScriptParticle::ExposePrimaryEColor::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryEColorWord = value;
	RwRGBA col;
	col.red = p->m_primaryEColor.red;
	col.green = p->m_primaryEColor.green;
	col.blue = p->m_primaryEColor.blue;
	col.alpha = p->m_primaryEColor.alpha;
	CPB_TexPart *base = p->m_buf->Processor();
	base->SetColor(STAGE_TO, col);
}

// 0x40bc40
void ScriptParticle::ExposePrimaryEColor::ExposeR::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryEColor.red = (unsigned char)value;
	RwRGBA col;
	col.red = p->m_primaryEColor.red;
	col.green = p->m_primaryEColor.green;
	col.blue = p->m_primaryEColor.blue;
	col.alpha = p->m_primaryEColor.alpha;
	CPB_TexPart *base = p->m_buf->Processor();
	base->SetColor(STAGE_TO, col);
}

// 0x40bcb0
void ScriptParticle::ExposePrimaryEColor::ExposeG::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryEColor.green = (unsigned char)value;
	RwRGBA col;
	col.red = p->m_primaryEColor.red;
	col.green = p->m_primaryEColor.green;
	col.blue = p->m_primaryEColor.blue;
	col.alpha = p->m_primaryEColor.alpha;
	CPB_TexPart *base = p->m_buf->Processor();
	base->SetColor(STAGE_TO, col);
}

// 0x40bd20
void ScriptParticle::ExposePrimaryEColor::ExposeB::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryEColor.blue = (unsigned char)value;
	RwRGBA col;
	col.red = p->m_primaryEColor.red;
	col.green = p->m_primaryEColor.green;
	col.blue = p->m_primaryEColor.blue;
	col.alpha = p->m_primaryEColor.alpha;
	CPB_TexPart *base = p->m_buf->Processor();
	base->SetColor(STAGE_TO, col);
}

// 0x40bd90
void ScriptParticle::ExposePrimaryEColor::ExposeA::SetValue(int value, ScriptReference *owner)
{
	GameParticle *p = Emitter(owner);
	p->m_primaryEColor.alpha = (unsigned char)value;
	RwRGBA col;
	col.red = p->m_primaryEColor.red;
	col.green = p->m_primaryEColor.green;
	col.blue = p->m_primaryEColor.blue;
	col.alpha = p->m_primaryEColor.alpha;
	CPB_TexPart *base = p->m_buf->Processor();
	base->SetColor(STAGE_TO, col);
}

}  // namespace Script

namespace Script {
// GLOBAL: 0x5d28d0 Script::g_vt_ScriptParticle_ExposePrimarySColor_ExposeA
extern char g_vt_ScriptParticle_ExposePrimarySColor_ExposeA[];
// GLOBAL: 0x5d2918 Script::g_vt_ScriptParticle_ExposePrimarySColor_ExposeB
extern char g_vt_ScriptParticle_ExposePrimarySColor_ExposeB[];
// GLOBAL: 0x5d2960 Script::g_vt_ScriptParticle_ExposePrimarySColor_ExposeG
extern char g_vt_ScriptParticle_ExposePrimarySColor_ExposeG[];
// GLOBAL: 0x5d29a8 Script::g_vt_ScriptParticle_ExposePrimarySColor_ExposeR
extern char g_vt_ScriptParticle_ExposePrimarySColor_ExposeR[];

// 0x43b2d0
void *ScriptParticle::ExposePrimarySColor::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptParticle::ExposePrimarySColor::ExposeR, ScriptParticle::ExposePrimarySColor::ExposeG, ScriptParticle::ExposePrimarySColor::ExposeB, ScriptParticle::ExposePrimarySColor::ExposeA> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5d29f0 Script::g_vt_ScriptParticle_ExposePrimarySColorSpread_ExposeA
extern char g_vt_ScriptParticle_ExposePrimarySColorSpread_ExposeA[];
// GLOBAL: 0x5d2a38 Script::g_vt_ScriptParticle_ExposePrimarySColorSpread_ExposeB
extern char g_vt_ScriptParticle_ExposePrimarySColorSpread_ExposeB[];
// GLOBAL: 0x5d2a80 Script::g_vt_ScriptParticle_ExposePrimarySColorSpread_ExposeG
extern char g_vt_ScriptParticle_ExposePrimarySColorSpread_ExposeG[];
// GLOBAL: 0x5d2ac8 Script::g_vt_ScriptParticle_ExposePrimarySColorSpread_ExposeR
extern char g_vt_ScriptParticle_ExposePrimarySColorSpread_ExposeR[];

// 0x43b350
void *ScriptParticle::ExposePrimarySColorSpread::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptParticle::ExposePrimarySColorSpread::ExposeR, ScriptParticle::ExposePrimarySColorSpread::ExposeG, ScriptParticle::ExposePrimarySColorSpread::ExposeB, ScriptParticle::ExposePrimarySColorSpread::ExposeA> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5d2b10 Script::g_vt_ScriptParticle_ExposePrimaryEColor_ExposeA
extern char g_vt_ScriptParticle_ExposePrimaryEColor_ExposeA[];
// GLOBAL: 0x5d2b58 Script::g_vt_ScriptParticle_ExposePrimaryEColor_ExposeB
extern char g_vt_ScriptParticle_ExposePrimaryEColor_ExposeB[];
// GLOBAL: 0x5d2ba0 Script::g_vt_ScriptParticle_ExposePrimaryEColor_ExposeG
extern char g_vt_ScriptParticle_ExposePrimaryEColor_ExposeG[];
// GLOBAL: 0x5d2be8 Script::g_vt_ScriptParticle_ExposePrimaryEColor_ExposeR
extern char g_vt_ScriptParticle_ExposePrimaryEColor_ExposeR[];

// 0x43b3e0
void *ScriptParticle::ExposePrimaryEColor::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptParticle::ExposePrimaryEColor::ExposeR, ScriptParticle::ExposePrimaryEColor::ExposeG, ScriptParticle::ExposePrimaryEColor::ExposeB, ScriptParticle::ExposePrimaryEColor::ExposeA> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5d2c30 Script::g_vt_ScriptParticle_ExposePrimaryEColorSpread_ExposeA
extern char g_vt_ScriptParticle_ExposePrimaryEColorSpread_ExposeA[];
// GLOBAL: 0x5d2c78 Script::g_vt_ScriptParticle_ExposePrimaryEColorSpread_ExposeB
extern char g_vt_ScriptParticle_ExposePrimaryEColorSpread_ExposeB[];
// GLOBAL: 0x5d2cc0 Script::g_vt_ScriptParticle_ExposePrimaryEColorSpread_ExposeG
extern char g_vt_ScriptParticle_ExposePrimaryEColorSpread_ExposeG[];
// GLOBAL: 0x5d2d08 Script::g_vt_ScriptParticle_ExposePrimaryEColorSpread_ExposeR
extern char g_vt_ScriptParticle_ExposePrimaryEColorSpread_ExposeR[];

// 0x43b460
void *ScriptParticle::ExposePrimaryEColorSpread::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptParticle::ExposePrimaryEColorSpread::ExposeR, ScriptParticle::ExposePrimaryEColorSpread::ExposeG, ScriptParticle::ExposePrimaryEColorSpread::ExposeB, ScriptParticle::ExposePrimaryEColorSpread::ExposeA> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

namespace Script {

// 0x43b100
const char *ScriptParticle::ExposeOutputRate::GetName()
{
	return "output rate";
}

// 0x43b130
const char *ScriptParticle::ExposeLifetime::GetName()
{
	return "lifetime";
}

}  // namespace Script


ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_Particle();

namespace Script {

// 0x43b120
const char *ScriptParticle::ExposeOutputRateSpread::GetName()
{
	return "output rate spread";
}

// 0x43b140
const char *ScriptParticle::ExposeLifetimeSpread::GetName()
{
	return "lifetime spread";
}

// 0x43b160
const char *ScriptParticle::ExposePrimarySVelocity::GetName()
{
	return "primary start velocity";
}

// 0x43b170
const char *ScriptParticle::ExposePrimarySVelocitySpread::GetName()
{
	return "primary start velocity spread";
}

// 0x43b180
const char *ScriptParticle::ExposePrimarySGravity::GetName()
{
	return "primary start gravity";
}

// 0x43b190
const char *ScriptParticle::ExposePrimarySDrag::GetName()
{
	return "primary start drag";
}

// 0x43b1a0
const char *ScriptParticle::ExposePrimarySColor::GetName()
{
	return "primary start color";
}

// 0x43b1b0
const char *ScriptParticle::ExposePrimarySColorSpread::GetName()
{
	return "primary start color spread";
}

// 0x43b1c0
const char *ScriptParticle::ExposePrimarySSize::GetName()
{
	return "primary start size";
}

// 0x43b1d0
const char *ScriptParticle::ExposePrimarySSizeSpread::GetName()
{
	return "primary start size spread";
}

// 0x43b1e0
const char *ScriptParticle::ExposePrimaryEVelocity::GetName()
{
	return "primary end velocity";
}

// 0x43b1f0
const char *ScriptParticle::ExposePrimaryEVelocitySpread::GetName()
{
	return "primary end velocity spread";
}

// 0x43b200
const char *ScriptParticle::ExposePrimaryEGravity::GetName()
{
	return "primary end gravity";
}

// 0x43b210
const char *ScriptParticle::ExposePrimaryEDrag::GetName()
{
	return "primary end drag";
}

// 0x43b220
const char *ScriptParticle::ExposePrimaryEColor::GetName()
{
	return "primary end color";
}

// 0x43b230
const char *ScriptParticle::ExposePrimaryEColorSpread::GetName()
{
	return "primary end color spread";
}

// 0x43b240
const char *ScriptParticle::ExposePrimaryESize::GetName()
{
	return "primary end size";
}

// 0x43b260
const char *ScriptParticle::ExposePrimaryESizeSpread::GetName()
{
	return "primary end size spread";
}

// 0x43b280
const char *ScriptParticle::ExposeEmitterSpreadX::GetName()
{
	return "emitter spread x";
}

// 0x43b290
const char *ScriptParticle::ExposeEmitterSpreadY::GetName()
{
	return "emitter spread y";
}

// 0x43b2a0
const char *ScriptParticle::ExposeEmitterSpreadZ::GetName()
{
	return "emitter spread z";
}

// 0x43b2c0
const char *ScriptParticle::ExposeVisualRadius::GetName()
{
	return "visual radius";
}

// 0x43b850
const char *ScriptParticle::ExposeAngularSpreadX::GetName()
{
	return "angular spread x";
}

// 0x43b860
const char *ScriptParticle::ExposeAngularSpreadZ::GetName()
{
	return "angular spread z";
}

}  // namespace Script

// 0x43b110
int ExposeOutputRate_GetMax()
{
	return 5000;
}

// 0x43b250
int ExposePrimaryVelocity_GetMax()
{
	return 1000;
}

// 0x43b270
int ExposePrimarySizeSpread_GetMax()
{
	return 50;
}

// 0x43b2b0
int ExposeParticleSpread_GetMax()
{
	return 500;
}

// 0x43b4e0
int ExposeColorComponent_GetMax()
{
	return 255;
}

// 0x43b870
Script::ScriptReference *ScriptParticle_GetType()
{
	return TFBType_Particle();
}

// 0x40cf90
const char *ScriptParticle_GetName()
{
	return "particle";
}

// FOLDED: 0x413c10 Script::ScriptParticle::ExposeAngularSpreadX::GetFloat (the game's ScriptSprite::ExposeRotation::GetFloat: the field at the same offset)
float Script::ScriptParticle::ExposeAngularSpreadX::GetFloat(Script::ScriptReference *owner) { return Emitter(owner)->m_angularSpreadX; }
