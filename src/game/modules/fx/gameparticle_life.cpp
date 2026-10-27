// The constructor inlines Placement's (ScriptParticle has none).
#include "modules/fx/gameparticle.h"
#include "modules/fx/partspray.h"
#include "modules/fx/attachpoint.h"
#include "modules/tfb/gameactor.h"
#include "rw/rwcore.h"
#include "tfb/render/occluder.h"
#include "modules/tfb/gamecamera.h"

namespace RWS { namespace FX {

// 0x40cfa0
GameParticle::GameParticle(CFXPartSpray *owner) : m_visible(1), m_refCount(0), m_owner(owner), m_ownsFrame(0), m_link()
{
	m_buf = RWS_NEW Pool(0, 0);
	m_spawnSource = this;
	m_visualRadius = owner->m_visualRadius;
	m_options = owner->m_options;
	m_link.m_owner = this;
}

// 0x40d090
// 0x40d690
GameParticle::~GameParticle()
{
	if (m_buf)
		delete m_buf;
	if (m_ownsFrame)
		FrameHelper::FrameDestroy(m_frame);
}
}}

// GLOBAL: 0x61811c g_renderQueueFreeList
// GLOBAL: 0x618110 g_renderQueueHead
// GLOBAL: 0x618118 g_renderQueueCount
// GLOBAL: 0x5ca758 g_twelveW3a
// GLOBAL: 0x40a620 ??3PartListNodeLS@@SAXPAX@Z
#include "modules/tfb/renderqueue.h"
namespace RWS { namespace FX { void QueueVisibleParticles(RwCamera *cam); } }
extern const float g_twelveW3a;

// Near-match of 0x40b3d0: register roles and the node reload differ; the game allocates with a raw RwEngineInstance+0x118 call, without a
// class operator delete EH funclet.
void RWS::FX::QueueVisibleParticles(RwCamera *cam)
{
	for (Script::SetNode *node = TFB_GetActiveParticleSet()->m_head; node; node = node->m_next) {
		GameParticle *p = static_cast<GameParticle *>(node->m_object);
		RWS::TFB::Collider *t = p->m_link.m_carrier;
		if (t) {
			if (t->m_clump != 0 && (unsigned)p->m_link.m_id >= (unsigned)RWS::TFB::ActorOf(*t).m_corpus->m_id) {
				RwMatrix *ltm = RwFrameGetLTM(p->m_link.m_frame);
				RwFrameTransform(p->m_frame, ltm, rwCOMBINEREPLACE);
				AttachPoint *point = p->m_link.m_attachPoint;
				if (point->m_dirty)
					point->RebuildLocalMatrix();
				RwFrameTransform(p->m_frame, &point->m_local, rwCOMBINEPRECONCAT);
			} else
				p->m_link.Release();
		}
		float sphere[4];
		sphere[0] = p->m_frame->modelling.pos.x;
		sphere[1] = p->m_frame->modelling.pos.y;
		sphere[2] = p->m_frame->modelling.pos.z;
		sphere[3] = p->m_visualRadius * g_twelveW3a;
		int vis = 0;
		if (RwCameraFrustumTestSphere(cam, reinterpret_cast<const RwSphere *>(sphere)))
			vis = OccluderTestSphere(cam, (SphereP413 *)sphere, 1);
		p->m_visible = vis != 0;
		if (vis != 0)
			new RenderQueueNode(p->m_buf, CameraSignedViewDistance(cam, (const RwV3d *)sphere), vis == 1 ? 3 : 2, 1, 0);   // particle pass kinds 3 / 2
	}
}

