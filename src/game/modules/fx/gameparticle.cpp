#include "modules/fx/gameparticle.h"

extern const RwV3d g_particleXAxis;     // GLOBAL: 0x5cb310 g_particleXAxis
extern const RwV3d g_particleYAxis;     // GLOBAL: 0x5cb31c g_particleYAxis
extern const RwV3d g_particleZAxis;     // GLOBAL: 0x5cb328 g_particleZAxis
extern const float g_floatZero;         // GLOBAL: 0x5ca5a8 g_floatZero
int *CutScene_StackSlot(int delta);     // 0x436c00

namespace RWS { namespace FX {

// 0x40a640
// Activation distance in game units: FLT_MAX = always, 0 = never. Outside cut scenes only when "ignore cut scenes" is set; in a cut scene
// (state 2) the range x 12, or always when both ranges are 0.
float GameParticle::GetActivationDistance()
{
	if (*CutScene_StackSlot(0) != 2)
		return (m_options & 1) == 0 ? g_floatZero : 3.4028235e+38f;
	if (m_activationRange == g_floatZero && m_deactivationRange == g_floatZero)
		return 3.4028235e+38f;
	return m_activationRange * 12.0f;
}

// 0x40a6b0
int GameParticle::AdjustRefCount(int delta)
{
	m_refCount += delta;
	if (m_refCount == 0) {
		delete this;
		return 0;
	}
	return m_refCount;
}

// 0x40a6e0
char GameParticle::Control(bool on)
{
	if (m_spawnSource) {
		if (m_spawnSource == this)
			return true;
		if (m_spawnSource->m_20.FindNode(this))
			return true;
	}
	return false;
}

// 0x40a710
Script::ScriptReference *GameParticle::GetSpawnSource()
{
	if (!m_spawnSource || m_spawnSource == this)
		return 0;
	return m_spawnSource;
}

// 0x40a730
bool GameParticle::SetFacing(float value, int flag)
{
	m_facing = value;
	if (m_frame) {
		RwV3d pos = m_frame->modelling.pos;
		RwFrameRotate(m_frame, &g_particleZAxis, m_roll, rwCOMBINEREPLACE);
		RwFrameRotate(m_frame, &g_particleXAxis, -m_tilt, rwCOMBINEPOSTCONCAT);
		RwFrameRotate(m_frame, &g_particleYAxis, m_facing, rwCOMBINEPOSTCONCAT);
		RwFrameTranslate(m_frame, &pos, rwCOMBINEPOSTCONCAT);
	}
	return true;
}

// 0x40a7d0
float GameParticle::GetFacing()
{
	return m_facing;
}

// 0x40a7e0
bool GameParticle::SetTilt(float value, Script::ScriptReference *owner)
{
	m_tilt = value;
	if (m_frame) {
		RwV3d pos = m_frame->modelling.pos;
		RwFrameRotate(m_frame, &g_particleZAxis, m_roll, rwCOMBINEREPLACE);
		RwFrameRotate(m_frame, &g_particleXAxis, -m_tilt, rwCOMBINEPOSTCONCAT);
		RwFrameRotate(m_frame, &g_particleYAxis, m_facing, rwCOMBINEPOSTCONCAT);
		RwFrameTranslate(m_frame, &pos, rwCOMBINEPOSTCONCAT);
	}
	return true;
}

// 0x40a880
float GameParticle::GetTilt()
{
	return m_tilt;
}

// 0x40a890
bool GameParticle::SetRoll(float value, Script::ScriptReference *owner)
{
	m_roll = value;
	if (m_frame) {
		RwV3d pos = m_frame->modelling.pos;
		RwFrameRotate(m_frame, &g_particleZAxis, m_roll, rwCOMBINEREPLACE);
		RwFrameRotate(m_frame, &g_particleXAxis, -m_tilt, rwCOMBINEPOSTCONCAT);
		RwFrameRotate(m_frame, &g_particleYAxis, m_facing, rwCOMBINEPOSTCONCAT);
		RwFrameTranslate(m_frame, &pos, rwCOMBINEPOSTCONCAT);
	}
	return true;
}

// 0x40a930
float GameParticle::GetRoll()
{
	return m_roll;
}

// 0x40a940
Vec3 *GameParticle::GetVector(Script::ScriptReference *owner)
{
	return (Vec3 *)&static_cast<GameParticle *>(owner)->m_frame->modelling.pos;   // Vec3 = RwV3d layout
}
}}
