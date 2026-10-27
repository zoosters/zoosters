// The panels (ZOOSTERS_DEBUGUI). Read-only views of game state through the src/game headers; the only writes are the overlay's own settings and the Render panel's
// pass toggles (port_dbg_hide: drawing only, the game logic still runs every pass).
//   Player  the camera-follow actor's body (Corpus): position, velocity, speeds, heading, ground / air state and timers
//   Input   keys down (the game's own key names), mouse state, the active input context and the action values it produces
//   Render  draw counts of the frame (what the game submitted), frame time, pass toggles
//   Variables, Sandbox  debugui_vars.cpp (these two write game state)
#ifndef ZOOSTERS_DEBUGUI
#error debugui_panels.cpp is built only with ZOOSTERS_DEBUGUI (build option --debugui)
#endif
#include "debugui_platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imgui.h"
#include "imgui_internal.h"               // ImGuiSettingsHandler
#include "debugui.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "framework/startup/win32/input/inputmanager.h"
#include "framework/startup/win32/input/controls.h"
#include <unordered_set>

using RWS::TFB::GameActor;
extern GameActor *g_cameraFollowActor;   // GLOBAL: 0x61818c g_cameraFollowActor
extern int g_currentLevel;               // GLOBAL: 0x606a38 g_currentLevel (-1 before the first level: title / menus)
extern float g_frameDt;                  // GLOBAL: 0x618318 g_frameDt (seconds of this tick)
extern float g_frameRate;                // GLOBAL: g_frameRate (ticks per second)
extern "C" int port_frames_presented(void);

static std::unordered_set<const void *> &liveActors(void) { static std::unordered_set<const void *> s; return s; }   // (constructed before the first actor)
void port_debugui_actor_life(const void *actor, int alive)
{
	if (alive) liveActors().insert(actor);
	else liveActors().erase(actor);
}
int port_debugui_actor_alive(const void *actor) { return actor && liveActors().count(actor) != 0; }
GameActor *port_debugui_player(void) { return port_debugui_actor_alive(g_cameraFollowActor) ? g_cameraFollowActor : 0; }
extern "C" int port_draw_im2d, port_draw_im3d, port_draw_d3d, port_draw_world, port_draw_atomic;

// level index = g_currentLevel
static const char *const kLevels[] = { "kingofny", "escape", "street", "mutiny", "jungle", "rave", "lemurs", "banquet", "coming", "beach",
                                       "rescue", "battle", "shuffle", "golf", "map" };
static const char *levelName(int l) { return l >= 0 && l < (int)(sizeof kLevels / sizeof kLevels[0]) ? kLevels[l] : (l < 0 ? "(menus)" : "?"); }

enum { PANEL_TIMER, PANEL_PLAYER, PANEL_INPUT, PANEL_RENDER, PANEL_PHYSICS, PANEL_COLLISION, PANEL_TRIGGERS, PANEL_PRACTICE, PANEL_VARS, PANEL_SANDBOX, PANELS };
static const char *const kPanelNames[PANELS] = { "Timer", "Player", "Input", "Render", "Physics", "Collision", "Triggers", "Practice", "Variables", "Sandbox" };
static bool g_show[PANELS] = { true, true, true, true, false, false, false, false, false, false };
static bool g_pin[PANELS] = { true, false, false, false, false, false, false, false, false, false };    // stays on screen (no input) while the overlay is closed

// shown / pinned in debugui.ini as [Zoosters][Panels] (ImGui keeps the window positions there itself)
static void *panelsReadOpen(ImGuiContext *, ImGuiSettingsHandler *, const char *name) { return strcmp(name, "Panels") == 0 ? (void *)1 : 0; }
static void panelsReadLine(ImGuiContext *, ImGuiSettingsHandler *, void *, const char *line)
{
	char name[32];
	int show, pin;
	if (sscanf(line, "%31[^=]=%d,%d", name, &show, &pin) != 3) return;
	for (int p = 0; p < PANELS; p++)
		if (strcmp(name, kPanelNames[p]) == 0) { g_show[p] = show != 0; g_pin[p] = pin != 0; }
}
static void panelsWriteAll(ImGuiContext *, ImGuiSettingsHandler *h, ImGuiTextBuffer *buf)
{
	buf->appendf("[%s][Panels]\n", h->TypeName);
	for (int p = 0; p < PANELS; p++) buf->appendf("%s=%d,%d\n", kPanelNames[p], (int)g_show[p], (int)g_pin[p]);
	buf->append("\n");
}
// before the first NewFrame, which loads the file
void port_debugui_panels_settings(void)
{
	ImGuiSettingsHandler h;
	h.TypeName = "Zoosters";
	h.TypeHash = ImHashStr("Zoosters");
	h.ReadOpenFn = panelsReadOpen;
	h.ReadLineFn = panelsReadLine;
	h.WriteAllFn = panelsWriteAll;
	ImGui::AddSettingsHandler(&h);
}

// ---- per-frame state (updated every present, whether panels are shown or not, so timers and splits never miss a frame)
struct Split { int level; double real, game; int frame; };
static Split g_splits[64];
static int g_nsplits;
static double g_real, g_game, g_levelStartReal, g_levelStartGame;
static int g_levelStartFrame, g_lastLevel = -2;
static LARGE_INTEGER g_qpf, g_t0, g_tPrev;
static float g_maxSpeed, g_maxHSpeed;
static float g_frameMs[120];
static int g_frameMsPos;

static void tick(void)
{
	LARGE_INTEGER now;
	if (!g_qpf.QuadPart) { QueryPerformanceFrequency(&g_qpf); QueryPerformanceCounter(&g_t0); g_tPrev = g_t0; }
	QueryPerformanceCounter(&now);
	g_real = (double)(now.QuadPart - g_t0.QuadPart) / (double)g_qpf.QuadPart;
	port_debugui_physics_tick();
	g_frameMs[g_frameMsPos++ % 120] = (float)((double)(now.QuadPart - g_tPrev.QuadPart) * 1000.0 / (double)g_qpf.QuadPart);
	g_tPrev = now;
	g_game += g_frameDt;
	int lv = g_currentLevel;
	if (lv != g_lastLevel) {
		if (g_lastLevel != -2 && g_nsplits < 64) {
			Split &s = g_splits[g_nsplits++];
			s.level = g_lastLevel; s.real = g_real - g_levelStartReal; s.game = g_game - g_levelStartGame; s.frame = port_frames_presented() - g_levelStartFrame;
		}
		g_lastLevel = lv; g_levelStartReal = g_real; g_levelStartGame = g_game; g_levelStartFrame = port_frames_presented();
		g_maxSpeed = g_maxHSpeed = 0.0f;
	}
}

static void fmtTime(char *buf, size_t n, double t)
{
	int ms = (int)(t * 1000.0 + 0.5);
	_snprintf(buf, n - 1, "%d:%02d.%03d", ms / 60000, ms / 1000 % 60, ms % 1000);
	buf[n - 1] = 0;
}

// window flags for a panel: interactive while the overlay is open, a plain see-through HUD (no input, no title bar) while closed
static int panelFlags(int open) { return open ? ImGuiWindowFlags_AlwaysAutoResize
                                              : ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar |
                                                ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing; }

static void pinBox(int p, int open) { if (open) { ImGui::SameLine(); ImGui::Checkbox("pin", &g_pin[p]); } }
static void help(const char *text)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip()) { ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f); ImGui::TextUnformatted(text); ImGui::PopTextWrapPos(); ImGui::EndTooltip(); }
}

static void timerPanel(int open)
{
	char a[32], b[32];
	ImGui::Text("level %d %s  frame %d", g_currentLevel, levelName(g_currentLevel), port_frames_presented());
	pinBox(PANEL_TIMER, open);
	fmtTime(a, sizeof a, g_real - g_levelStartReal); fmtTime(b, sizeof b, g_game - g_levelStartGame);
	ImGui::Text("level real %s game %s", a, b);
	fmtTime(a, sizeof a, g_real); fmtTime(b, sizeof b, g_game);
	ImGui::Text("total real %s game %s", a, b);
	if (!open) return;
	ImGui::Text("%d frames this level", port_frames_presented() - g_levelStartFrame);
	help("real = wall clock, loads included. game = the sum of the game's tick time (g_frameDt). A split is taken at every level change.");
	if (ImGui::Button("reset timer + splits")) { g_nsplits = 0; g_t0.QuadPart = 0; g_qpf.QuadPart = 0; g_game = 0; g_lastLevel = -2; }
	if (g_nsplits && ImGui::BeginTable("splits", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
		ImGui::TableSetupColumn("level"); ImGui::TableSetupColumn("real"); ImGui::TableSetupColumn("game"); ImGui::TableSetupColumn("frames");
		ImGui::TableHeadersRow();
		for (int i = 0; i < g_nsplits; i++) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("%d %s", g_splits[i].level, levelName(g_splits[i].level));
			fmtTime(a, sizeof a, g_splits[i].real); ImGui::TableNextColumn(); ImGui::TextUnformatted(a);
			fmtTime(a, sizeof a, g_splits[i].game); ImGui::TableNextColumn(); ImGui::TextUnformatted(a);
			ImGui::TableNextColumn(); ImGui::Text("%d", g_splits[i].frame);
		}
		ImGui::EndTable();
	}
}

static void playerPanel(int open)
{
	GameActor *act = port_debugui_player();
	Corpus *c = act ? act->m_corpus : 0;
	if (!c) { ImGui::TextDisabled("no controlled actor (g_cameraFollowActor %p)", (void *)act); pinBox(PANEL_PLAYER, open); return; }
	const RwMatrix &m = c->m_lastFrame;
	const RwV3d &v = c->m_velocity;
	float speed = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z), hspeed = sqrtf(v.x * v.x + v.z * v.z);
	if (speed > g_maxSpeed) g_maxSpeed = speed;
	if (hspeed > g_maxHSpeed) g_maxHSpeed = hspeed;
	float heading = atan2f(m.at.x, m.at.z) * 57.29578f;
	ImGui::Text("pos %9.2f %9.2f %9.2f", m.pos.x, m.pos.y, m.pos.z);
	pinBox(PANEL_PLAYER, open);
	ImGui::Text("vel %9.2f %9.2f %9.2f", v.x, v.y, v.z);
	ImGui::Text("speed %7.2f  horiz %7.2f", speed, hspeed);
	ImGui::Text("max   %7.2f  horiz %7.2f", g_maxSpeed, g_maxHSpeed);
	ImGui::Text("heading %6.1f deg", heading);
	unsigned f = c->m_flags;
	ImGui::Text("%s%s%s%s%s", f & CORPUS_AIRBORNE ? "AIRBORNE " : "", f & CORPUS_GROUND_FOUND ? "GROUND " : "", f & CORPUS_MOVING ? "MOVING " : "",
	            f & CORPUS_COLLIDED ? "COLLIDED " : "", f & CORPUS_HIT_LEDGE ? "LEDGE " : "");
	if (!open) return;
	ImGui::Text("state %.3f s  rise %.3f  late jump %.3f", c->m_stateTime, c->m_riseTime, c->m_lateJumpTime);
	ImGui::Text("jump hold %.3f  jump speed %.2f", c->m_jumpHoldTime > 1e30f ? -1.0f : c->m_jumpHoldTime, c->m_jumpSpeed);
	ImGui::Text("ground normal %.3f %.3f %.3f", c->m_groundNormal.x, c->m_groundNormal.y, c->m_groundNormal.z);
	if (ImGui::Button("reset max")) g_maxSpeed = g_maxHSpeed = 0.0f;
	char tip[256];
	_snprintf(tip, sizeof tip - 1, "The controlled actor's body (Corpus). Velocity is per second after the physics step; max is per level. "
	          "flags %08x, actor %p, corpus %p", f, (void *)act, (void *)c);
	tip[sizeof tip - 1] = 0;
	help(tip);
}

static const char *const kActionNames[] = { 0, 0, 0, 0, "jump", "action", "fire", "alt attack", "screenshot", "white", "reset cam", "show ui",
	"pause", "back", "L thumb", "R thumb", "forward", "backward", "turn left", "turn right", "cam up", "cam down", "cam left", "cam right",
	"dpad up", "dpad down", "dpad left", "dpad right" };

static float actionValue(const CtlMap &map, int action)
{
	float best = 0.0f;
	for (int s = 0; s < CTL_SLOTS; s++)
		if (map.m_entries[action][s].m_type && fabsf(map.m_entries[action][s].m_value) > fabsf(best)) best = map.m_entries[action][s].m_value;
	return best;
}

static void inputPanel(int open)
{
	InputManager *im = g_inputManager;
	if (!im) { ImGui::TextDisabled("no input manager yet"); pinBox(PANEL_INPUT, open); return; }
	char keys[512] = "";
	for (int k = 0; k < 0x100; k++)
		if (im->m_keys[k] & 0x80) {
			size_t n = strlen(keys);
			_snprintf(keys + n, sizeof keys - n - 1, "%s%s", n ? " " : "", im->m_keyNames[k][0] ? im->m_keyNames[k] : "?");
		}
	ImGui::Text("keys  %s", keys[0] ? keys : "-");
	pinBox(PANEL_INPUT, open);
	const DIMOUSESTATE2 &ms = im->m_mouseState;
	ImGui::Text("mouse dx %4d dy %4d wheel %4d  buttons %c%c%c", ms.lX, ms.lY, ms.lZ, ms.rgbButtons[0] ? 'L' : '-', ms.rgbButtons[1] ? 'R' : '-',
	            ms.rgbButtons[2] ? 'M' : '-');
	CtlBuf *ctx = im->m_contexts[0];
	int idx = -1;
	for (int i = 0; i < 9; i++) if (ctx && g_ctlBuffers[i] == ctx) idx = i;
	ImGui::Text("context %d%s", idx, ctx && ctx->m_accumulateMouse ? " (mouse accumulates)" : "");
	if (!ctx) return;
	const CtlMap &map = ctx->m_map;
	float fx = actionValue(map, CTL_ACTION_TURN_RIGHT) - actionValue(map, CTL_ACTION_TURN_LEFT);
	float fy = actionValue(map, CTL_ACTION_FORWARD) - actionValue(map, CTL_ACTION_BACKWARD);
	float cx = actionValue(map, CTL_ACTION_CAMERA_RIGHT) - actionValue(map, CTL_ACTION_CAMERA_LEFT);
	float cy = actionValue(map, CTL_ACTION_CAMERA_UP) - actionValue(map, CTL_ACTION_CAMERA_DOWN);
	// two stick plots: move (left) and camera (right)
	ImDrawList *dl = ImGui::GetWindowDrawList();
	ImVec2 p = ImGui::GetCursorScreenPos();
	const float r = 28.0f;
	for (int s = 0; s < 2; s++) {
		ImVec2 c(p.x + r + 2 + s * (2 * r + 16), p.y + r + 2);
		float x = s ? cx : fx, y = s ? cy : fy;
		dl->AddCircle(c, r, IM_COL32(200, 200, 200, 160), 24);
		dl->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x + r, c.y), IM_COL32(120, 120, 120, 120));
		dl->AddLine(ImVec2(c.x, c.y - r), ImVec2(c.x, c.y + r), IM_COL32(120, 120, 120, 120));
		dl->AddCircleFilled(ImVec2(c.x + x * r, c.y - y * r), 4.0f, IM_COL32(255, 210, 60, 255));
	}
	ImGui::Dummy(ImVec2(4 * r + 20, 2 * r + 6));
	ImGui::SameLine();
	char held[256] = "";
	for (int a = 4; a <= 0xf; a++)
		if (kActionNames[a] && actionValue(map, a) > 0.5f) { size_t n = strlen(held); _snprintf(held + n, sizeof held - n - 1, "%s%s", n ? "\n" : "", kActionNames[a]); }
	ImGui::TextUnformatted(held[0] ? held : "-");
	if (!open) return;
	if (ImGui::TreeNode("all actions")) {
		for (int a = 4; a < (int)(sizeof kActionNames / sizeof kActionNames[0]); a++)
			ImGui::Text("%02x %-12s %6.3f", a, kActionNames[a], actionValue(map, a));
		ImGui::TreePop();
	}
}

static void renderPanel(int open)
{
	float avg = 0.0f, worst = 0.0f;
	for (int i = 0; i < 120; i++) { avg += g_frameMs[i]; if (g_frameMs[i] > worst) worst = g_frameMs[i]; }
	avg /= 120.0f;
	ImGui::Text("%.2f ms (%.0f fps) worst %.2f", avg, avg > 0 ? 1000.0f / avg : 0.0f, worst);
	pinBox(PANEL_RENDER, open);
	ImGui::Text("world %d atomic %d d3d %d", port_draw_world, port_draw_atomic, port_draw_d3d);
	ImGui::Text("im2d %d im3d %d  ticks %.0f/s", port_draw_im2d, port_draw_im3d, g_frameRate);
	if (!open) return;
	ImGui::PlotLines("##ms", g_frameMs, 120, g_frameMsPos % 120, "frame ms (0-50)", 0.0f, 50.0f, ImVec2(240, 40));
	static const char *const names[PORT_DBG_PASSES] = { "world", "atomics", "Im2D/HUD", "Im3D", "D3D" };
	ImGui::Text("hide:");
	help("Hides a draw category (drawing only: the game still runs it, the counters still count it). world = level geometry (world "
	     "sectors), atomics = characters and props, Im2D = HUD / menus / fades, Im3D = immediate 3D (particles, lines), D3D = the game's "
	     "own direct draws.");
	for (int i = 0; i < PORT_DBG_PASSES; i++) {
		bool h = port_dbg_hide[i] != 0;
		if (ImGui::Checkbox(names[i], &h)) port_dbg_hide[i] = h;
		if (i != 2 && i + 1 < PORT_DBG_PASSES) ImGui::SameLine();
	}
}

static void physicsPanel(int open) { port_debugui_timeline_panel(open); pinBox(PANEL_PHYSICS, open); }
static void collisionPanel(int open) { port_debugui_collision_panel(open); pinBox(PANEL_COLLISION, open); }
static void triggersPanel(int open) { port_debugui_triggers_panel(open); pinBox(PANEL_TRIGGERS, open); }
static void practicePanel(int open) { port_debugui_practice_panel(open); pinBox(PANEL_PRACTICE, open); }
static void varsPanel(int open) { port_debugui_vars_panel(open); pinBox(PANEL_VARS, open); }
static void sandboxPanel(int open) { port_debugui_sandbox_panel(open); pinBox(PANEL_SANDBOX, open); }

extern "C" void port_debugui_panels(int open)
{
	static bool first = true;
	if (first) {   // test aid: PORT_DEBUGUI_SHOW=<panel,...> (0 Timer .. 9 Sandbox) = exactly these panels shown, and pinned
		first = false;
		if (const char *e = getenv("PORT_DEBUGUI_SHOW")) {
			for (int p = 0; p < PANELS; p++) g_show[p] = g_pin[p] = false;
			for (const char *h = e; h && *h; ) { int k = atoi(h); if (k >= 0 && k < PANELS) g_show[k] = g_pin[k] = true; h = strchr(h, ','); if (h) h++; }
		}
	}
	tick();
	port_debugui_practice_tick(open);
	port_debugui_studio_tick(open);
	if (open) {
		ImGui::SetNextWindowPos(ImVec2(8, 8), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Zoosters debug (F1)", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
			for (int p = 0; p < PANELS; p++) { ImGui::Checkbox(kPanelNames[p], &g_show[p]); if (p + 1 < PANELS && p % 5 != 4) ImGui::SameLine(); }
			help("F1 opens and closes this overlay. Panels with \"pin\" checked stay on screen (no input) while it is closed.");
		}
		ImGui::End();
	}
	static bool saved[2][PANELS];   // ImGui writes the file only when something is marked dirty
	if (memcmp(saved[0], g_show, sizeof g_show) || memcmp(saved[1], g_pin, sizeof g_pin)) {
		memcpy(saved[0], g_show, sizeof g_show); memcpy(saved[1], g_pin, sizeof g_pin);
		ImGui::MarkIniSettingsDirty();
	}
	static void (*const draw[PANELS])(int) = { timerPanel, playerPanel, inputPanel, renderPanel, physicsPanel, collisionPanel, triggersPanel, practicePanel, varsPanel, sandboxPanel };
	static const ImVec2 pos[PANELS] = { ImVec2(8, 64), ImVec2(8, 200), ImVec2(330, 272), ImVec2(330, 64), ImVec2(380, 8), ImVec2(380, 330), ImVec2(8, 330), ImVec2(200, 120), ImVec2(8, 300), ImVec2(770, 8) };
	// world-space draws: a panel that is switched on (shown) draws every frame, also while it is not on screen (overlay closed, not pinned)
	if (g_show[PANEL_COLLISION]) port_debugui_collision_draw();
	if (g_show[PANEL_TRIGGERS]) port_debugui_triggers_draw(open || g_pin[PANEL_TRIGGERS]);
	// Variables and Sandbox: a fixed default size (resizable, scrolling) inside the display, 1280x720 at least; "##2" gives them a fresh ini
	// entry (the first version stored an auto-sized window). Pinned (overlay closed) they are small auto-sized HUDs at the same place.
	static const ImVec2 size[PANELS] = { ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0),
	                                     ImVec2(620, 400), ImVec2(500, 704) };
	for (int p = 0; p < PANELS; p++) {
		if (!g_show[p] || (!open && !g_pin[p])) continue;
		ImVec2 ps = pos[p];
		int flags = panelFlags(open);
		char title[48];
		snprintf(title, sizeof title, "%s", kPanelNames[p]);
		if (size[p].x > 0) {
			ImVec2 ds = ImGui::GetIO().DisplaySize, sz(ImMin(size[p].x, ds.x - 16), ImMin(size[p].y, ds.y - 16));
			ps.x = ImClamp(ps.x, 8.0f, ImMax(8.0f, ds.x - sz.x - 8)); ps.y = ImClamp(ps.y, 8.0f, ImMax(8.0f, ds.y - sz.y - 8));
			char full[48];
			snprintf(full, sizeof full, "%s##2", kPanelNames[p]);
			if (open) {
				flags &= ~ImGuiWindowFlags_AlwaysAutoResize;
				ImGui::SetNextWindowSize(sz, ImGuiCond_FirstUseEver);
				snprintf(title, sizeof title, "%s", full);
			} else {
				snprintf(title, sizeof title, "%s##2hud", kPanelNames[p]);
				if (ImGuiWindow *w = ImGui::FindWindowByName(full)) ps = w->Pos;
				ImGui::SetNextWindowPos(ps, ImGuiCond_Always);
			}
		}
		if (open || size[p].x <= 0) ImGui::SetNextWindowPos(ps, ImGuiCond_FirstUseEver);
		if (!open) ImGui::SetNextWindowBgAlpha(0.45f);
		if (ImGui::Begin(title, open ? &g_show[p] : 0, flags)) draw[p](open);
		ImGui::End();
	}
}
