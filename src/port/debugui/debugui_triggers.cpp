// Trigger zones (ZOOSTERS_DEBUGUI). Scripts test "is X near" with `check value <actor>, <=, N` (Script::ValueCheck::Execute 0x430580):
// the value is Script::PositionMeasurement::GetFloat 0x42f770, the 3D distance from the running script object (g_scriptSelf) in units
// of 12 world units, as the closest approach within the next 1/30 s (RWS::FX::PlayerObj::Dist 0x409700).
// Two #ifdef ZOOSTERS_DEBUGUI hooks in those functions report each check here; a "site" is one ValueCheck op in a script.
// Drawn as spheres of radius N * 12 around the script object: green = passed when it last ran, red = failed, grey = not run for 2 s
// (the script is in another state).
#ifndef ZOOSTERS_DEBUGUI
#error debugui_triggers.cpp is built only with ZOOSTERS_DEBUGUI (build option --debugui)
#endif
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <stdint.h>
#include "zoosters/os.h"   // MemoryReadable (nameOf)
#endif
#include "imgui.h"
#include "debugui.h"
#include "script/script.h"
#include "script/actor.h"
#include "script/referenceset.h"           // TFB_GetInactiveSet (parked actors, control mode 2), TFB_GetNPCSet (active NPCs, mode 1)
#include "modules/tfb/gameactor.h"
#include "modules/fx/gameparticle.h"
#include "modules/fx/partspray.h"
#include <stdlib.h>
#include "debugui_platform.h"

using RWS::TFB::GameActor;
extern GameActor *g_cameraFollowActor;             // GLOBAL: 0x61818c g_cameraFollowActor
int *CutScene_StackSlot(int delta);                // 0x436c00

// ---- instance names: Script::ScriptReference::ResolveSymbol 0x434350 reports the name each object is registered under (the
// stream entity / script identifier name, e.g. "Marty Actor"); an #ifdef ZOOSTERS_DEBUGUI hook in that function calls
// port_dbg_symbol_name
struct NameEntry { const void *obj; char *name; };
static NameEntry *g_names;
static int g_nnames, g_capNames;
void port_dbg_symbol_name(Script::ScriptReference *object, const char *name)
{
	if (!object || !name || !name[0]) return;
	for (int i = 0; i < g_nnames; i++)
		if (g_names[i].obj == object) {
			if (strcmp(g_names[i].name, name)) { free(g_names[i].name); g_names[i].name = _strdup(name); }   // the address was reused
			return;
		}
	if (g_nnames == g_capNames) {
		int cap = g_capNames ? g_capNames * 2 : 1024;
		NameEntry *n = (NameEntry *)realloc(g_names, sizeof(NameEntry) * cap);
		if (!n) return;
		g_names = n; g_capNames = cap;
	}
	g_names[g_nnames].obj = object;
	g_names[g_nnames].name = _strdup(name);
	g_nnames++;
}
static const char *registeredName(const void *object)
{
	for (int i = 0; i < g_nnames; i++) if (g_names[i].obj == object) return g_names[i].name;
	return 0;
}
const char *port_debugui_object_name(const void *object) { return registeredName(object); }   // the Studio outliner / inspector
// The live actors that have a registered name (the variable panel; unnamed clones cannot be addressed again after a level change). Names are
// registered under the ScriptReference view of the object.
int port_debugui_named_actors(GameActor **actors, const char **names, int max)
{
	int n = 0;
	for (int i = 0; i < g_nnames && n < max; i++) {
		GameActor *a = static_cast<GameActor *>(static_cast<Script::ScriptReference *>(const_cast<void *>(g_names[i].obj)));
		if (!port_debugui_actor_alive(a)) continue;
		bool dup = false;
		for (int k = 0; k < n && !dup; k++) dup = actors[k] == a;
		if (dup) continue;
		actors[n] = a; names[n] = g_names[i].name; n++;
	}
	return n;
}

extern "C" int port_frames_presented(void);

struct TrigSite {
	void *op;                     // the ValueCheck
	char self[48], target[48];    // registered names of the script object and the measured object (type name if none; "YOU" = the controlled actor)
	float center[3], other[3];    // positions at the last check
	float distance, rhs;          // in script units (world / 12)
	int compare;                  // 0 <=, 1 =, 2 >=, 3 <, 4 >, 5 <>
	bool result;
	int lastFrame, firstFrame, evals, gapLast;
};
enum { MAX_SITES = 512 };
static TrigSite g_sites[MAX_SITES];
static int g_nsites;
static void *g_curOp;             // the ValueCheck between begin and end
static bool g_measured;           // a position measurement happened inside it
static float g_mCenter[3], g_mOther[3];
static char g_mSelf[48], g_mTarget[48];
static bool g_drawSpheres = true, g_onlyRecent = true, g_drawLinks = true, g_drawWake = true, g_drawParticles = false;
static float g_rangeRadius = 400.0f;             // wake / particle ranges drawn only for objects this close to the player
static const char *const kCompare[6] = { "<=", "=", ">=", "<", ">", "<>" };

static void nameOf(Script::ScriptReference *r, char *out, size_t n)
{
	out[0] = 0;
	if (!r) return;
	const char *reg = registeredName(r);
	if (g_cameraFollowActor && r == static_cast<Script::ScriptReference *>(g_cameraFollowActor)) {
		_snprintf(out, n - 1, "YOU%s%s", reg ? ": " : "", reg ? reg : "");
		out[n - 1] = 0;
		return;
	}
	if (reg) { strncpy(out, reg, n - 1); out[n - 1] = 0; return; }
	// slot 0 is not a name getter on every measured object (street's tour faulted on one returning a float, 0x403a5515), so the result is
	// only copied when it points into committed readable memory (SEH does not help: the port's vectored fault handler ends the run first)
	const char *s = static_cast<Script::NamedObject *>(r)->GetName();
#ifdef _WIN32
	MEMORY_BASIC_INFORMATION mi;
	if (s && VirtualQuery(s, &mi, sizeof mi) == sizeof mi && mi.State == MEM_COMMIT && !(mi.Protect & (PAGE_GUARD | PAGE_NOACCESS))) {
		size_t room = (size_t)((const char *)mi.BaseAddress + mi.RegionSize - s);
#else   // zoosters::os::MemoryReadable (os_sdl.cpp: the kernel reads it): the whole buffer, else up to the end of the pointer's page
	size_t room = n, page = 4096 - ((uintptr_t)s & 4095);
	if (s && !zoosters::os::MemoryReadable(s, room))
		room = page < n && zoosters::os::MemoryReadable(s, page) ? page : 0;
	if (s && room) {
#endif
		size_t k = 0;
		while (k < n - 1 && k < room && s[k]) { out[k] = s[k]; k++; }
		out[k] = 0;
	} else {
		strncpy(out, "?", n - 1); out[n - 1] = 0;
	}
}

// Script::PositionMeasurement::GetFloat: `m` is the measured object, `self` the running script's object (positions read as the game
// read them just before, through slot 17 GetVector)
void port_dbg_position_measure(Script::PositionMeasurement *m, Script::ScriptReference *owner, Script::ScriptReference *self,
                               Script::ScriptReference *selfOwner, float distance)
{
	(void)distance;
	if (!g_curOp || !m || !self) return;
	const Vec3 *a = static_cast<Script::Placement *>(self)->GetVector(selfOwner);
	const Vec3 *b = reinterpret_cast<Script::VectorMeasurement *>(m)->GetVector(owner);
	if (!a || !b) return;
	g_mCenter[0] = a->x; g_mCenter[1] = a->y; g_mCenter[2] = a->z;
	g_mOther[0] = b->x; g_mOther[1] = b->y; g_mOther[2] = b->z;
	nameOf(self, g_mSelf, sizeof g_mSelf);
	nameOf(reinterpret_cast<Script::ScriptReference *>(m), g_mTarget, sizeof g_mTarget);
	g_measured = true;
}

void port_dbg_value_check_begin(void *op) { g_curOp = op; g_measured = false; }

void port_dbg_value_check_end(void *op, float lhs, int compare, float rhs, bool result)
{
	bool measured = g_measured && g_curOp == op;
	g_curOp = 0;
	if (!measured) return;                 // a comparison of something else (a counter, a flag)
	int f = port_frames_presented();
	TrigSite *s = 0;
	for (int i = 0; i < g_nsites; i++) if (g_sites[i].op == op) { s = &g_sites[i]; break; }
	if (!s) {
		if (g_nsites == MAX_SITES) return;
		s = &g_sites[g_nsites++];
		memset(s, 0, sizeof *s);
		s->op = op;
		s->firstFrame = f;
		s->lastFrame = f;
	}
	s->gapLast = f - s->lastFrame;
	s->lastFrame = f;
	s->evals++;
	memcpy(s->center, g_mCenter, sizeof s->center);
	memcpy(s->other, g_mOther, sizeof s->other);
	memcpy(s->self, g_mSelf, sizeof s->self);
	memcpy(s->target, g_mTarget, sizeof s->target);
	s->distance = lhs;
	s->compare = compare;
	s->rhs = rhs;
	s->result = result;
}

static void circle(const float *c, float r, int plane, unsigned rgba)
{
	const int N = 32;
	float prev[3], cur[3];
	for (int i = 0; i <= N; i++) {
		float a = (float)i * 6.2831853f / N, u = cosf(a) * r, v = sinf(a) * r;
		cur[0] = c[0]; cur[1] = c[1]; cur[2] = c[2];
		if (plane == 0) { cur[0] += u; cur[2] += v; }          // horizontal
		else if (plane == 1) { cur[0] += u; cur[1] += v; }
		else { cur[1] += u; cur[2] += v; }
		if (i) port_debugui_line(prev, cur, rgba);
		memcpy(prev, cur, sizeof prev);
	}
}

// ---- the other range mechanisms (read from the game's sets, no hooks): parked actors wake when a player comes within
// activationRange * 12 (Script::ScriptActor::GetActivationRange 0x429ca0, the inactive-set loop of the script actor update); active NPCs
// park again beyond (deactivation + activation) * 12 (GetDeactivationRange 0x429d10); particle systems activate within their activation
// distance (GameParticle::GetActivationDistance 0x40a640). All use PlayerObj::Dist (3D, closest approach within 1/30 s) to each player.
// Outside cut-scene state 2 the actor ranges are inactive (0 / never), as the getters say.
struct RangeRow { char name[48]; float pos[3]; float radius, dist; int kind; };   // kind 0 wake, 1 sleep, 2 particle
static RangeRow g_ranges[256];
static int g_nranges;

static void collectRanges(void)
{
	g_nranges = 0;
	GameActor *me = port_debugui_player();
	if (!me || *CutScene_StackSlot(0) != 2) return;
	const Vec3 *mp = me->GetVector(me);
	if (!mp) return;
	Script::ReferenceSet *sets[2] = { TFB_GetInactiveSet(), TFB_GetNPCSet() };
	for (int k = 0; k < 2; k++) {
		Script::ReferenceSet *set = sets[k];
		for (Script::SetNode *n = set ? set->m_head : 0; n && g_nranges < 256; n = n->m_next) {
			GameActor *a = static_cast<GameActor *>(n->m_object);
			if (!a) continue;
			const RWS::TFB::ActorParams &p = a->m_params;
			float r = k == 0 ? p.m_activationRange * 12.0f : (p.m_activationRange + p.m_deactivationRange) * 12.0f;
			if (r <= 0.0f || (k == 0 && a->m_controlMode != 2) || (k == 1 && a->m_controlMode != 1)) continue;
			const Vec3 *ap = a->GetVector(a);
			if (!ap) continue;
			float dx = ap->x - mp->x, dy = ap->y - mp->y, dz = ap->z - mp->z, d = sqrtf(dx * dx + dy * dy + dz * dz);
			if (d - r > g_rangeRadius) continue;
			RangeRow &row = g_ranges[g_nranges++];
			nameOf(a, row.name, sizeof row.name);
			row.pos[0] = ap->x; row.pos[1] = ap->y; row.pos[2] = ap->z;
			row.radius = r; row.dist = d; row.kind = k;
		}
	}
	if (!g_drawParticles) return;
	Script::ReferenceSet *ps = ::TFB_GetInactiveParticleSet();
	for (Script::SetNode *n = ps ? ps->m_head : 0; n && g_nranges < 256; n = n->m_next) {
		RWS::FX::GameParticle *gp = static_cast<RWS::FX::GameParticle *>(n->m_object);
		if (!gp) continue;
		float r = gp->GetActivationDistance();
		if (r <= 0.0f || r >= 3.0e38f) continue;
		const Vec3 *pp = gp->GetVector(gp);
		if (!pp) continue;
		float dx = pp->x - mp->x, dy = pp->y - mp->y, dz = pp->z - mp->z, d = sqrtf(dx * dx + dy * dy + dz * dz);
		if (d - r > g_rangeRadius) continue;
		RangeRow &row = g_ranges[g_nranges++];
		nameOf(gp, row.name, sizeof row.name);
		row.pos[0] = pp->x; row.pos[1] = pp->y; row.pos[2] = pp->z;
		row.radius = r; row.dist = d; row.kind = 2;
	}
}

extern "C" FILE *port_log;
// every frame while the Triggers panel is switched on (shown), pinned or not, overlay open or not; the panel reads what this left
void port_debugui_triggers_draw(int panelVisible)
{
	if (panelVisible || g_drawWake) collectRanges();
	if (!g_drawWake && !g_drawSpheres && !g_drawLinks) return;
	if (g_drawWake)
		for (int i = 0; i < g_nranges; i++) {
			const RangeRow &r = g_ranges[i];
			unsigned col = r.kind == 0 ? 0x40d0ff90 : r.kind == 1 ? 0xc060ff90 : 0xffa03090;
			circle(r.pos, r.radius, 0, col);                    // the horizontal ring only: dozens of these fill the view otherwise
		}
	int f = port_frames_presented();
	for (int i = 0; i < g_nsites; i++) {
		const TrigSite &s = g_sites[i];
		bool recent = f - s.lastFrame <= 120;
		if (!g_drawSpheres || (g_onlyRecent && !recent)) continue;
		unsigned col = !recent ? 0xa0a0a080 : s.result ? 0x40ff40e0 : 0xff4040e0;
		float r = s.rhs * 12.0f;
		if (r > 0.0f && r < 20000.0f) {
			circle(s.center, r, 0, col);
			circle(s.center, r, 1, col & 0xffffff80);
			circle(s.center, r, 2, col & 0xffffff80);
		}
		if (g_drawLinks && recent) port_debugui_line(s.center, s.other, 0xffff40c0);
	}
}

void port_debugui_triggers_panel(int open)
{
	static int logAt = getenv("PORT_DEBUGUI_RANGELOG") ? atoi(getenv("PORT_DEBUGUI_RANGELOG")) : -1;   // test aid: the range rows once
	if (logAt >= 0 && port_frames_presented() >= logAt && port_log) {
		logAt = -1;
		fprintf(port_log, "debugui: ranges at frame %d (%d rows):\n", port_frames_presented(), g_nranges);
		for (int i = 0; i < g_nranges; i++)
			fprintf(port_log, "  kind %d %-32s radius %.0f dist %.0f at %.0f %.0f %.0f\n", g_ranges[i].kind, g_ranges[i].name, g_ranges[i].radius,
			        g_ranges[i].dist, g_ranges[i].pos[0], g_ranges[i].pos[1], g_ranges[i].pos[2]);
	}
	int f = port_frames_presented();
	int active = 0;
	for (int i = 0; i < g_nsites; i++) if (f - g_sites[i].lastFrame <= 120) active++;
	ImGui::Text("range checks: %d sites, %d active (last 2 s)", g_nsites, active);
	if (!open) return;
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip()) {
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 32.0f);
		ImGui::TextUnformatted("Every script 'check value <actor>, <op>, N' that compares a distance. The distance is from the script's own "
		                       "object to the actor, in 3D, in units of 12 world units, at the closest approach within the next 1/30 s (both "
		                       "velocities count). A site is only checked while its script is in that state: 'every' is the gap between its "
		                       "last two checks in frames.");
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
	ImGui::Checkbox("script checks", &g_drawSpheres); ImGui::SameLine();
	ImGui::Checkbox("wake / sleep", &g_drawWake); ImGui::SameLine();
	ImGui::Checkbox("particles", &g_drawParticles); ImGui::SameLine();
	ImGui::Checkbox("only active", &g_onlyRecent); ImGui::SameLine();
	ImGui::Checkbox("links", &g_drawLinks); ImGui::SameLine();
	if (ImGui::Button("forget")) g_nsites = 0;
	if (!ImGui::BeginTable("trig", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
	                                      ImGuiTableFlags_Sortable, ImVec2(0, 240)))
		return;
	ImGui::TableSetupScrollFreeze(0, 1);
	ImGui::TableSetupColumn("script object"); ImGui::TableSetupColumn("centre"); ImGui::TableSetupColumn("measures"); ImGui::TableSetupColumn("distance");
	ImGui::TableSetupColumn("test"); ImGui::TableSetupColumn("now"); ImGui::TableSetupColumn("last"); ImGui::TableSetupColumn("every");
	ImGui::TableHeadersRow();
	// active sites first, nearest to passing first
	int order[MAX_SITES];
	for (int i = 0; i < g_nsites; i++) order[i] = i;
	for (int i = 1; i < g_nsites; i++)
		for (int j = i; j > 0; j--) {
			const TrigSite &a = g_sites[order[j]], &b = g_sites[order[j - 1]];
			bool ra = f - a.lastFrame <= 120, rb = f - b.lastFrame <= 120;
			float ka = fabsf(a.distance - a.rhs), kb = fabsf(b.distance - b.rhs);
			if ((ra && !rb) || (ra == rb && ka < kb)) { int t = order[j]; order[j] = order[j - 1]; order[j - 1] = t; } else break;
		}
	for (int k = 0; k < g_nsites; k++) {
		const TrigSite &s = g_sites[order[k]];
		bool recent = f - s.lastFrame <= 120;
		ImGui::TableNextRow();
		if (recent) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, s.result ? IM_COL32(30, 110, 30, 90) : IM_COL32(120, 30, 30, 90));
		ImGui::TableNextColumn(); ImGui::TextUnformatted(s.self[0] ? s.self : "(unnamed)");
		ImGui::TableNextColumn(); ImGui::Text("%.0f %.0f %.0f", s.center[0], s.center[1], s.center[2]);
		ImGui::TableNextColumn(); ImGui::TextUnformatted(s.target[0] ? s.target : "(unnamed)");
		ImGui::TableNextColumn(); ImGui::Text("%.1f", s.distance);
		ImGui::TableNextColumn(); ImGui::Text("%s %.1f (%.0f u)", s.compare >= 0 && s.compare < 6 ? kCompare[s.compare] : "?", s.rhs, s.rhs * 12.0f);
		ImGui::TableNextColumn(); ImGui::TextUnformatted(s.result ? "pass" : "fail");
		ImGui::TableNextColumn(); ImGui::Text("%d", f - s.lastFrame);
		ImGui::TableNextColumn(); ImGui::Text("%d", s.gapLast);
	}
	ImGui::EndTable();
	if (ImGui::TreeNodeEx("ranges", ImGuiTreeNodeFlags_DefaultOpen, "wake / sleep / particle ranges near you (%d)", g_nranges)) {
		ImGui::SliderFloat("within", &g_rangeRadius, 200.0f, 20000.0f, "%.0f u of their edge");
		ImGui::TextDisabled("cyan: parked actor wakes when you come within; purple: active NPC parks beyond; orange: particle system starts");
		for (int i = 0; i < g_nranges; i++) {
			const RangeRow &r = g_ranges[i];
			static const char *const kKind[3] = { "wakes within", "parks beyond", "starts within" };
			bool inside = r.kind == 1 ? r.dist <= r.radius : r.dist < r.radius;
			ImGui::Text("%-28s %s %.0f u  (you %.0f u: %s)", r.name, kKind[r.kind], r.radius, r.dist, inside ? "inside" : "outside");
		}
		ImGui::TreePop();
	}
}
