// Studio mode (ZOOSTERS_DEBUGUI). F9 opens the overlay, turns the free camera on (the game logic is held, see debugui_practice.cpp) and
// lets the level's actors be picked, moved, turned and edited at run time. Nothing is written back to the level files.
//   pick        left click in the scene: a ray from the view through the cursor against every actor's collision box
//               (Collider::m_worldBox; logic-only actors without a box get a small one so they can be picked too)
//   gizmo       ImGuizmo: move along X / Y / Z, turn about the vertical axis (world space)
//   outliner    every actor of the player / NPC / parked sets and the broad phase, by the name it was registered under; Focus flies the
//               free camera to it
//   inspector   position, facing / tilt / roll, the ActorParams the movement reads, ranges, collision box, inverse mass, tint
//   undo        Ctrl+Z (64 steps); "reset" restores the object as it was when first edited
// Edits go through the game's own setters where they exist (GameActor::SetVector -> PlaceAt, SetFacing; tilt and roll are the fields
// FinishMovement applies), then the rendered frame (the body's RwFrame, or m_matrix without a body) and the collision box
// (Collider::UpdateBounds) are updated at once, so the edit shows while the game is held.
// A moved body starts airborne (Corpus::LeaveGround, the game's teleport): unheld, a moved prop falls to the ground.
#ifndef ZOOSTERS_DEBUGUI
#error debugui_studio.cpp is built only with ZOOSTERS_DEBUGUI (build option --debugui)
#endif
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imgui.h"
#include "ImGuizmo.h"
#include "debugui.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "tfb/collision/collider.h"
#include "tfb/collision/broadphase.h"
#include "script/referenceset.h"           // TFB_GetPlayerSet / TFB_GetNPCSet / TFB_GetInactiveSet

using RWS::TFB::GameActor;
using RWS::TFB::Collider;
extern GameActor *g_cameraFollowActor;             // GLOBAL: 0x61818c g_cameraFollowActor
extern int g_currentLevel;                         // GLOBAL: 0x606a38 g_currentLevel
extern int port_freecam_on;
extern "C" int port_frames_presented(void);

static bool g_studio;
static GameActor *g_sel;
static bool g_showAll = true, g_showLabels = true;
static int g_level = -99;

// ---------------------------------------------------------------- the actors of the level
enum { MAX_ACTORS = 2048 };
static GameActor *g_actors[MAX_ACTORS];
static int g_nactors;

static void addActor(GameActor *a)
{
	if (!a || g_nactors >= MAX_ACTORS || !port_debugui_actor_alive(a)) return;   // (live actors only: debugui.h)
	for (int i = 0; i < g_nactors; i++) if (g_actors[i] == a) return;
	g_actors[g_nactors++] = a;
}

static void collectActors(void)
{
	g_nactors = 0;
	Script::ReferenceSet *sets[3] = { TFB_GetPlayerSet(), TFB_GetNPCSet(), TFB_GetInactiveSet() };
	for (int k = 0; k < 3; k++) {
		Script::ReferenceSet *set = sets[k];
		for (Script::SetNode *n = set ? set->m_head : 0; n; n = n->m_next) addActor(static_cast<GameActor *>(n->m_object));
	}
	for (int i = 0; i < g_boundsCount && i < RWS::TFB::BROADPHASE_MAX_BOUNDS; i++) {   // colliders in the broad phase (each once)
		RWS::TFB::ColliderBound *b = g_boundsItems[0][i];
		Collider *c = b ? b->m_collider : 0;
		if (c && b == &c->m_min) addActor(&RWS::TFB::ActorOf(*c));
	}
}

static bool isKnown(GameActor *a) { for (int i = 0; i < g_nactors; i++) if (g_actors[i] == a) return true; return false; }

static const char *nameOf(GameActor *a)
{
	const char *n = port_debugui_object_name(static_cast<Script::ScriptReference *>(a));
	return n ? n : "(unnamed)";
}

// ---------------------------------------------------------------- placement: what the editor reads and writes
struct Placement3 { Vec3 pos; float yaw, tilt, roll; };

static Placement3 readPlacement(GameActor *a)
{
	Placement3 p;
	const RwV3d &v = a->m_corpus ? a->m_corpus->m_lastFrame.pos : a->m_matrix.pos;
	p.pos.x = v.x; p.pos.y = v.y; p.pos.z = v.z;
	p.yaw = a->GetFacing();
	p.tilt = a->m_tilt;
	p.roll = a->m_roll;
	return p;
}

// the frame the game builds from facing / tilt / roll (Corpus::ApplyMovement + FinishMovement, without the slope of ground tilters)
static void buildMatrix(const Placement3 &p, RwMatrix *m)
{
	float y = p.yaw * 0.017453292f;
	memset(m, 0, sizeof *m);
	m->at.x = sinf(y); m->at.y = 0.0f; m->at.z = cosf(y);
	m->up.x = 0.0f; m->up.y = 1.0f; m->up.z = 0.0f;
	m->right.x = m->at.z; m->right.y = 0.0f; m->right.z = -m->at.x;   // up x at
	RwMatrixUpdate(m);
	if (p.tilt != 0.0f) { RwV3d ax = m->right; RwMatrixRotate(m, &ax, p.tilt, rwCOMBINEPOSTCONCAT); }
	if (p.roll != 0.0f) { RwV3d ax = m->at; RwMatrixRotate(m, &ax, p.roll, rwCOMBINEPOSTCONCAT); }
	m->pos.x = p.pos.x; m->pos.y = p.pos.y; m->pos.z = p.pos.z;
	RwMatrixUpdate(m);
}

static void applyPlacement(GameActor *a, const Placement3 &p)
{
	Placement3 cur = readPlacement(a);
	if (cur.pos.x != p.pos.x || cur.pos.y != p.pos.y || cur.pos.z != p.pos.z) {
		if (a->m_corpus || a->m_controlMode < 2) a->SetVector(&p.pos, a);           // the scripts' "set position" (PlaceAt)
		else { a->m_matrix.pos.x = p.pos.x; a->m_matrix.pos.y = p.pos.y; a->m_matrix.pos.z = p.pos.z; }   // a parked actor without body
	}
	a->SetFacing(p.yaw, 1);
	a->m_tilt = p.tilt;
	a->m_roll = p.roll;
	RwMatrix m;
	buildMatrix(p, &m);
	if (Corpus *c = a->m_corpus) {                    // shown now, while the game is held
		c->m_frame->modelling = m;
		RwFrameUpdateObjects(c->m_frame);
		c->m_lastFrame = m;
	} else {
		a->m_matrix = m;
	}
	a->UpdateBounds(&m, 0.0f);
}

// ---------------------------------------------------------------- undo
struct Snapshot { GameActor *actor; Placement3 place; RWS::TFB::ActorParams params; float inverseMass; uint32_t tint; };
enum { UNDO_MAX = 64, FIRST_MAX = 256 };
static Snapshot g_undo[UNDO_MAX];
static int g_nundo;
static Snapshot g_first[FIRST_MAX];   // each object as it was before its first edit (reset)
static int g_nfirst;

static Snapshot snap(GameActor *a)
{
	Snapshot s;
	s.actor = a;
	s.place = readPlacement(a);
	s.params = a->m_params;
	s.inverseMass = static_cast<Collider *>(a)->m_inverseMass;
	s.tint = a->m_tintColor;
	return s;
}

static void restoreSnap(const Snapshot &s)
{
	if (!isKnown(s.actor)) return;
	GameActor *a = s.actor;
	bool box = memcmp(&a->m_params.m_zExtent, &s.params.m_zExtent, sizeof(float) * 6) != 0;
	a->m_params = s.params;
	static_cast<Collider *>(a)->m_inverseMass = s.inverseMass;
	a->m_tintColor = s.tint;
	if (box) a->UpdateCollisionExtents(false);
	applyPlacement(a, s.place);
}

static void pushUndo(GameActor *a)          // before an edit starts
{
	Snapshot s = snap(a);
	if (g_nundo == UNDO_MAX) { memmove(g_undo, g_undo + 1, sizeof(Snapshot) * (UNDO_MAX - 1)); g_nundo--; }
	g_undo[g_nundo++] = s;
	for (int i = 0; i < g_nfirst; i++) if (g_first[i].actor == a) return;
	if (g_nfirst < FIRST_MAX) g_first[g_nfirst++] = s;
}

static void undo(void)
{
	while (g_nundo > 0) {
		Snapshot s = g_undo[--g_nundo];
		if (isKnown(s.actor)) { restoreSnap(s); g_sel = s.actor; return; }
	}
}

// ---------------------------------------------------------------- view, picking, drawing
struct View { float pos[3], fwd[3], right[3], up[3], vw[2], nf[2], w, h; bool ok; };
static View g_view;

static void readView(void)
{
	ImGuiIO &io = ImGui::GetIO();
	g_view.w = io.DisplaySize.x; g_view.h = io.DisplaySize.y;
	g_view.ok = port_debugui_view(g_view.pos, g_view.fwd, g_view.right, g_view.up, g_view.vw, g_view.nf) && g_view.w > 0;
}

static float dot3(const float *a, const float *b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

// world point -> screen pixels (false when behind the camera)
static bool project(const float *p, ImVec2 *out)
{
	float d[3] = { p[0] - g_view.pos[0], p[1] - g_view.pos[1], p[2] - g_view.pos[2] };
	float z = dot3(d, g_view.fwd);
	if (z < 1.0f) return false;
	float x = dot3(d, g_view.right) / (z * g_view.vw[0]), y = dot3(d, g_view.up) / (z * g_view.vw[1]);
	out->x = (x * 0.5f + 0.5f) * g_view.w;
	out->y = (0.5f - y * 0.5f) * g_view.h;
	return true;
}

static void boxOf(GameActor *a, CollisionBox *b)
{
	*b = static_cast<Collider *>(a)->m_worldBox;
	for (int k = 0; k < 3; k++) if (b->m_extent[k] < 4.0f) b->m_extent[k] = 4.0f;   // logic-only actors: a small box to click
}

// ray (origin o, direction d) against an oriented box: entry distance, or -1
static float rayBox(const float *o, const float *d, const CollisionBox &b)
{
	const RwV3d *ax[3] = { &b.m_frame.right, &b.m_frame.up, &b.m_frame.at };
	float rel[3] = { o[0] - b.m_frame.pos.x, o[1] - b.m_frame.pos.y, o[2] - b.m_frame.pos.z };
	float tmin = -1e30f, tmax = 1e30f;
	for (int k = 0; k < 3; k++) {
		float a[3] = { ax[k]->x, ax[k]->y, ax[k]->z };
		float e = dot3(a, rel), f = dot3(a, d);
		if (fabsf(f) < 1e-6f) { if (fabsf(e) > b.m_extent[k]) return -1.0f; continue; }
		float t1 = (-b.m_extent[k] - e) / f, t2 = (b.m_extent[k] - e) / f;
		if (t1 > t2) { float t = t1; t1 = t2; t2 = t; }
		if (t1 > tmin) tmin = t1;
		if (t2 < tmax) tmax = t2;
		if (tmin > tmax) return -1.0f;
	}
	return tmax < 0.0f ? -1.0f : (tmin > 0.0f ? tmin : 0.0f);
}

static GameActor *pickAt(ImVec2 mouse)
{
	float x = (mouse.x / g_view.w) * 2.0f - 1.0f, y = 1.0f - (mouse.y / g_view.h) * 2.0f;
	float d[3];
	for (int k = 0; k < 3; k++) d[k] = g_view.fwd[k] + g_view.right[k] * x * g_view.vw[0] + g_view.up[k] * y * g_view.vw[1];
	float len = sqrtf(dot3(d, d));
	for (int k = 0; k < 3; k++) d[k] /= len;
	GameActor *best = 0;
	float bestT = 1e30f;
	for (int i = 0; i < g_nactors; i++) {
		CollisionBox b;
		boxOf(g_actors[i], &b);
		float t = rayBox(g_view.pos, d, b);
		if (t >= 0.0f && t < bestT) { bestT = t; best = g_actors[i]; }
	}
	return best;
}

static void drawBox(const CollisionBox &box, unsigned rgba)
{
	const RwMatrix &m = box.m_frame;
	float c[8][3];
	for (int i = 0; i < 8; i++) {
		float sx = (i & 1) ? 1.0f : -1.0f, sy = (i & 2) ? 1.0f : -1.0f, sz = (i & 4) ? 1.0f : -1.0f;
		c[i][0] = m.pos.x + sx * box.m_extent[0] * m.right.x + sy * box.m_extent[1] * m.up.x + sz * box.m_extent[2] * m.at.x;
		c[i][1] = m.pos.y + sx * box.m_extent[0] * m.right.y + sy * box.m_extent[1] * m.up.y + sz * box.m_extent[2] * m.at.y;
		c[i][2] = m.pos.z + sx * box.m_extent[0] * m.right.z + sy * box.m_extent[1] * m.up.z + sz * box.m_extent[2] * m.at.z;
	}
	static const int e[12][2] = { {0,1},{2,3},{4,5},{6,7}, {0,2},{1,3},{4,6},{5,7}, {0,4},{1,5},{2,6},{3,7} };
	for (int k = 0; k < 12; k++) port_debugui_line(c[e[k][0]], c[e[k][1]], rgba);
}

// ---------------------------------------------------------------- gizmo (ImGuizmo: OpenGL-style column-major matrices)
static void gizmo(void)
{
	if (!g_sel || !g_view.ok) return;
	ImGuizmo::BeginFrame();
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
	ImGuizmo::SetRect(0, 0, g_view.w, g_view.h);
	const float *R = g_view.right, *U = g_view.up, *F = g_view.fwd, *P = g_view.pos;
	float B[3] = { -F[0], -F[1], -F[2] };
	float view[16] = { R[0], U[0], B[0], 0,  R[1], U[1], B[1], 0,  R[2], U[2], B[2], 0,
	                   -dot3(R, P), -dot3(U, P), -dot3(B, P), 1 };
	float n = g_view.nf[0], f = g_view.nf[1];
	float proj[16] = { 1.0f / g_view.vw[0], 0, 0, 0,  0, 1.0f / g_view.vw[1], 0, 0,  0, 0, -(f + n) / (f - n), -1,  0, 0, -2.0f * f * n / (f - n), 0 };
	Placement3 p = readPlacement(g_sel);
	RwMatrix m;
	Placement3 flat = p;
	flat.tilt = flat.roll = 0.0f;
	buildMatrix(flat, &m);
	float model[16] = { m.right.x, m.right.y, m.right.z, 0,  m.up.x, m.up.y, m.up.z, 0,  m.at.x, m.at.y, m.at.z, 0,  m.pos.x, m.pos.y, m.pos.z, 1 };
	static bool wasUsing;
	if (ImGuizmo::Manipulate(view, proj, (ImGuizmo::OPERATION)(ImGuizmo::TRANSLATE | ImGuizmo::ROTATE_Y), ImGuizmo::WORLD, model)) {
		if (!wasUsing) pushUndo(g_sel);
		p.pos.x = model[12]; p.pos.y = model[13]; p.pos.z = model[14];
		float yaw = atan2f(model[8], model[10]) * 57.29578f;
		p.yaw = yaw < 0.0f ? yaw + 360.0f : yaw;
		applyPlacement(g_sel, p);
	}
	wasUsing = ImGuizmo::IsUsing();
}

// ---------------------------------------------------------------- windows
static void outliner(void)
{
	ImGui::SetNextWindowPos(ImVec2(8, 70), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(250, 330), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Outliner")) { ImGui::End(); return; }
	static char filter[64];
	ImGui::SetNextItemWidth(-1);
	ImGui::InputTextWithHint("##find", "find", filter, sizeof filter);
	ImGui::Text("%d actors", g_nactors);
	ImGui::BeginChild("list");
	for (int i = 0; i < g_nactors; i++) {
		GameActor *a = g_actors[i];
		const char *name = nameOf(a);
		if (filter[0]) {   // case-insensitive substring
			char lo[64], lf[64];
			int k;
			for (k = 0; name[k] && k < 63; k++) lo[k] = (char)tolower((unsigned char)name[k]);
			lo[k] = 0;
			for (k = 0; filter[k] && k < 63; k++) lf[k] = (char)tolower((unsigned char)filter[k]);
			lf[k] = 0;
			if (!strstr(lo, lf)) continue;
		}
		ImGui::PushID(a);
		char label[96];
		_snprintf(label, sizeof label - 1, "%s%s", name, a == g_cameraFollowActor ? "  (you)" : "");
		label[sizeof label - 1] = 0;
		if (ImGui::Selectable(label, a == g_sel, ImGuiSelectableFlags_AllowDoubleClick)) {
			g_sel = a;
			if (ImGui::IsMouseDoubleClicked(0)) { Placement3 p = readPlacement(a); port_debugui_freecam_focus(&p.pos.x, 250.0f); }
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
	ImGui::End();
}

static bool editFloat(const char *label, float *v, float speed, GameActor *a)
{
	bool changed = ImGui::DragFloat(label, v, speed);
	if (ImGui::IsItemActivated()) pushUndo(a);
	return changed;
}
static bool editInt(const char *label, int32_t *v, GameActor *a)
{
	int x = *v;
	bool changed = ImGui::DragInt(label, &x, 0.2f);
	if (ImGui::IsItemActivated()) pushUndo(a);
	if (changed) *v = x;
	return changed;
}

static void inspector(void)
{
	ImGui::SetNextWindowPos(ImVec2(g_view.w - 330, 70), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(320, 470), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Inspector")) { ImGui::End(); return; }
	GameActor *a = g_sel;
	if (!a) { ImGui::TextDisabled("click an object in the scene, or pick one in the Outliner"); ImGui::End(); return; }
	ImGui::Text("%s", nameOf(a));
	ImGui::TextDisabled("%p  %s  control mode %d", (void *)a, a->m_corpus ? "body" : "no body", a->m_controlMode);
	if (ImGui::Button("focus")) { Placement3 p = readPlacement(a); port_debugui_freecam_focus(&p.pos.x, 250.0f); }
	ImGui::SameLine();
	if (ImGui::Button("reset")) { for (int i = 0; i < g_nfirst; i++) if (g_first[i].actor == a) { pushUndo(a); restoreSnap(g_first[i]); } }
	ImGui::SameLine();
	ImGui::BeginDisabled(g_nundo == 0);
	if (ImGui::Button("undo (Ctrl+Z)")) undo();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("deselect")) { g_sel = 0; ImGui::End(); return; }

	Placement3 p = readPlacement(a);
	bool moved = false;
	ImGui::SeparatorText("placement");
	moved |= ImGui::DragFloat3("position", &p.pos.x, 1.0f);
	if (ImGui::IsItemActivated()) pushUndo(a);
	moved |= editFloat("facing", &p.yaw, 0.5f, a);
	moved |= editFloat("tilt", &p.tilt, 0.5f, a);
	moved |= editFloat("roll", &p.roll, 0.5f, a);
	if (moved) applyPlacement(a, p);

	RWS::TFB::ActorParams &q = a->m_params;
	if (ImGui::CollapsingHeader("movement", ImGuiTreeNodeFlags_DefaultOpen)) {
		editFloat("ground top speed", &q.m_groundSpeed, 1.0f, a);
		editFloat("ground accel", &q.m_groundAccel, 1.0f, a);
		editFloat("ground decel", &q.m_groundDecel, 1.0f, a);
		editFloat("ground turn rate", &q.m_groundTurnRate, 1.0f, a);
		editFloat("air top speed", &q.m_airSpeed, 1.0f, a);
		editFloat("air accel", &q.m_airAccel, 1.0f, a);
		editFloat("air decel", &q.m_airDecel, 1.0f, a);
		editFloat("air turn rate", &q.m_airTurnRate, 1.0f, a);
	}
	if (ImGui::CollapsingHeader("jump and fall", ImGuiTreeNodeFlags_DefaultOpen)) {
		editFloat("initial jump velocity", &q.m_initialJumpVelocity, 1.0f, a);
		editFloat("up top speed", &q.m_upSpeed, 1.0f, a);
		editFloat("up accel", &q.m_upAccel, 1.0f, a);
		editFloat("gravity (down accel)", &q.m_downAccel, 1.0f, a);
		editFloat("down top speed", &q.m_downSpeed, 1.0f, a);
		editInt("jump coast frames", &q.m_coastFrames, a);
		editInt("jump delay", &q.m_jumpDelay, a);
	}
	if (ImGui::CollapsingHeader("physics")) {
		editFloat("bounce restitution", &q.m_bounceRestitution, 0.01f, a);
		editFloat("weight", &q.m_weight, 0.1f, a);
		float &im = static_cast<Collider *>(a)->m_inverseMass;
		editFloat("inverse mass", &im, 0.01f, a);
		bool box = false;
		box |= editFloat("extent W (x)", &q.m_xExtent, 0.05f, a);
		box |= editFloat("extent L (up)", &q.m_zExtent, 0.05f, a);
		box |= editFloat("extent H (z)", &q.m_yExtent, 0.05f, a);
		box |= editFloat("offset x", &q.m_xOffset, 0.05f, a);
		box |= editFloat("offset up", &q.m_zOffset, 0.05f, a);
		box |= editFloat("offset z", &q.m_yOffset, 0.05f, a);
		if (box) { a->UpdateCollisionExtents(false); RwMatrix m; buildMatrix(readPlacement(a), &m); a->UpdateBounds(&m, 0.0f); }
		ImGui::TextDisabled("extents / offsets in the editor's units (x 12 = world)");
	}
	if (ImGui::CollapsingHeader("ranges and look")) {
		editFloat("activation range", &q.m_activationRange, 0.1f, a);
		editFloat("deactivation range", &q.m_deactivationRange, 0.1f, a);
		editInt("health", &q.m_hitPoints, a);
		float col[4] = { a->m_tint[0] / 255.0f, a->m_tint[1] / 255.0f, a->m_tint[2] / 255.0f, a->m_tint[3] / 255.0f };
		if (ImGui::ColorEdit4("tint", col)) for (int k = 0; k < 4; k++) a->m_tint[k] = (uint8_t)(col[k] * 255.0f + 0.5f);
		if (ImGui::IsItemActivated()) pushUndo(a);
	}
	ImGui::End();
}

// ---------------------------------------------------------------- every frame
int port_debugui_studio_on(void) { return g_studio; }

void port_debugui_studio_tick(int open)
{
	static bool openedOverlay;   // F9 opened the overlay: leaving Studio closes it again
	if (ImGui::IsKeyPressed(ImGuiKey_F9, false)) {
		g_studio = !g_studio;
		port_freecam_on = g_studio;
		if (g_studio) { openedOverlay = !open; port_debugui_set_open(1); }
		else if (openedOverlay) { openedOverlay = false; port_debugui_set_open(0); }
	}
	if (g_currentLevel != g_level) { g_level = g_currentLevel; g_sel = 0; g_nundo = 0; g_nfirst = 0; }   // a new level: old pointers are gone
	// test aid: PORT_DEBUGUI_STUDIO=<frame>,<name part>[,dx,dy,dz,dyaw]: Studio on at that frame with the first actor whose name contains
	// <name part> selected and in focus; 30 frames later that move / turn is applied the way the gizmo applies one
	static const char *aid = getenv("PORT_DEBUGUI_STUDIO");
	static int aidAt = aid ? atoi(aid) : -1, aidMoveAt = -1, aidUndoAt = -1;
	static float aidMove[4];
	int f = port_frames_presented();
	if (aidAt >= 0 && f >= aidAt) {
		aidAt = -1;
		g_studio = true; port_freecam_on = 1; port_debugui_set_open(1);
		collectActors();
		const char *c1 = strchr(aid, ',');
		char part[64] = "";
		if (c1) { sscanf(c1 + 1, "%63[^,]", part); const char *c2 = strchr(c1 + 1, ','); if (c2) sscanf(c2 + 1, "%f,%f,%f,%f", &aidMove[0], &aidMove[1], &aidMove[2], &aidMove[3]); }
		for (int i = 0; i < g_nactors && part[0]; i++) if (strstr(nameOf(g_actors[i]), part)) { g_sel = g_actors[i]; break; }
		if (g_sel) { Placement3 p = readPlacement(g_sel); port_debugui_freecam_focus(&p.pos.x, 250.0f); aidMoveAt = f + 30; }
	}
	if (aidMoveAt >= 0 && f >= aidMoveAt) {
		aidMoveAt = -1;
		if (g_sel && isKnown(g_sel)) {
			pushUndo(g_sel);
			Placement3 p = readPlacement(g_sel);
			p.pos.x += aidMove[0]; p.pos.y += aidMove[1]; p.pos.z += aidMove[2]; p.yaw += aidMove[3];
			applyPlacement(g_sel, p);
			if (getenv("PORT_DEBUGUI_STUDIO_UNDO")) aidUndoAt = f + 30;   // test aid: and undo it 30 frames later
		}
	}
	if (aidUndoAt >= 0 && f >= aidUndoAt) { aidUndoAt = -1; undo(); }
	if (!g_studio) return;
	collectActors();
	if (g_sel && !isKnown(g_sel)) g_sel = 0;
	readView();
	if (!g_view.ok) return;
	ImGuiIO &io = ImGui::GetIO();
	if (open && ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsKeyPressed(ImGuiKey_Z, false)) undo();
	if (open && ImGui::IsMouseClicked(0) && !io.WantCaptureMouse && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing()) g_sel = pickAt(io.MousePos);
	GameActor *hover = open && !io.WantCaptureMouse ? pickAt(io.MousePos) : 0;
	for (int i = 0; i < g_nactors; i++) {          // every actor's box (faint), the hovered one brighter, the selected one yellow
		GameActor *a = g_actors[i];
		if (a != g_sel && a != hover && !g_showAll) continue;
		CollisionBox b;
		boxOf(a, &b);
		drawBox(b, a == g_sel ? 0xffe040ff : a == hover ? 0xffffffc0 : 0x8090a060);
	}
	ImDrawList *dl = ImGui::GetForegroundDrawList();
	if (g_showLabels)
		for (int i = 0; i < g_nactors; i++) {
			GameActor *a = g_actors[i];
			if (a != g_sel && a != hover) continue;
			Placement3 p = readPlacement(a);
			ImVec2 s;
			if (project(&p.pos.x, &s)) {
				const char *n = nameOf(a);
				ImVec2 sz = ImGui::CalcTextSize(n);
				dl->AddRectFilled(ImVec2(s.x - 3, s.y - sz.y - 10), ImVec2(s.x + sz.x + 3, s.y - 7), IM_COL32(0, 0, 0, 160), 3.0f);
				dl->AddText(ImVec2(s.x, s.y - sz.y - 9), a == g_sel ? IM_COL32(255, 224, 64, 255) : IM_COL32(255, 255, 255, 255), n);
			}
		}
	if (!open) return;
	gizmo();
	outliner();
	inspector();
	ImGui::SetNextWindowPos(ImVec2(g_view.w * 0.5f - 160, g_view.h - 40), ImGuiCond_FirstUseEver);
	ImGui::Begin("Studio", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
	ImGui::TextColored(ImVec4(1, 0.85f, 0.3f, 1), "STUDIO (F9)");
	ImGui::SameLine();
	ImGui::Checkbox("all boxes", &g_showAll);
	ImGui::SameLine();
	ImGui::Checkbox("labels", &g_showLabels);
	ImGui::SameLine();
	ImGui::TextDisabled("click: select  drag gizmo: move / turn  right-drag + WASD: fly");
	ImGui::End();
}
