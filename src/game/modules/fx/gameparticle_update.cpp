// Particle shield: activates GameParticles near a player, deactivates those that left, ticks the active ones.
#define _ALLOW_KEYWORD_MACROS   // (modern STL headers refuse a keyword macro otherwise)
#define protected public
#include "modules/fx/partspray.h"
#include "script/actor.h"

int *CutScene_StackSlot(int index);                       // 0x436c00

// The player set is a Script::ReferenceSet of SetNodes (head offset 0x10 and node offsets are the x86 values).
namespace RWS { namespace FX {
typedef Script::SetNode PlayerNode;
typedef Script::ReferenceSet PlayerSet;
}}
#define PLAYER_OBJ(n) ((Script::PositionMeasurement *)(n)->m_object)
namespace RWS { namespace FX {
inline PlayerSet *TFB_GetPlayerSet() { return (PlayerSet *)::TFB_GetPlayerSet(); }

// 0x40cc20
void UpdateGameParticles()
{
	if (*CutScene_StackSlot(0) == 3)
		return;
	if (*CutScene_StackSlot(0) == 2) {
		Script::SetNode *node = TFB_GetInactiveParticleSet()->m_head;
		while (node) {
			GameParticle *p = static_cast<GameParticle *>(node->m_object);
			Script::SetNode *next = node->m_next;
			float range = p->GetActivationDistance();
			if (range != 0.0f) {
				if (range == 3.4028235e38f) {
					TFB_GetActiveParticleSet()->Insert(p, 3.4028235e38f);
					TFB_GetInactiveParticleSet()->RemoveNode(node);
					p->m_buf->Resize(p->m_poolCapacity, 0);
				} else {
					float rangeSq = range * range;
					for (PlayerNode *n = TFB_GetPlayerSet()->m_head; n; n = n->m_next) {
						if (PLAYER_OBJ(n)->Dist(PLAYER_OBJ(n), p, p) < rangeSq) {
							TFB_GetActiveParticleSet()->Insert(p, 3.4028235e38f);
							TFB_GetInactiveParticleSet()->RemoveNode(node);
							p->m_buf->Resize(p->m_poolCapacity, 0);
							break;
						}
					}
				}
			}
			node = next;
		}
	}
	Script::SetNode *node = TFB_GetActiveParticleSet()->m_head;
	while (node) {
		Script::SetNode *next = node->m_next;
		GameParticle *p = static_cast<GameParticle *>(node->m_object);
		bool stay = false;
		float radius;
		if (*CutScene_StackSlot(0) != 2) {
			if (p->m_options & 1)
				stay = true;
			else
				radius = 0.0f;
		} else {
			if (p->m_activationRange == 0.0f && p->m_deactivationRange == 0.0f) {
				stay = true;
			} else {
				radius = (p->m_deactivationRange + p->m_activationRange) * 12.0f;
				if (radius == 3.4028235e38f)
					stay = true;
			}
		}
		if (!stay) {
			float rangeSq = radius * radius;
			for (PlayerNode *n = TFB_GetPlayerSet()->m_head; n; n = n->m_next) {
				if (PLAYER_OBJ(n)->Dist(PLAYER_OBJ(n), p, p) <= rangeSq) {
					stay = true;
					break;
				}
			}
		}
		if (!stay) {
			float act = p->m_activationRange;
			float deact = p->m_deactivationRange;
			bool isClone = p->m_spawnSource != p;
			p->Reset();
			if (!isClone) {
				p->m_activationRange = act;
				p->m_deactivationRange = deact;
			}
		} else if (p->m_visible) {
			p->m_buf->Tick();
			RwMatrix *modelling = &p->m_frame->modelling;
			p->m_buf->Processor()->SetOrigin(modelling->pos);
			p->m_buf->Processor()->SetUpAxis(modelling->up);
			p->m_buf->Processor()->SetRightAxis(modelling->right);
			p->m_buf->Processor()->SetAtAxis(modelling->at);
			p->m_buf->Emit();
		}
		node = next;
	}
}
}}
