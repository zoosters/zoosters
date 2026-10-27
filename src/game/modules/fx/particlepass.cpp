#include "rw/rwcore.h"
#include "rw/rwviewbase.h"
#include "modules/fx/ptankext.h"
#include "modules/fx/particlepass.h"
#include "tfb/render/tfbrenderstate.h"
// GLOBAL: 0x6308e8 g_ptankPluginOffset
extern int g_ptankPluginOffset;
#include "modules/fx/particlepass_views.h"
// RWS::Particle::ParticlePool<T>::Tank: m_atomic, m_live, m_ages, m_next (4 pointers)
struct ParticlePassNode { PassAtomicView *m_atomic; void *m_activeFlags; void *m_lifeTimes; ParticlePassNode *m_next; };
typedef RpPTankExtView PtankPluginView;
#define PTANK_PLUGIN(a) (*(PtankPluginView **)((char *)(a) + g_ptankPluginOffset))

// 0x41af60
void ParticlePass::Render()
{
	if (m_atomics) {
		int zTest;   // state 6 = rwRENDERSTATEZTESTENABLE
		RwRenderStateGet(rwRENDERSTATEZTESTENABLE, (void *)&zTest);
		if (zTest == 0)
			RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)1);
		int zWrite;  // state 8 = rwRENDERSTATEZWRITEENABLE
		RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, (void *)&zWrite);
		if (m_10 == 0 || m_10 == 2 || (m_srcBlend == 5 && m_dstBlend == 2)) {
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
			StencilEnable(0);
		} else {
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)1);
			StencilEnable(1);
		}
		TFBRenderState::Apply(0);
		SetBlendAndAlphaTest((RwBlendFunction)m_srcBlend, (RwBlendFunction)m_dstBlend, 2);
		if (m_1c) {
			StencilWrite(1);
			StencilEnable(1);
		}
		for (ParticlePassNode *n = m_atomics; n; n = n->m_next)
			n->m_atomic->renderCallBack(n->m_atomic);
		if (m_10 == 2 && !(m_srcBlend == 5 && m_dstBlend == 2)) {
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)1);
			for (ParticlePassNode *n = m_atomics; n; n = n->m_next) {
				PTANK_PLUGIN(n->m_atomic)->srcBlend = 1;
				PTANK_PLUGIN(n->m_atomic)->dstBlend = 2;
				PTANK_PLUGIN(n->m_atomic)->instFlags |= 0x10000000;
				n->m_atomic->renderCallBack(n->m_atomic);
				PTANK_PLUGIN(n->m_atomic)->srcBlend = m_srcBlend;
				PTANK_PLUGIN(n->m_atomic)->dstBlend = m_dstBlend;
				PTANK_PLUGIN(n->m_atomic)->instFlags |= 0x10000000;
			}
		}
		SetBlendAndAlphaTest(rwBLENDSRCALPHA, rwBLENDINVSRCALPHA, 2);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)(intptr_t)zWrite);
		if (zTest == 0)
			RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
	}
}
