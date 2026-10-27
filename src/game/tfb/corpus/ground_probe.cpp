// Ground probe of a body's physics record.
#define ZOOSTERS_RWS_SENDMSG 1
#include "tfb/corpus/corpus.h"
#include "tfb/level/level.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus_private.h"

RpCollisionTriangle *Corpus_GroundProbeCallback(RpIntersection *intersection, RpWorldSector *sector, RpCollisionTriangle *tri,
                                                RwReal dist, void *data);   // 0x423320

// The world query of a body's probe: the ground direction (up while airborne, else the ground normal) and the triangle flags on.
// 0x4240c0
static __declspec(noinline) void Corpus_GroundProbe(CorpusProbe *p, RpIntersection *isect)
{
	RwV3d *up = &p->m_up;
	up->x = 0.0f;
	up->y = 1.0f;
	up->z = 0.0f;
	const RwV3d *src = (p->m_corpus->m_flags & CORPUS_AIRBORNE) ? up : &p->m_corpus->m_groundNormal;
	p->m_groundDir = *src;
	p->m_hitAtomic = 0;
	p->m_useTriFlags = 1;
	if (RWS::CLevel::sm_pActiveWorld)
		RpCollisionWorldForAllIntersections(RWS::CLevel::sm_pActiveWorld, isect, Corpus_GroundProbeCallback, p);
}
void g_keepGroundProbe(CorpusProbe *p, RpIntersection *i) { Corpus_GroundProbe(p, i); }

// Probing pass of a body (HandleEvents with m_firstPass set): sets up the record, then queries the world in the body's bounds plus one unit (not while riding).
// A moving walker's box is shortened by 9 from below (from 12 on) so that it steps over small edges; the second ground ray starts 0.75 above the ground.
// 0x424140
void Corpus::ResolveGroundContact(void *rec)
{
	CorpusProbe *p = (CorpusProbe *)rec;
	RpIntersection isect;
	if (m_flags & CORPUS_STEP_DONE)
		return;
	m_hitCarrier = 0;
	m_flags &= ~(CORPUS_HIT_LEDGE | CORPUS_HIT_TRIGGER | CORPUS_GROUND_FOUND);
	m_owner->UpdateBounds(&m_frame->modelling, p->m_t);
	p->m_move = m_owner->m_velocity;
	RwV3d *d = &p->m_pos;
	const RwV3d *s = &m_frame->modelling.pos;
	d->x = s->x;
	d->y = s->y;
	d->z = s->z;
	p->m_box = m_owner->m_worldBox;
	if ((m_flags & (CORPUS_LEFT_GROUND | CORPUS_AIRBORNE)) != CORPUS_AIRBORNE) {
		if (m_flags & CORPUS_MOVING) {
			if (m_owner->m_params.m_groundAccel != g_floatZero || m_owner->m_params.m_moveType != RWS::TFB::ACTOR_MOVE_FREE) {
				if (p->m_box.m_extent[1] >= 12.0f) {
					RwV3d s;
					s.x = p->m_box.m_frame.up.x * 9.0f;
					s.y = p->m_box.m_frame.up.y * 9.0f;
					s.z = p->m_box.m_frame.up.z * 9.0f;
					p->m_box.m_frame.pos.x += s.x;
					p->m_box.m_frame.pos.y += s.y;
					p->m_box.m_frame.pos.z += s.z;
					p->m_box.m_extent[1] -= 9.0f;
				}
			}
		}
		RwV3d v;
		v.x = (m_groundNormal.x * 0.75f);   // the parentheses move the fld of v.y before the push of &t
		v.y = m_groundNormal.y * 0.75f;
		v.z = m_groundNormal.z * 0.75f;
		p->m_pos2 = V3Add(d, &v);
	}
	p->m_corpus = this;
	if (m_link.m_carrier && m_link.m_index < 0)
		return;
	isect.type = rpINTERSECTBOX;
	isect.t.box.sup = m_owner->m_max.m_point;
	isect.t.box.inf = m_owner->m_min.m_point;
	isect.t.box.inf.x = isect.t.box.inf.x - 1.0f;
	isect.t.box.inf.y = isect.t.box.inf.y - 1.0f;
	isect.t.box.inf.z = isect.t.box.inf.z - 1.0f;
	isect.t.box.sup.x = isect.t.box.sup.x + 1.0f;
	isect.t.box.sup.y = isect.t.box.sup.y + 1.0f;
	isect.t.box.sup.z = isect.t.box.sup.z + 1.0f;
	Corpus_GroundProbe(p, &isect);
}

// The world query's callback: the triangle sweep and, when it found the body's ground (a floor with prelight colours, full move, no ground yet), the ground colour: the prelight colour at the hit, halved towards grey.
// 0x423320
RpCollisionTriangle *Corpus_GroundProbeCallback(RpIntersection *intersection, RpWorldSector *sector, RpCollisionTriangle *tri,
                                                RwReal dist, void *data)
{
	CorpusProbe *p = (CorpusProbe *)data;
	const RpWorldSectorView *view = (const RpWorldSectorView *)sector;
	bool ok;
	RwV3d c0, c1, c2, t1, t2;
	RwV3d A, B;
	p->m_triFlags = (unsigned char *)view->triangles;
	if (p->m_t == p->m_rem && !(p->m_corpus->m_flags & CORPUS_GROUND_FOUND))
		ok = true;
	else
		ok = false;
	RpCollisionTriangle *hit = Corpus_SphereTriangleSweepD(intersection, tri, dist, p);
	if (hit && ok) {
		Corpus *c = p->m_corpus;
		const unsigned char *preLit = (const unsigned char *)view->preLitLum;
		if ((c->m_flags & CORPUS_GROUND_FOUND) && preLit && hit->normal.y >= 0.7071f) {
			const unsigned short *ix = (const unsigned short *)(view->triangles + hit->index * 8);
			const unsigned char *lum = preLit + ix[0] * 4;
			c0.x = (float)lum[0];
			c0.y = (float)lum[1];
			c0.z = (float)lum[2];
			lum = preLit + ix[1] * 4;
			c1.x = (float)lum[0];
			c1.y = (float)lum[1];
			c1.z = (float)lum[2];
			lum = preLit + ix[2] * 4;
			c2.x = (float)lum[0];
			c2.y = (float)lum[1];
			c2.z = (float)lum[2];
			A = V3Sub(&c1, &c0);
			B = V3Sub(&c2, &c0);
			c->m_groundColor[0] = (unsigned char)((A.x * c->m_link.m_u + B.x * c->m_link.m_v + c0.x - 128.0f) * 0.5f + 128.0f);
			c = p->m_corpus;
			c->m_groundColor[1] = (unsigned char)((A.y * c->m_link.m_u + B.y * c->m_link.m_v + c0.y - 128.0f) * 0.5f + 128.0f);
			c = p->m_corpus;
			c->m_groundColor[2] = (unsigned char)((A.z * c->m_link.m_u + B.z * c->m_link.m_v + c0.z - 128.0f) * 0.5f + 128.0f);
		}
	}
	return hit;
}
