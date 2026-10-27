// Semantic trace for x86-vs-x64 comparison (normal builds only). PORT_SEMTRACE=<file>: once per presented frame a block of lines with
// values read through real member accesses (no offsets, no sizeof): player actors' position / corpus state, actors per script set,
// collision query counts. The same source builds for x86 and x64, so the logs diff line by line (floats with 3 decimals, no pointers).
// Chained behind the other present hooks like gtrace.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rw/rwcore.h"
#include "script/referenceset.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "modules/tfb/gamecamera.h"
#include "modules/fx/gameparticle.h"

extern "C" int port_frames_presented(void);
extern "C" void port_rng_stats(unsigned *calls, unsigned *seeds, unsigned long long *hash);   // gtrace_port.cpp
extern "C" void (*port_present_hook)(void);
extern "C" unsigned port_collq_world[8], port_collq_world_hits[8], port_collq_atomic[8];   // shim/rwcollis.inc
extern float g_frameDt;                         // 0x618318 (tfb/corpus/corpus_private.h)

extern "C" unsigned port_rq[24];   // render-queue counters (src/game PORT_RQ): 0 AddAtomic calls, 2 occluder-culled, 3 passed, 4 Update false, 8 sector queue walks, 9 ties, 10 flag4 missing, 11 stamp skip, 12 queued by sector walk, 14 yon cells tested, 15 yon cells culled, 16 yon leaf atomics, 17 yon stamp skip, 18/19 corpus callers
unsigned port_rq[24];
static unsigned g_prevRq[24];
static FILE *g_sem;
static void (*g_prev)(void);
static unsigned g_prevWorld[8], g_prevHits[8], g_prevAtomic[8];

using RWS::TFB::GameActor;

static void actorLine(const char *tag, int idx, GameActor *a)
{
	const RwMatrix &m = a->m_matrix;
	fprintf(g_sem, "%s%d  params ext %.3f %.3f %.3f off %.3f %.3f %.3f flags %x move %d groundAccel %.3f | box local %.3f %.3f %.3f world %.3f %.3f %.3f | bounds min %.3f %.3f %.3f max %.3f %.3f %.3f vel %.3f %.3f %.3f ctlmode %d\n", tag, idx,
	        a->m_params.m_xExtent, a->m_params.m_zExtent, a->m_params.m_yExtent, a->m_params.m_xOffset, a->m_params.m_zOffset, a->m_params.m_yOffset, a->m_params.m_flags, a->m_params.m_moveType, a->m_params.m_groundAccel,
	        a->m_localBox.m_extent[0], a->m_localBox.m_extent[1], a->m_localBox.m_extent[2], a->m_worldBox.m_extent[0], a->m_worldBox.m_extent[1], a->m_worldBox.m_extent[2],
	        a->m_min.m_point.x, a->m_min.m_point.y, a->m_min.m_point.z, a->m_max.m_point.x, a->m_max.m_point.y, a->m_max.m_point.z, a->m_velocity.x, a->m_velocity.y, a->m_velocity.z, a->m_controlMode);
	Corpus *c = a->m_corpus;
	fprintf(g_sem, "%s%d matrix %.3f %.3f %.3f facing %.3f corpus %d\n", tag, idx, m.pos.x, m.pos.y, m.pos.z, a->m_facing, c ? 1 : 0);
	if (!c) return;
	const RwMatrix *fm = c->m_frame ? RwFrameGetMatrix(c->m_frame) : 0;
	fprintf(g_sem, "%s%d  frame %.3f %.3f %.3f lastFrame %.3f %.3f %.3f vel %.3f %.3f %.3f\n", tag, idx,
	        fm ? fm->pos.x : 0.0f, fm ? fm->pos.y : 0.0f, fm ? fm->pos.z : 0.0f, c->m_lastFrame.pos.x, c->m_lastFrame.pos.y, c->m_lastFrame.pos.z,
	        c->m_velocity.x, c->m_velocity.y, c->m_velocity.z);
	fprintf(g_sem, "%s%d  flags %08x ground %d air %d contact %.3f %.3f %.3f normal %.3f %.3f %.3f stateTime %.3f link %d surf %d ctl %d\n", tag, idx,
	        c->m_flags, (c->m_flags & CORPUS_GROUND_FOUND) ? 1 : 0, (c->m_flags & CORPUS_AIRBORNE) ? 1 : 0,
	        c->m_contactPoint.x, c->m_contactPoint.y, c->m_contactPoint.z, c->m_groundNormal.x, c->m_groundNormal.y, c->m_groundNormal.z,
	        c->m_stateTime, c->m_link.m_index, c->m_surface, c->m_controller);
}

#ifdef _WIN64
extern "C" void port_watch_arm(void *addr);   // port/fault64.cpp (PORT_SEMWATCH)
#endif
// count and (up to `maxList`) list the members of a script set
static int walkSet(const char *tag, Script::ScriptReference *setRef, int maxList)
{
	Script::ReferenceSet *set = (Script::ReferenceSet *)setRef;
	if (!set) return -1;
	int n = 0;
	float minY = 1e30f, maxY = -1e30f;
	for (Script::SetNode *node = set->m_head; node; node = node->m_next, n++) {
		GameActor *a = static_cast<GameActor *>(static_cast<Script::ScriptActor *>(node->m_object));
		if (!a) continue;
		float y = a->m_corpus ? RwFrameGetMatrix(a->m_corpus->m_frame)->pos.y : a->m_matrix.pos.y;
		if (y < minY) minY = y;
		if (y > maxY) maxY = y;
		if (n < maxList) actorLine(tag, n, a);
#ifdef _WIN64
		// PORT_SEMWATCH=<set><index>:<frame> (e.g. npc11:9): after that frame's dump, a hardware write watchpoint on the actor's corpus velocity
		// (x and y; port/fault64.cpp port_watch_arm): every write of the next frames is logged with its stack (compare two builds' writers)
		static const char *sw = getenv("PORT_SEMWATCH");
		if (sw && a->m_corpus) {
			char want[32]; int wf = -1;
			if (sscanf(sw, "%31[a-z]%*[^:]:%d", want, &wf) == 2 && !strcmp(want, tag) && atoi(sw + strlen(want)) == n && wf == port_frames_presented()) {
				port_watch_arm(&a->m_corpus->m_velocity);
			}
		}
#endif
	}
	fprintf(g_sem, "%s count %d (list count %d) y range %.3f .. %.3f\n", tag, n, set->m_count, n ? minY : 0.0f, n ? maxY : 0.0f);
	return n;
}

// ---- particles (`ptc` lines). The GameParticles of the particle shield's three sets (active, inactive, removed), in set order, each
// pool walked as the renderer sees it: busy tanks then idle tanks in list order, slots in index order. The hash is order-sensitive.
// Per live particle: slot, lifetime, position, velocity, size, colour, sub-texture UVs, quantised (positions / sizes 1/1000, velocities
// 1/100000, UVs 1/4096) so last-bit noise between compilers does not flip the digest. Read locks only.
// Active set: every system; inactive / removed: only those holding live particles.
//   ptc <set><i> src <1 original | 0 clone> vis <m_visible> en <emitting> sec <split on>/<child count>/<child count jitter>/<child age jitter>
//       max <pool capacity> blocks <busy>/<idle> live <n> prim <n> secp <n> h <fnv64>
// secp = particles past their split age; non-zero sec count = a system that spawns children.
// Then one `ptc sets` line: systems per set, live totals. The set getters construct nothing here: the sets exist from static
// initialisation on (TypeInitHost::Init registers TFBType_Particle, whose prototype reset walks all three).
namespace {
struct Fnv64 {
	unsigned long long h;
	Fnv64() : h(0xcbf29ce484222325ull) {}
	void u(unsigned v) { for (int i = 0; i < 4; i++, v >>= 8) h = (h ^ (v & 0xff)) * 0x100000001b3ull; }
	void q(float x, float scale)
	{
		double d = (double)x * scale;
		if (d != d) { u(0x7fc00000u); return; }                      // NaN
		if (d > 2.0e9) d = 2.0e9;
		if (d < -2.0e9) d = -2.0e9;
		long long r = (long long)(d < 0 ? d - 0.5 : d + 0.5);          // round half away from zero
		u((unsigned)(int)r);
	}
};
struct PtcTotals { int systems, live, prim, sec; };
}

static void particleSystem(char set, int idx, RWS::FX::GameParticle *p, PtcTotals &tot)
{
	typedef RWS::FX::GameParticle::Pool Pool;
	using RWS::Particle::CPB_TexPart;
	Pool *b = p->m_buf;
	tot.systems++;
	if (!b || !b->m_proc) {
		if (set == 'A') fprintf(g_sem, "ptc %c%d nobuf\n", set, idx);
		return;
	}
	CPB_TexPart *base = b->m_proc;
	Fnv64 h;
	int live = 0, prim = 0, sec = 0, nUsed = 0, nFree = 0;
	for (int list = 0; list < 2; list++) {
		for (Pool::Tank *n = list ? b->m_idle : b->m_busy; n; n = n->m_next) {
			(list ? nFree : nUsed)++;
			h.u(0xb10c0000u | (unsigned)list);
			h.u(n->m_ordinal);
			h.u(n->m_capacity);
			h.u(n->m_liveCount);
			RpPTankLockStruct pos = {0, 0}, col = {0, 0}, size = {0, 0}, uv = {0, 0};
			RpPTankAtomicLock(n->m_atomic, &pos, rpPTANKLFLAGPOSITION, rpPTANKLOCKREAD);
			RpPTankAtomicLock(n->m_atomic, &col, rpPTANKLFLAGCOLOR, rpPTANKLOCKREAD);
			RpPTankAtomicLock(n->m_atomic, &size, rpPTANKLFLAGSIZE, rpPTANKLOCKREAD);
			RpPTankAtomicLock(n->m_atomic, &uv, rpPTANKLFLAGVTX2TEXCOORDS, rpPTANKLOCKREAD);
			CPB_TexPart::Motion *anim = base->m_motionBlocks && n->m_ordinal < base->m_blockCount ? base->m_motionBlocks[n->m_ordinal] : 0;
			for (RwUInt32 i = 0; i < n->m_capacity; i++) {
				if (!n->m_live[i]) continue;
				live++;
				RwUInt32 life = n->m_ages[i];
				if (base->m_splitOn && life >= base->m_splitAge) sec++; else prim++;
				h.u(i);
				h.u(life);
				if (pos.data) { const RwV3d *v = (const RwV3d *)(pos.data + pos.stride * i); h.q(v->x, 1000.0f); h.q(v->y, 1000.0f); h.q(v->z, 1000.0f); }
				if (anim) { const RwV3d &v = anim[i].m_move; h.q(v.x, 100000.0f); h.q(v.y, 100000.0f); h.q(v.z, 100000.0f); }
				if (size.data) { const RwV2d *s = (const RwV2d *)(size.data + size.stride * i); h.q(s->x, 1000.0f); h.q(s->y, 1000.0f); }
				if (col.data) { const RwRGBA *c = (const RwRGBA *)(col.data + col.stride * i); h.u(c->red | c->green << 8 | c->blue << 16 | (unsigned)c->alpha << 24); }
				if (uv.data) { const RwV2d *t = (const RwV2d *)(uv.data + uv.stride * i); h.q(t[0].x, 4096.0f); h.q(t[0].y, 4096.0f); h.q(t[1].x, 4096.0f); h.q(t[1].y, 4096.0f); }
			}
		}
	}
	tot.live += live;
	tot.prim += prim;
	tot.sec += sec;
	if (set != 'A' && live == 0) return;
	fprintf(g_sem, "ptc %c%d src %d vis %d en %d sec %d/%u/%u/%u max %u blocks %d/%d live %d prim %d secp %d h %016llx\n", set, idx,
	        p->m_spawnSource == p ? 1 : 0, p->m_visible ? 1 : 0, base->m_emitting ? 1 : 0, base->m_splitOn ? 1 : 0, base->m_scaledChildCount,
	        base->m_scaledChildCountJitter, base->m_childAgeJitter, b->m_capacity, nUsed, nFree, live, prim, sec, h.h);
}

static void particleLines(void)
{
	struct { char tag; Script::ReferenceSet *set; } sets[3] = {
		{'A', TFB_GetActiveParticleSet()}, {'I', TFB_GetInactiveParticleSet()}, {'R', TFB_GetRemovedParticleSet()}};
	PtcTotals tot[3];
	memset(tot, 0, sizeof tot);
	for (int s = 0; s < 3; s++) {
		if (!sets[s].set) continue;
		int i = 0;
		for (Script::SetNode *node = sets[s].set->m_head; node; node = node->m_next, i++) {
			RWS::FX::GameParticle *p = static_cast<RWS::FX::GameParticle *>(node->m_object);
			if (p) particleSystem(sets[s].tag, i, p, tot[s]);
		}
	}
	fprintf(g_sem, "ptc sets");
	for (int s = 0; s < 3; s++) fprintf(g_sem, " %c %d/%d/%d/%d", sets[s].tag, tot[s].systems, tot[s].live, tot[s].prim, tot[s].sec);
	fprintf(g_sem, "\n");
}

// ---- random numbers (`rng` line): the game thread's rand calls this frame and since the start, srand calls, and the running FNV-1a hash of
// every rand value and srand seed (gtrace_port.cpp). The hash pins both count and order of the draws.
static unsigned g_prevRngCalls;
static void rngLine(void)
{
	unsigned calls, seeds;
	unsigned long long hash;
	port_rng_stats(&calls, &seeds, &hash);
	fprintf(g_sem, "rng calls %u total %u srand %u h %016llx\n", calls - g_prevRngCalls, calls, seeds, hash);
	g_prevRngCalls = calls;
}

static void semHook(void)
{
	if (g_prev) g_prev();
	if (!g_sem) return;
	int f = port_frames_presented();
	fprintf(g_sem, "frame %d\n", f);
	fprintf(g_sem, "camera pos %.3f %.3f %.3f tgt %.3f %.3f %.3f posCur %.3f %.3f %.3f tgtCur %.3f %.3f %.3f\n", g_camPos.x, g_camPos.y, g_camPos.z, g_camTgt.x, g_camTgt.y, g_camTgt.z,
	        g_camPosCur.x, g_camPosCur.y, g_camPosCur.z, g_camTgtCur.x, g_camTgtCur.y, g_camTgtCur.z);
	fprintf(g_sem, "camstate snap %d timeLeft %.4f total %.4f start %.4f mode %d aux %d dt %.5f pitchHold %d yawHold %d object %d", g_camSnapPending ? 1 : 0, g_camTimeLeft, g_camTimeTotal, g_camTimeStart, g_camMode, g_camModeAux, g_frameDt, g_camPitchHold, g_camYawHold, g_cameraObject ? 1 : 0);
	if (g_cameraObject) fprintf(g_sem, " eye %.3f %.3f %.3f focus %.3f %.3f %.3f", g_cameraObject->m_matrix.pos.x, g_cameraObject->m_matrix.pos.y, g_cameraObject->m_matrix.pos.z, g_cameraObject->m_focus.x, g_cameraObject->m_focus.y, g_cameraObject->m_focus.z);
	fprintf(g_sem, "\n");
	if (g_gameCamDst) {   // the RwCamera the game renders through: its frame matrix, view window, clip planes
		const RwMatrix *lm = RwFrameGetLTM(RwCameraGetFrame(g_gameCamDst));
		const RwV2d *vw = RwCameraGetViewWindow(g_gameCamDst);
		fprintf(g_sem, "rwcam pos %.3f %.3f %.3f at %.3f %.3f %.3f up %.3f %.3f %.3f right %.3f %.3f %.3f vw %.4f %.4f valid %d\n", lm->pos.x, lm->pos.y, lm->pos.z, lm->at.x, lm->at.y, lm->at.z,
		        lm->up.x, lm->up.y, lm->up.z, lm->right.x, lm->right.y, lm->right.z, vw->x, vw->y, g_gameCamValid ? 1 : 0);
	}
	walkSet("player", TFB_GetPlayerSet(), 2);
	walkSet("npc", TFB_GetNPCSet(), 64);
	walkSet("inactive", TFB_GetInactiveSet(), 0);
	walkSet("dead", TFB_GetDeadSet(), 0);
	fprintf(g_sem, "coll world");
	for (int t = 1; t < 6; t++) fprintf(g_sem, " t%d %u/%u", t, port_collq_world[t] - g_prevWorld[t], port_collq_world_hits[t] - g_prevHits[t]);
	fprintf(g_sem, " atomic");
	for (int t = 1; t < 6; t++) fprintf(g_sem, " t%d %u", t, port_collq_atomic[t] - g_prevAtomic[t]);
	fprintf(g_sem, "\n");
	fprintf(g_sem, "rq");
	for (int i = 0; i < 24; i++) if (i != 5 && i != 6 && i != 7 && i != 13 && i < 20) fprintf(g_sem, " %d:%u", i, port_rq[i] - g_prevRq[i]);
	fprintf(g_sem, "\n");
	particleLines();
	rngLine();
	memcpy(g_prevRq, port_rq, sizeof g_prevRq);
	memcpy(g_prevWorld, port_collq_world, sizeof g_prevWorld);
	memcpy(g_prevHits, port_collq_world_hits, sizeof g_prevHits);
	memcpy(g_prevAtomic, port_collq_atomic, sizeof g_prevAtomic);
	fflush(g_sem);
}

// PORT_SEMQ=<last frame> or <first>-<last>: one line per world query up to that presented frame (type, the primitive's six floats, tree walk or brute force, sectors / triangles visited, callback hits)
extern "C" void port_semtrace_query(int type, const float *six, int tree, int sectors, int tris, int hits)
{
	static const char *e = getenv("PORT_SEMQ");
	if (!g_sem || !e) return;
	int lo = 0, hi = 0;
	if (sscanf(e, "%d-%d", &lo, &hi) != 2) { lo = 0; hi = atoi(e); }   // PORT_SEMQ=<last> or <first>-<last>
	int f = port_frames_presented();
	if (f < lo || f > hi) return;
	fprintf(g_sem, "q type %d %.2f %.2f %.2f / %.2f %.2f %.2f tree %d sectors %d tris %d hits %d\n", type, six[0], six[1], six[2], six[3], six[4], six[5], tree, sectors, tris, hits);
}

extern "C" void port_semtrace_init(void)
{
	const char *path = getenv("PORT_SEMTRACE");
	if (!path || !*path) return;
	g_sem = fopen(path, "w");
	if (!g_sem) return;
	g_prev = port_present_hook;
	port_present_hook = &semHook;
}
