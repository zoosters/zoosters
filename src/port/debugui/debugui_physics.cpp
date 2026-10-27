// Physics timeline and collision overlay (ZOOSTERS_DEBUGUI). Read-only: sampled once per presented frame from the controlled actor's
// Corpus and Collider; world collision is queried with the game's own RpCollisionWorldForAllIntersections.
// Flags (physics step: src/game/tfb/corpus/physics_step.cpp + corpus_events.cpp):
//   * EMBED   the body started a sub-step inside a surface: pushed 1.5 units out along the (summed) contact normal, and touches at
//             time 0 are ignored for the rest of that tick (Collider::m_embedded). Wedging repeats it every tick.
//   * STEEP   standing (not airborne) on a "ground" whose normal is steeper than 45 degrees (y < 0.7071): the steep-slope guard
//             (FollowGroundHit) only runs while the ground is flat, and the end of the step turns the velocity onto this ground
//             (EndStep), so moving into such a surface becomes vertical speed.
//   * LAUNCH  the velocity changed by more than the threshold in one frame.
//   * PUSH    the position moved more than the velocity explains (a push-out or a teleport).
#ifndef ZOOSTERS_DEBUGUI
#error debugui_physics.cpp is built only with ZOOSTERS_DEBUGUI (build option --debugui)
#endif
#include "debugui_platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imgui.h"
#include "debugui.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "tfb/collision/collider.h"
#include "tfb/collision/broadphase.h"
#include "rw/rpcollis.h"
#include "rw/rpsectorview.h"
#include "tfb/level/level.h"   // RWS::CLevel::sm_pActiveWorld (0x62972c)

using RWS::TFB::GameActor;
using RWS::TFB::Collider;
extern float g_frameDt;                  // GLOBAL: 0x618318 g_frameDt
extern "C" int port_frames_presented(void);

// ---------------------------------------------------------------- per-frame samples
enum { EV_EMBED = 1, EV_STEEP = 2, EV_LAUNCH = 4, EV_PUSH = 8 };
struct Snap {
	int frame;
	float pos[3], vel[3], mv[3], gn[3];   // vel = the body's velocity (Corpus), mv = the actor's move velocity (Collider; per second, turned
	                                      // onto the ground by EndStep: what moves the body in the next tick)
	unsigned flags;
	float pushed;      // |position change - velocity * dt|
	int events;
};
enum { RING = 600, CAP_BEFORE = 120, CAP_AFTER = 60, MAX_CAPTURES = 6 };
static Snap g_ring[RING];
static int g_ringPos, g_ringCount;
struct Capture { char label[64]; Snap rows[CAP_BEFORE + CAP_AFTER]; int n; };
static Capture g_caps[MAX_CAPTURES];
static int g_ncaps, g_capSel = -1;
static int g_pendingCapture = -1;            // frame at which the pending capture is taken
static char g_pendingLabel[64];
static float g_launchThreshold = 500.0f;     // units per second of velocity change in one frame
static float g_pushThreshold = 1.0f;         // units of unexplained movement
static bool g_capOn[4] = { true, true, true, false };   // which events start a capture: EMBED STEEP LAUNCH PUSH
static const char *const kEvNames[4] = { "EMBED", "STEEP", "LAUNCH", "PUSH" };
static char g_lastExport[MAX_PATH];

static Corpus *playerBody(void) { GameActor *a = port_debugui_player(); return a ? a->m_corpus : 0; }

static void eventText(int ev, char *buf, size_t n)
{
	buf[0] = 0;
	for (int k = 0; k < 4; k++)
		if (ev & (1 << k)) { size_t l = strlen(buf); _snprintf(buf + l, n - l - 1, "%s%s", l ? " " : "", kEvNames[k]); }
	buf[n - 1] = 0;
}

static void takeCapture(void)
{
	Capture &c = g_caps[g_ncaps < MAX_CAPTURES ? g_ncaps++ : (memmove(g_caps, g_caps + 1, sizeof(Capture) * (MAX_CAPTURES - 1)), MAX_CAPTURES - 1)];
	strncpy(c.label, g_pendingLabel, sizeof c.label - 1);
	c.label[sizeof c.label - 1] = 0;
	int n = g_ringCount < CAP_BEFORE + CAP_AFTER ? g_ringCount : CAP_BEFORE + CAP_AFTER;
	for (int i = 0; i < n; i++) c.rows[i] = g_ring[(g_ringPos - n + i + RING) % RING];
	c.n = n;
	g_capSel = (int)(&c - g_caps);
}

static void exportCapture(const Capture &c);

void port_debugui_physics_tick(void)
{
	Corpus *c = playerBody();
	int f = port_frames_presented();
	if (g_pendingCapture >= 0 && f >= g_pendingCapture) { takeCapture(); g_pendingCapture = -1; }
	if (!c) return;
	const Snap *prev = g_ringCount ? &g_ring[(g_ringPos - 1 + RING) % RING] : 0;
	Snap &s = g_ring[g_ringPos];
	s.frame = f;
	const RwMatrix &m = c->m_lastFrame;
	s.pos[0] = m.pos.x; s.pos[1] = m.pos.y; s.pos[2] = m.pos.z;
	s.vel[0] = c->m_velocity.x; s.vel[1] = c->m_velocity.y; s.vel[2] = c->m_velocity.z;
	const RwV3d &mv = static_cast<Collider *>(c->m_owner)->m_velocity;
	s.mv[0] = mv.x; s.mv[1] = mv.y; s.mv[2] = mv.z;
	s.gn[0] = c->m_groundNormal.x; s.gn[1] = c->m_groundNormal.y; s.gn[2] = c->m_groundNormal.z;
	s.flags = c->m_flags;
	s.pushed = 0.0f;
	s.events = 0;
	if (c->m_owner && c->m_owner->m_embedded) s.events |= EV_EMBED;
	if (!(c->m_flags & CORPUS_AIRBORNE) && s.gn[1] < 0.7071f) s.events |= EV_STEEP;
	if (prev && prev->frame == f - 1) {
		float dv = 0.0f, best = 1e30f;
		for (int k = 0; k < 3; k++) dv += (s.mv[k] - prev->mv[k]) * (s.mv[k] - prev->mv[k]);
		if (sqrtf(dv) > g_launchThreshold) s.events |= EV_LAUNCH;
		for (int which = 0; which < 2; which++) {   // the move of this frame against the velocity before and after it
			const float *v = which ? s.mv : prev->mv;
			float e = 0.0f;
			for (int k = 0; k < 3; k++) { float d = (s.pos[k] - prev->pos[k]) - v[k] * g_frameDt; e += d * d; }
			if (e < best) best = e;
		}
		s.pushed = sqrtf(best);
		if (s.pushed > g_pushThreshold) s.events |= EV_PUSH;
	}
	g_ringPos = (g_ringPos + 1) % RING;
	if (g_ringCount < RING) g_ringCount++;
	static int dumpAt = getenv("PORT_DEBUGUI_CAPTURE") ? atoi(getenv("PORT_DEBUGUI_CAPTURE")) : -1;   // test aid: capture + export then
	if (dumpAt >= 0 && f >= dumpAt) {
		dumpAt = -1;
		_snprintf(g_pendingLabel, sizeof g_pendingLabel - 1, "frame %d: PORT_DEBUGUI_CAPTURE", f);
		takeCapture();
		exportCapture(g_caps[g_capSel]);
	}
	int trig = 0;
	for (int k = 0; k < 4; k++) if (g_capOn[k] && (s.events & (1 << k))) trig |= 1 << k;
	if (trig && g_pendingCapture < 0) {
		char ev[64];
		eventText(trig, ev, sizeof ev);
		_snprintf(g_pendingLabel, sizeof g_pendingLabel - 1, "frame %d: %s", f, ev);
		g_pendingLabel[sizeof g_pendingLabel - 1] = 0;
		g_pendingCapture = f + CAP_AFTER;
	}
}

static void exportCapture(const Capture &c)
{
	char name[64], path[1024];
	snprintf(name, sizeof name, "physics_capture_%d.tsv", c.n ? c.rows[0].frame : 0);
	if (!port_debugui_file(name, path, sizeof path)) return;
	FILE *f = fopen(path, "w");
	if (!f) return;
	fprintf(f, "# %s\nframe\tpos.x\tpos.y\tpos.z\tvel.x\tvel.y\tvel.z\tmove.x\tmove.y\tmove.z\tground.x\tground.y\tground.z\tflags\tpushed\tevents\n", c.label);
	for (int i = 0; i < c.n; i++) {
		const Snap &s = c.rows[i];
		char ev[64];
		eventText(s.events, ev, sizeof ev);
		fprintf(f, "%d\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.4f\t%.4f\t%.4f\t%08x\t%.3f\t%s\n", s.frame, s.pos[0], s.pos[1], s.pos[2],
		        s.vel[0], s.vel[1], s.vel[2], s.mv[0], s.mv[1], s.mv[2], s.gn[0], s.gn[1], s.gn[2], s.flags, s.pushed, ev);
	}
	fclose(f);
	strncpy(g_lastExport, path, sizeof g_lastExport - 1);
}

static void helpMark(const char *text)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip()) { ImGui::PushTextWrapPos(ImGui::GetFontSize() * 32.0f); ImGui::TextUnformatted(text); ImGui::PopTextWrapPos(); ImGui::EndTooltip(); }
}

static void snapTable(const Snap *rows, int n, float height)
{
	if (!ImGui::BeginTable("snaps", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg,
	                       ImVec2(0, height)))
		return;
	ImGui::TableSetupScrollFreeze(0, 1);
	ImGui::TableSetupColumn("frame"); ImGui::TableSetupColumn("pos"); ImGui::TableSetupColumn("body vel"); ImGui::TableSetupColumn("move vel");
	ImGui::TableSetupColumn("ground n"); ImGui::TableSetupColumn("air"); ImGui::TableSetupColumn("pushed"); ImGui::TableSetupColumn("events");
	ImGui::TableHeadersRow();
	ImGuiListClipper clip;
	clip.Begin(n);
	while (clip.Step())
		for (int i = clip.DisplayStart; i < clip.DisplayEnd; i++) {
			const Snap &s = rows[i];
			ImGui::TableNextRow();
			if (s.events) {
				ImU32 bg = (s.events & EV_LAUNCH) ? IM_COL32(150, 130, 0, 110) : (s.events & EV_STEEP) ? IM_COL32(170, 90, 0, 110)
				         : (s.events & EV_EMBED) ? IM_COL32(150, 30, 30, 110) : IM_COL32(60, 60, 140, 110);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, bg);
			}
			char ev[64];
			eventText(s.events, ev, sizeof ev);
			ImGui::TableNextColumn(); ImGui::Text("%d", s.frame);
			ImGui::TableNextColumn(); ImGui::Text("%8.1f %7.1f %8.1f", s.pos[0], s.pos[1], s.pos[2]);
			ImGui::TableNextColumn(); ImGui::Text("%7.1f %7.1f %7.1f", s.vel[0], s.vel[1], s.vel[2]);
			ImGui::TableNextColumn(); ImGui::Text("%7.1f %7.1f %7.1f", s.mv[0], s.mv[1], s.mv[2]);
			ImGui::TableNextColumn(); ImGui::Text("%5.2f %5.2f %5.2f", s.gn[0], s.gn[1], s.gn[2]);
			ImGui::TableNextColumn(); ImGui::TextUnformatted(s.flags & CORPUS_AIRBORNE ? "air" : "gnd");
			ImGui::TableNextColumn(); ImGui::Text("%.2f", s.pushed);
			ImGui::TableNextColumn(); ImGui::TextUnformatted(ev);
		}
	ImGui::EndTable();
}

void port_debugui_timeline_panel(int open)
{
	// the last 300 frames as plots
	static float hs[300], vs[300], gy[300], pu[300];
	int n = g_ringCount < 300 ? g_ringCount : 300;
	for (int i = 0; i < n; i++) {
		const Snap &s = g_ring[(g_ringPos - n + i + RING) % RING];
		hs[i] = sqrtf(s.mv[0] * s.mv[0] + s.mv[2] * s.mv[2]); vs[i] = s.mv[1]; gy[i] = (s.flags & CORPUS_AIRBORNE) ? 1.0f : s.gn[1]; pu[i] = s.pushed;
	}
	const Snap *last = g_ringCount ? &g_ring[(g_ringPos - 1 + RING) % RING] : 0;
	char ev[64] = "";
	if (last) eventText(last->events, ev, sizeof ev);
	ImGui::Text("now: %s%s", last && !(last->flags & CORPUS_AIRBORNE) ? "ground" : "air", ev[0] ? "  " : "");
	if (ev[0]) { ImGui::SameLine(0, 0); ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "%s", ev); }
	if (!open) return;
	helpMark("EMBED: started a physics sub-step inside a surface (pushed 1.5 units out along the summed contact normals; contacts at "
	         "time 0 ignored for the rest of the tick). STEEP: standing on a 'ground' steeper than 45 degrees (the steep-slope guard is "
	         "off and the velocity is turned onto that surface). LAUNCH: velocity changed by more than the threshold in one frame. "
	         "PUSH: moved more than the velocity explains.");
	ImVec2 sz(360, 40);
	ImGui::PlotLines("move horiz", hs, n, 0, 0, 0.0f, 1000.0f, sz);
	ImGui::PlotLines("move vert", vs, n, 0, 0, -800.0f, 800.0f, sz);
	ImGui::PlotLines("ground n.y", gy, n, 0, "(1 in the air)", -1.0f, 1.0f, sz);
	ImGui::PlotLines("pushed", pu, n, 0, 0, 0.0f, 10.0f, sz);
	ImGui::SliderFloat("launch dv", &g_launchThreshold, 100.0f, 3000.0f, "%.0f u/s");
	ImGui::SliderFloat("push", &g_pushThreshold, 0.1f, 20.0f, "%.1f u");
	ImGui::Text("capture on:");
	for (int k = 0; k < 4; k++) { ImGui::SameLine(); ImGui::Checkbox(kEvNames[k], &g_capOn[k]); }
	if (g_pendingCapture >= 0) ImGui::TextColored(ImVec4(1, 1, 0, 1), "capturing: %s (until frame %d)", g_pendingLabel, g_pendingCapture);
	if (ImGui::Button("capture now")) { _snprintf(g_pendingLabel, sizeof g_pendingLabel - 1, "frame %d: manual", port_frames_presented()); g_pendingCapture = port_frames_presented(); }
	ImGui::SameLine();
	if (ImGui::Button("clear captures")) { g_ncaps = 0; g_capSel = -1; }
	if (g_ncaps) {
		if (g_capSel < 0 || g_capSel >= g_ncaps) g_capSel = g_ncaps - 1;
		if (ImGui::BeginCombo("capture", g_caps[g_capSel].label)) {
			for (int i = 0; i < g_ncaps; i++) if (ImGui::Selectable(g_caps[i].label, i == g_capSel)) g_capSel = i;
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button("export TSV")) exportCapture(g_caps[g_capSel]);
		if (g_lastExport[0]) ImGui::TextDisabled("%s", g_lastExport);
		snapTable(g_caps[g_capSel].rows, g_caps[g_capSel].n, 260.0f);
	} else {
		ImGui::TextDisabled("no capture yet (one is taken %d frames after an event)", (int)CAP_AFTER);
	}
}

// ---------------------------------------------------------------- collision overlay (3D lines)
static bool g_showTris = true, g_showBoxes = true, g_showArrows = true;
static float g_radius = 60.0f;
static int g_triCount[5];        // floor, steep, ceiling, trigger, ledge (drawn this frame)
static int g_boxCount;

enum { TRI_FLOOR, TRI_STEEP, TRI_CEILING, TRI_TRIGGER, TRI_LEDGE };
static const unsigned kTriColour[5] = { 0x40d040b0, 0xff9020e0, 0x4080ffb0, 0xff40ffff, 0x40ffffff };
static const char *const kTriNames[5] = { "floor (n.y >= 0.7071)", "steep / wall", "ceiling (n.y < -0.2)", "trigger flag (1)", "ledge flag (0x20)" };

struct TriQuery { int n, max; };
static RpCollisionTriangle *triCallback(RpIntersection *, RpWorldSector *sector, RpCollisionTriangle *tri, RwReal, void *data)
{
	TriQuery *q = (TriQuery *)data;
	if (q->n >= q->max) return 0;
	q->n++;
	const RpWorldSectorView *view = (const RpWorldSectorView *)sector;
	unsigned char flags = view && view->triangles ? (unsigned char)view->triangles[tri->index * 8 + 6] : 0;
	int kind = (flags & 1) ? TRI_TRIGGER : (flags & 0x20) ? TRI_LEDGE : tri->normal.y >= 0.7071f ? TRI_FLOOR : tri->normal.y < -0.2f ? TRI_CEILING : TRI_STEEP;
	g_triCount[kind]++;
	for (int e = 0; e < 3; e++) {
		const RwV3d *a = tri->vertices[e], *b = tri->vertices[(e + 1) % 3];
		port_debugui_line(&a->x, &b->x, kTriColour[kind]);
	}
	return tri;
}

static void boxLines(const CollisionBox &box, unsigned rgba)
{
	const RwMatrix &m = box.m_frame;
	float corner[8][3];
	for (int i = 0; i < 8; i++) {
		float sx = (i & 1) ? 1.0f : -1.0f, sy = (i & 2) ? 1.0f : -1.0f, sz = (i & 4) ? 1.0f : -1.0f;
		corner[i][0] = m.pos.x + sx * box.m_extent[0] * m.right.x + sy * box.m_extent[1] * m.up.x + sz * box.m_extent[2] * m.at.x;
		corner[i][1] = m.pos.y + sx * box.m_extent[0] * m.right.y + sy * box.m_extent[1] * m.up.y + sz * box.m_extent[2] * m.at.y;
		corner[i][2] = m.pos.z + sx * box.m_extent[0] * m.right.z + sy * box.m_extent[1] * m.up.z + sz * box.m_extent[2] * m.at.z;
	}
	static const int edges[12][2] = { {0,1},{2,3},{4,5},{6,7}, {0,2},{1,3},{4,6},{5,7}, {0,4},{1,5},{2,6},{3,7} };
	for (int e = 0; e < 12; e++) port_debugui_line(corner[edges[e][0]], corner[edges[e][1]], rgba);
}

static void arrow(const float *from, const float *dir, float len, unsigned rgba)
{
	float to[3] = { from[0] + dir[0] * len, from[1] + dir[1] * len, from[2] + dir[2] * len };
	port_debugui_line(from, to, rgba);
}

// every frame while the Collision panel is switched on (shown), pinned or not, overlay open or not
void port_debugui_collision_draw(void)
{
	memset(g_triCount, 0, sizeof g_triCount);
	g_boxCount = 0;
	if (!g_showTris && !g_showBoxes && !g_showArrows) return;
	GameActor *act = port_debugui_player();
	Corpus *c = act ? act->m_corpus : 0;
	if (!c) return;
	const RwV3d &p = c->m_lastFrame.pos;
	if (g_showTris && RWS::CLevel::sm_pActiveWorld) {
		RpIntersection is;
		memset(&is, 0, sizeof is);
		is.type = rpINTERSECTSPHERE;
		is.t.sphere.center = p;
		is.t.sphere.radius = g_radius;
		TriQuery q = { 0, 20000 };
		RpCollisionWorldForAllIntersections(RWS::CLevel::sm_pActiveWorld, &is, triCallback, &q);
	}
	if (g_showBoxes) {
		const float r2 = (g_radius * 3.0f) * (g_radius * 3.0f);
		for (int i = 0; i < g_boundsCount && i < RWS::TFB::BROADPHASE_MAX_BOUNDS; i++) {
			RWS::TFB::ColliderBound *b = g_boundsItems[0][i];
			Collider *col = b ? b->m_collider : 0;
			if (!col || b != &col->m_min) continue;        // each collider once (its min corner)
			const RwV3d &cp = col->m_worldBox.m_frame.pos;
			float dx = cp.x - p.x, dy = cp.y - p.y, dz = cp.z - p.z;
			if (dx * dx + dy * dy + dz * dz > r2) continue;
			bool me = col == static_cast<Collider *>(act);
			unsigned rgba = me ? (col->m_embedded ? 0xff3030ff : 0xffff40ff) : (RWS::TFB::ActorOf(*col).m_corpus ? 0x80c0ffd0 : 0xe0e0e0c0);
			boxLines(col->m_worldBox, rgba);
			g_boxCount++;
		}
	}
	if (g_showArrows) {
		const float cp[3] = { c->m_contactPoint.x, c->m_contactPoint.y, c->m_contactPoint.z };
		const float gn[3] = { c->m_groundNormal.x, c->m_groundNormal.y, c->m_groundNormal.z };
		arrow(cp, gn, 25.0f, (c->m_flags & CORPUS_AIRBORNE) ? 0xffff4080 : (gn[1] < 0.7071f ? 0xff3030ff : 0xffff40ff));
		const float pos[3] = { p.x, p.y, p.z };
		const float v[3] = { c->m_velocity.x, c->m_velocity.y, c->m_velocity.z };
		arrow(pos, v, 0.1f, 0xffffffff);                   // where it gets in 0.1 s
	}
}

void port_debugui_collision_panel(int open)
{
	if (!open) { ImGui::Text("tris %d  boxes %d", g_triCount[0] + g_triCount[1] + g_triCount[2] + g_triCount[3] + g_triCount[4], g_boxCount); return; }
	ImGui::Checkbox("world triangles", &g_showTris); ImGui::SameLine();
	ImGui::Checkbox("actor boxes", &g_showBoxes); ImGui::SameLine();
	ImGui::Checkbox("arrows", &g_showArrows);
	bool xray = port_debugui_xray != 0;
	if (ImGui::Checkbox("x-ray (through walls)", &xray)) port_debugui_xray = xray;
	ImGui::SliderFloat("radius", &g_radius, 10.0f, 400.0f, "%.0f units");
	for (int k = 0; k < 5; k++) {
		unsigned c = kTriColour[k];
		ImGui::ColorButton(kTriNames[k], ImVec4((c >> 24 & 0xff) / 255.0f, (c >> 16 & 0xff) / 255.0f, (c >> 8 & 0xff) / 255.0f, 1.0f),
		                   ImGuiColorEditFlags_NoTooltip, ImVec2(12, 12));
		ImGui::SameLine();
		ImGui::Text("%-22s %d", kTriNames[k], g_triCount[k]);
	}
	ImGui::Text("boxes %d: you yellow (red while embedded), bodies blue, props grey", g_boxCount);
	ImGui::TextDisabled("arrows: ground normal at the contact point (red = steep 'ground'), velocity (0.1 s)");
}
