// Per-frame rumble (force feedback) mixing.
#include "modules/tfb/gamecontroller.h"
#include "framework/core/ceventhandler.h"
#include "modules/tfb/gameactor.h"
#include "script/types/scriptcontroller.h"
#include "rw/rwcore.h"
#include "script/referenceset.h"

extern float g_frameDt;                       // GLOBAL: 0x618318 g_frameDt
// GLOBAL: 0x617988 g_rumbleOff
unsigned char g_rumbleOff;                    // the Options menu's rumble switch (config key Settings\\Options\\RumbleOff)

Script::ReferenceSet *RumbleSet_Get();         // 0x4034d0
struct CutSceneSlot { int m_state; int m_counter; int m_stamp; };   // CutScene_StackSlot(0): the cut-scene stack slot (same layout as script/ops_actor.cpp: state, counter, stamp)

int *CutScene_StackSlot(int delta);  // 0x436c00
unsigned GetTimeMs();                      // 0x44d2c0
void BlockInputsUntilReleased(int a);
void SetPadRumble(float a, float b);          // 0x4058e0 set the pad actuator levels (ctl/ctl_pads.cpp)

// 0x403530
void RWS::TFB::GameController::HandleEvents(RWS::CMsg &msg)
{
	if (msg.Id == RWS::iMsgRunningPostTick) {
		unsigned mask = m_blockMask;
		if (mask && GetTimeMs() >= (unsigned)m_blockUntil) {
			m_blockMask = 0;
			for (int i = 0; i < 6; i++) {
				unsigned bit = 1 << i;
				if ((mask & bit) && GetInputValue((int *)TFB_GetPlayerRefs(), i, 0.0f) == 0.0f)
					mask &= ~bit;
			}
			m_blockMask = mask;
		}
		float act1 = 0.0f;
		float act2 = 0.0f;
		int type = ((CutSceneSlot *)CutScene_StackSlot(0))->m_state;
		unsigned level = (unsigned)((CutSceneSlot *)CutScene_StackSlot(0))->m_counter;
		Script::SetNode *node = RumbleSet_Get()->m_head;
		while (node) {
			Script::ScriptController::ScriptRumble *r = (Script::ScriptController::ScriptRumble *)node->m_object;
			Script::SetNode *next = node->m_next;
			float level1 = (float)r->m_actuator1Level;
			float level2 = (float)r->m_actuator2Level;
			if (r->m_slotLevel < level) {
				if (type != 3) {
					if (r->m_duration != 0.0f || (level1 == 0.0f && level2 == 0.0f))
						r->Remove();
				}
			} else {
				float timeLeft = r->m_timeRemaining;
				if ((timeLeft == 0.0f && r->m_duration != 0.0f) || (level1 == 0.0f && level2 == 0.0f)) {
					r->Remove();
				} else {
					if (timeLeft != 0.0f) {
						float rest = timeLeft - g_frameDt;
						if (rest < 0.0f)
							rest = 0.0f;
						r->m_timeRemaining = rest;
					}
					if (r->m_radius == 0.0f) {
						act1 += level1;
						act2 += level2;
					} else {
						for (Script::SetNode *p = TFB_GetPlayerSet()->m_head; p; p = p->m_next) {
							GameActor *po = (GameActor *)p->m_object;
							if (po->m_corpus) {
								RwV3d va = *(RwV3d *)r->GetVector(r);
								RwV3d vb = *(RwV3d *)po->GetVector(po);
								RwV3d d;
								d.x = (((&va)->x) - ((&vb)->x));
								RwV3d vc = *(RwV3d *)r->GetVector(r);
								RwV3d vd = *(RwV3d *)po->GetVector(po);
								d.y = (((&vc)->y) - ((&vd)->y));
								RwV3d ve = *(RwV3d *)r->GetVector(r);
								RwV3d vf = *(RwV3d *)po->GetVector(po);
								d.z = (((&ve)->z) - ((&vf)->z));
								float rad = r->m_radius * 12.0f;
								float rem = rad - RwV3dLength(&d);
								if (!(rem <= 0.0f)) {
									if (r->m_attenuation == 0) {
										act1 += level1;
										act2 += level2;
									} else {
										float f = (r->m_attenuation == 2 ? rem * rem : rem) / (r->m_attenuation == 2 ? rad * rad : rad);
										act1 += level1 * f;
										act2 += level2 * f;
									}
								}
							}
						}
					}
				}
			}
			node = next;
		}
		if (!g_rumbleOff && type != -1) {
			if (act1 > 100.0f)
				act1 = 100.0f;
			if (act2 > 100.0f)
				act2 = 100.0f;
		} else {
			act2 = 0.0f;
			act1 = 0.0f;
		}
		SetPadRumble(act1, act2);
	} else if (msg.Id == RWS::iMsgStartSystem) {
		BlockInputsUntilReleased(0);
		SetPadRumble(0.0f, 0.0f);
	} else if (msg.Id == RWS::iMsgStopSystem) {
		while (RumbleSet_Get()->m_head)
			((Script::ScriptController::ScriptRumble *)RumbleSet_Get()->m_head->m_object)->Remove();
		SetPadRumble(0.0f, 0.0f);
	}
}

// 0x4025e0
void SetRumbleOff(int value)
{
	g_rumbleOff = value != 0;
}

// 0x4025f0
int GetRumbleOff()
{
	return g_rumbleOff;
}
