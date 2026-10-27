// Animation choice of a player-controlled body (Corpus::Tick), from its state and speed.
#define ZOOSTERS_RWS_SENDMSG 1
#include "tfb/corpus/corpus.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "script/ops_message.h"
#include <math.h>
#include "rw/rpplugins.h"
#include "rw/rwcore.h"
#include "script/types/scriptcamera.h"

// GLOBAL: 0x602f70 g_prevCutsceneFacing
extern float g_frameDt;
extern float g_prevCutsceneFacing;
float CameraFacing();                                // 0x41e6d0
int *CutScene_StackSlot(int delta);                  // 0x436c00

using RWS::TFB::ActorParams;


// Animation slots: 1 slow (idle/walk blend), 2 walk, 3 run, 4/5 rise/fall, 0x2b/0x2c turn on the spot
// right/left, 0x2d/0x2e land (slow/fast), 0x30/0x31 stop from a run, 0x32 skid (turning round at speed), 0x3b/0x3e lean
// right/left (aux), 0x3f + 5 k idle variant set k (k 0..2: +0 main, +1/+2 fidgets, +3/+4 switch to the next/previous set).

// Picks the animation: rise or fall in the air; on the ground landing, skid, or by forward speed run, walk, slow, or turn and idle; then the lean. `dir` is the wanted direction (skid test).
// 0x4235b0
void Corpus::UpdateAnimation(const RwV3d *dir)
{
	TFBModelState *model = &m_model;
	// in the air, after the late-jump window
	if ((m_flags & CORPUS_AIRBORNE) && m_lateJumpTime == 0.0f
	    && (m_stateTime > 1.0f || !(m_flags & CORPUS_GROUND_FOUND) || m_lastFrame.pos.y - m_contactPoint.y > 24.0f
	        || model->IsSlotPlaying(CORPUS_ANIM_RISE) || model->IsSlotPlaying(CORPUS_ANIM_FALL))) {
		if (m_owner->m_velocity.y < 0.0f) {   // falling: after the rise has played out
			if (model->IsSlotPlaying(CORPUS_ANIM_RISE)) {
				if (!model->WillFinish(g_frameDt))
					return;
			} else if (model->IsSlotPlaying(CORPUS_ANIM_FALL)) {
				return;
			}
			model->PlaySlot(CORPUS_ANIM_FALL, 0.0f);
			m_prevIdleSet = 0;
			return;
		}
		if (m_model.GetSlotIndex(4) != 4)   // slot 4 remapped: no rise animation of its own
			return;
		model->PlaySlot(CORPUS_ANIM_RISE, 0.0f);
		m_prevIdleSet = 0;
		return;
	}

	const ActorParams &params = m_owner->m_params;
	float speed = RwV3dLength(&m_velocity) * 0.083333336f;   // per second / 12: the parameters' units
	float fwd = 0.0f;   // the speed when pushing forwards (moving along the facing), else 0
	if (m_moveInput.x != 0.0f && RwV3dDotProduct(&m_frame->modelling.at, &m_velocity) >= 0.0f)
		fwd = speed;

	if (m_stateTime == 0.0f && (model->IsSlotPlaying(CORPUS_ANIM_RISE) || model->IsSlotPlaying(CORPUS_ANIM_FALL))) {
		model->PlaySlot(fwd > params.m_runSpeed ? 0x2e : 0x2d, 0.0f);
		m_prevIdleSet = 0;
	} else if (model->IsSlotPlaying(0x32)) {   // a skid plays to its end, then the slow cycle
		if (model->WillFinish(g_frameDt)) {
			model->PlaySlot(1, 0.0f);
			m_prevIdleSet = 0;
		}
	} else if ((CORPUS_AUX_ACTIVE(this)) && model->GetAuxAnimation() >= 0x39) {   // leaning: keep running
		model->PlaySlot(3, 0.0f);
		m_prevIdleSet = 0;
	} else if (fwd > params.m_runSpeed) {
		if (fwd > params.m_groundSpeed)   // also for the lean below
			fwd = params.m_groundSpeed;
		Run(dir, speed, fwd);
	} else if (fwd > params.m_walkSpeed) {
		Walk(fwd);
	} else if (fwd > m_standSpeed) {
		WalkSlowly(fwd);
	} else {
		Stand(speed, fwd);
	}
	UpdateLean(fwd);
}

// Run cycle, blended by forward speed between run and top speed; turning round at speed skids.
void Corpus::Run(const RwV3d *dir, float speed, float fwd)
{
	TFBModelState *model = &m_model;
	const ActorParams &params = m_owner->m_params;
	if (speed > params.m_runSpeed && m_moveInput.x > 0.5f
	    && ((m_velocity.y * dir->y + dir->x * m_velocity.x) + m_velocity.z * dir->z) / (speed * m_moveInput.x) < -0.2f) {
		model->PlaySlot(model->HasSlotAnimation(0x32) ? 0x32 : 3, 0.0f);
		m_prevIdleSet = 0;
	} else if (!model->IsSlotPlaying(0x2e)) {
		model->PlaySlot(3, 0.0f);
		m_prevIdleSet = 0;
	}
	float range = params.m_groundSpeed - params.m_runSpeed;
	int lo = params.m_runAnims[0];
	int hi = params.m_runAnims[1];
	float t = (float)(hi - lo) * 0.01f;
	if (range != 0.0f)
		t *= (fwd - params.m_runSpeed) / range;
	t += (float)lo * 0.01f;
	if (t < 0.0f)
		t = 0;
	model->SetSlotRate(3, t);
}

// The walk cycle (unless still running down to half way into the walk range), blended between the walk and run speed.
void Corpus::Walk(float fwd)
{
	TFBModelState *model = &m_model;
	const ActorParams &params = m_owner->m_params;
	float range = params.m_runSpeed - params.m_walkSpeed;
	if (model->IsSlotPlaying(3) && !(fwd < range * 0.5f + params.m_walkSpeed))
		return;
	model->PlaySlot(2, 0.0f);
	m_prevIdleSet = 0;
	int lo = params.m_walkAnims[0];
	int hi = params.m_walkAnims[1];
	float t = (float)(hi - lo) * 0.01f;
	if (range != 0.0f)
		t *= (fwd - params.m_walkSpeed) / range;
	t += (float)lo * 0.01f;
	if (t < 0.0f)
		t = 0;
	model->SetSlotRate(2, t);
}

// The slow cycle (unless still walking down to half the walk speed), blended up to the walk speed.
void Corpus::WalkSlowly(float fwd)
{
	TFBModelState *model = &m_model;
	const ActorParams &params = m_owner->m_params;
	float limit = params.m_walkSpeed;
	if (model->IsSlotPlaying(2) && !(limit * 0.5f > fwd))
		return;
	model->PlaySlot(1, 0.0f);
	m_prevIdleSet = 0;
	int lo = params.m_idleAnims[0];
	int hi = params.m_idleAnims[1];
	float t = (float)(hi - lo) * 0.01f;
	if (limit != 0.0f)
		t = t / limit * fwd;
	t += (float)lo * 0.01f;
	if (t < 0.0f)
		t = 0;
	model->SetSlotRate(1, t);
}

// Standing: turning on the spot plays the turn animations; else the idle variants (in a cut-scene the actor faces the camera
// first).
void Corpus::Stand(float speed, float fwd)
{
	TFBModelState *model = &m_model;
	bool idle = speed == 0.0f && m_moveInput.x == 0.0f && m_owner->m_targetFacing == 3.4028235e+38f;
	if (*CutScene_StackSlot(0) != 2) {
		m_prevIdleSet = 0;
		m_idleSet = 0;
	} else if (idle) {
		float facing = 3.4028235e38f;
		if (m_idleSet == 0 && (unsigned)m_prevIdleSet > 0) {
			Script::ScriptReference *camera = GetCameraObject();
			if (camera && (static_cast<Script::ScriptCamera *>(camera)->m_flags & 1)) {
				facing = CameraFacing() + 180.0f;
				if (facing != 3.4028235e38f && g_prevCutsceneFacing == facing)
					m_owner->m_targetFacing = facing;
			}
		}
		g_prevCutsceneFacing = facing;
	}
	if (fabsf(m_moveInput.y) > 1.0f) {   // turning
		if (!model->IsSlotPlaying(1) || model->WillFinish(g_frameDt)) {
			if (m_moveInput.y > 0.0f && model->HasSlotAnimation(0x2b))
				PlayAnim(0x2b, idle);
			else if (m_moveInput.y < 0.0f && model->HasSlotAnimation(0x2c))
				PlayAnim(0x2c, idle);
			else
				PlayAnim(1, idle);
		}
		return;
	}
	if ((model->IsSlotPlaying(0x2d) && !model->WillFinish(g_frameDt)) || (model->IsSlotPlaying(1) && fwd != 0.0f))
		return;   // landing still plays, or still slowing down
	int next = ChooseIdleVariant(speed);
	if (next >= 0) {
		model->PlaySlot(next, 0.0f);
		m_idleSlot = next;
	}
}

// The next idle slot (-1: keep the current one). Outside cut-scenes (or with slot 0 remapped) only slot 0; else a stop animation after a run, then the variant sets: main (80 %) or fidget (10 %); the switch animations (5 % each) move to the next or previous set that has animations.
int Corpus::ChooseIdleVariant(float speed)
{
	TFBModelState *model = &m_model;
	unsigned cur = m_model.GetLastSlot();
	if (m_model.GetSlotIndex(0) != 0 || *CutScene_StackSlot(0) != 2)   // slot 0 remapped / not a cut-scene
		return 0;
	if (cur == 0 && !model->WillFinish(g_frameDt))
		return -1;
	if (cur < 0x3f) {   // from moving: a stop animation first, then set 0
		m_prevIdleSet = 0;
		m_idleSet = 0;
		int next = 0x3f;
		if (cur == 0x30 || cur == 0x31) {
			if (model->IsSlotPlaying(cur))
				next = cur;
		} else if (m_moveInput.x == 0.0f && m_owner->m_params.m_groundSpeed * 0.75f < speed) {
			next = 0x30;
		}
		return model->HasSlotAnimation(next) ? next : 0;
	}
	if (model->IsSlotPlaying(cur) && !model->WillFinish(g_frameDt))
		return (int)cur;
	int base = m_idleSet * 5 + 0x3f;
	int toNext = base + 3;
	int set = m_idleSet;
	if (cur == (unsigned)toNext) {   // switched forwards: the next set with animations
		m_prevIdleSet = set;
		do {
			set++;
			if ((unsigned)set > 2)
				set = 0;
			cur = set * 5 + 0x3f;
		} while (!model->HasSlotAnimation(cur) && m_prevIdleSet != set);
		m_idleSet = set;
		return (int)cur;
	}
	int toPrevious = base + 4;
	if (cur == (unsigned)toPrevious) {   // switched backwards
		m_prevIdleSet = set;
		do {
			if (set == 0)
				set = 2;
			else
				set--;
			cur = set * 5 + 0x3f;
		} while (!model->HasSlotAnimation(cur) && m_prevIdleSet != set);
		m_idleSet = set;
		return (int)cur;
	}
	for (;;) {   // a random animation of the set (the fidgets not twice in a row)
		unsigned r = RpRandom() % 100;
		if (r < 5) {
			if (model->HasSlotAnimation(toNext))
				return toNext;
		} else if (r < 10) {
			if (model->HasSlotAnimation(toPrevious))
				return toPrevious;
		} else if (r < 90) {
			if (model->HasSlotAnimation(base))
				return base;
		} else if (r < 95) {
			if ((unsigned)(base + 1) != cur && model->HasSlotAnimation(base + 1))
				return base + 1;
		} else {
			if ((unsigned)(base + 2) != cur && model->HasSlotAnimation(base + 2))
				return base + 2;
		}
	}
}

// Lean into a turn while running (aux 0x3b right / 0x3e left), smoothed from the turn rate; else the aux state is cleared.
void Corpus::UpdateLean(float fwd)
{
	TFBModelState *model = &m_model;
	const ActorParams &params = m_owner->m_params;
	if (m_model.GetLastSlot() != 3) {
		m_moveInput.z = 0;
	} else {
		float w = 0;
		if (fwd > params.m_walkSpeed) {   // the turn as a share of the turn rate, squared, signed, -1..1
			float q = (m_moveInput.y / (g_frameDt * params.m_groundTurnRate)) * 0.3333f;
			w = q * q;
			if (q < 0.0f)
				w = -w;
			if (w < -1.0f)
				w = -1.0f;
			else if (w > 1.0f)
				w = 1.0f;
		}
		unsigned id = model->GetAuxAnimation();
		float p = m_model.GetAuxWeight();
		float x = 0.7f * m_moveInput.z + 0.3f * w;
		m_moveInput.z = x;
		if (x < 0.0f) {
			if (id != 0x3b) {
				id = 0x3e;
				w = -x * 0.1f + p * 0.9f;
			} else {
				w = x * 0.1f + p * 0.9f;
				if (w < 0.0f) {
					id = 0x3e;
					w = -w;
				}
			}
		} else {
			if (id != 0x3e) {
				id = 0x3b;
				w = x * 0.1f + p * 0.9f;
			} else {
				w = -x * 0.1f + p * 0.9f;
				if (w < 0.0f) {
					id = 0x3b;
					w = -w;
				}
			}
		}
		if (fwd != 0.0f && (!(fwd <= params.m_walkSpeed) || !(fabsf(w) < 0.1f))) {
			model->SetAuxSlot(3, id, w);
			return;
		}
	}
	m_model.ClearAux();
}

// 0x423580
void Corpus::PlayAnim(int slot, bool keep)
{
	m_model.PlaySlot(slot, 0.0f);
	if (!keep)
		m_prevIdleSet = 0;
}
