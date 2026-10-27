// Practice tools (ZOOSTERS_DEBUGUI). Unlike the other panels these change the game:
//   pause / frame advance / slow motion  the logic step is held (RWS::MainLoop::Poll asks port_debugui_logic_allowed before it runs
//                                        Logic::Poll; rendering goes on, so the overlay stays usable on the frozen frame)
//   saved positions                      the controlled actor's position, facing and velocities; restored through the game's own
//                                        GameActor::PlaceAt (via Corpus::LeaveGround: the body starts airborne) and SetFacing
//   free camera                          debugui_freecam.cpp
//   input recording                      the keys, mouse movement, wheel and buttons the game polled, written as a PORT_INPUT script
//                                        (game_stubs.cpp format) next to the exe: recording_<first frame>.input
// Hotkeys (work with the overlay closed): F2 pause / resume, F3 one logic step while paused, F4 slow motion 1 -> 1/2 -> 1/4 -> 1/8,
// F5 save the selected slot, F6 restore it, F7 free camera on / off, F8 start / stop recording the input, F10 HUD layout of the next
// frame to port.log (widescreen builds: which element went to which edge / scale; port/fixes/widescreen.cpp).
#ifndef ZOOSTERS_DEBUGUI
#error debugui_practice.cpp is built only with ZOOSTERS_DEBUGUI (build option --debugui)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imgui.h"
#include "debugui.h"
#ifdef ZOOSTERS_WIDESCREEN
extern "C" void port_wide2d_log_frame(int frame);   // port/fixes/widescreen.cpp
#endif
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "tfb/collision/collider.h"
#include "framework/startup/win32/input/inputmanager.h"
#include "debugui_platform.h"

using RWS::TFB::GameActor;
extern int g_currentLevel;               // GLOBAL: 0x606a38 g_currentLevel
extern "C" int port_frames_presented(void);
extern int port_freecam_on;
extern float port_freecam_speed;
void port_debugui_freecam_update(int open);
void port_debugui_freecam_offset(float up, float back);

// ---- pause / step / slow motion
static bool g_paused;
static int g_steps;                  // logic steps still allowed while paused
static int g_slow = 1;               // run one logic step in g_slow
static unsigned g_slowCount;
static unsigned g_heldSteps, g_ranSteps;
// The free camera pauses the logic by default: the game's logic depends on what was rendered (particle systems tick and draw
// random numbers only while their visibility flag, set by the frustum test QueueVisibleParticles 0x40b3d0, is on), so a different
// view changes the random sequence of the whole game.
static bool g_freecamPauses = true;

bool port_debugui_logic_allowed()
{
	bool run;
	if (g_paused || (port_freecam_on && g_freecamPauses)) {
		run = g_steps > 0;
		if (run) g_steps--;
	} else {
		run = g_slow <= 1 || (++g_slowCount % (unsigned)g_slow) == 0;
	}
	if (run) { g_ranSteps++; port_debugui_vars_apply(); } else g_heldSteps++;
	return run;
}

// ---- saved positions
struct SavedPos { bool used; int level; Vec3 pos; float facing; RwV3d bodyVel, moveVel; };
static SavedPos g_slots[4];
static int g_slot;
static char g_msg[96];

static void save(int k)
{
	GameActor *a = port_debugui_player();
	Corpus *c = a ? a->m_corpus : 0;
	if (!c) { strcpy(g_msg, "nothing to save: no controlled actor"); return; }
	SavedPos &s = g_slots[k];
	s.used = true;
	s.level = g_currentLevel;
	s.pos.x = c->m_lastFrame.pos.x; s.pos.y = c->m_lastFrame.pos.y; s.pos.z = c->m_lastFrame.pos.z;
	s.facing = a->GetFacing();
	s.bodyVel = c->m_velocity;
	s.moveVel = static_cast<RWS::TFB::Collider *>(a)->m_velocity;
	_snprintf(g_msg, sizeof g_msg - 1, "slot %d saved at frame %d", k + 1, port_frames_presented());
}

static void restore(int k)
{
	GameActor *a = port_debugui_player();
	Corpus *c = a ? a->m_corpus : 0;
	const SavedPos &s = g_slots[k];
	if (!c || !s.used) { strcpy(g_msg, "nothing to restore"); return; }
	if (s.level != g_currentLevel) { _snprintf(g_msg, sizeof g_msg - 1, "slot %d belongs to level %d", k + 1, s.level); return; }
	a->PlaceAt(&s.pos);
	a->SetFacing(s.facing, 1);
	c->m_velocity = s.bodyVel;
	static_cast<RWS::TFB::Collider *>(a)->m_velocity = s.moveVel;
	_snprintf(g_msg, sizeof g_msg - 1, "slot %d restored at frame %d", k + 1, port_frames_presented());
}

// ---- input recording: one sample per present of what the game polled (InputManager::m_keys / m_mouseState). The poll that ran before
// this present saw the presented-frame counter one lower (RwCameraShowRaster counts first), so a sample is filed under frame - 1, the
// frame number game_stubs.cpp's PORT_INPUT replay compares with. Keys and mouse buttons become `<first frame> <key> <frames held>`.
struct RecEvent { int frame; char text[40]; };
static RecEvent *g_rec;
static int g_nrec, g_capRec;
static bool g_recording;
static int g_recStart, g_recLevel, g_recLast;
static int g_keyDownAt[256], g_btnDownAt[8];
static char g_recFile[MAX_PATH];

static void recAdd(int frame, const char *text)
{
	if (g_nrec == g_capRec) {
		int cap = g_capRec ? g_capRec * 2 : 4096;
		RecEvent *n = (RecEvent *)realloc(g_rec, sizeof(RecEvent) * cap);
		if (!n) return;
		g_rec = n; g_capRec = cap;
	}
	g_rec[g_nrec].frame = frame;
	strncpy(g_rec[g_nrec].text, text, sizeof g_rec[g_nrec].text - 1);
	g_rec[g_nrec].text[sizeof g_rec[g_nrec].text - 1] = 0;
	g_nrec++;
}

static void recStart(void)
{
	g_nrec = 0;
	g_recording = true;
	g_recStart = port_frames_presented() - 1;
	g_recLast = g_recStart;
	g_recLevel = g_currentLevel;
	for (int k = 0; k < 256; k++) g_keyDownAt[k] = -1;
	for (int b = 0; b < 8; b++) g_btnDownAt[b] = -1;
	_snprintf(g_msg, sizeof g_msg - 1, "recording from frame %d", g_recStart);
}

static int cmpRec(const void *a, const void *b) { return ((const RecEvent *)a)->frame - ((const RecEvent *)b)->frame; }

static void recStop(void)
{
	char t[40];
	for (int k = 0; k < 256; k++)                       // keys still down: held up to the last sample
		if (g_keyDownAt[k] >= 0) { _snprintf(t, sizeof t - 1, "0x%02x %d", k, g_recLast - g_keyDownAt[k] + 1); t[sizeof t - 1] = 0; recAdd(g_keyDownAt[k], t); }
	for (int b = 0; b < 8; b++)
		if (g_btnDownAt[b] >= 0) { _snprintf(t, sizeof t - 1, "mbutton %d %d", b, g_recLast - g_btnDownAt[b] + 1); t[sizeof t - 1] = 0; recAdd(g_btnDownAt[b], t); }
	g_recording = false;
	qsort(g_rec, g_nrec, sizeof(RecEvent), cmpRec);
	char name[64], path[1024];
	snprintf(name, sizeof name, "recording_%d.input", g_recStart);
	if (!port_debugui_file(name, path, sizeof path)) return;
	FILE *f = fopen(path, "w");
	if (!f) { strcpy(g_msg, "recording: cannot write the file"); return; }
	fprintf(f, "# frames absolute\n");
	fprintf(f, "# recorded by the debug overlay (F8): level %d, presented frames %d..%d, %d events\n", g_recLevel, g_recStart, g_recLast, g_nrec);
	fprintf(f, "# replay: PORT_INPUT=<this file>, the game started the same way (same level path; exact only when both runs use PORT_FIXEDTIME)\n");
	for (int i = 0; i < g_nrec; i++) fprintf(f, "%d %s\n", g_rec[i].frame, g_rec[i].text);
	fclose(f);
	strncpy(g_recFile, path, sizeof g_recFile - 1);
	_snprintf(g_msg, sizeof g_msg - 1, "recording: %d events written", g_nrec);
}

static void recSample(void)
{
	InputManager *im = g_inputManager;
	if (!g_recording || !im) return;
	int f = port_frames_presented() - 1;
	if (f <= g_recLast && g_nrec) return;              // one sample per frame
	g_recLast = f;
	char t[40];
	for (int k = 0; k < 256; k++) {
		bool down = (im->m_keys[k] & 0x80) != 0;
		if (down && g_keyDownAt[k] < 0) g_keyDownAt[k] = f;
		if (!down && g_keyDownAt[k] >= 0) {
			_snprintf(t, sizeof t - 1, "0x%02x %d", k, f - g_keyDownAt[k]); t[sizeof t - 1] = 0;
			recAdd(g_keyDownAt[k], t);
			g_keyDownAt[k] = -1;
		}
	}
	const DIMOUSESTATE2 &ms = im->m_mouseState;
	if (ms.lX || ms.lY) { _snprintf(t, sizeof t - 1, "mouse %d %d", ms.lX, ms.lY); t[sizeof t - 1] = 0; recAdd(f, t); }
	if (ms.lZ / 120) { _snprintf(t, sizeof t - 1, "wheel %d", ms.lZ / 120); t[sizeof t - 1] = 0; recAdd(f, t); }
	for (int b = 0; b < 8; b++) {
		bool down = ms.rgbButtons[b] != 0;
		if (down && g_btnDownAt[b] < 0) g_btnDownAt[b] = f;
		if (!down && g_btnDownAt[b] >= 0) {
			_snprintf(t, sizeof t - 1, "mbutton %d %d", b, f - g_btnDownAt[b]); t[sizeof t - 1] = 0;
			recAdd(g_btnDownAt[b], t);
			g_btnDownAt[b] = -1;
		}
	}
}

static void hotkeys(void)
{
	if (ImGui::IsKeyPressed(ImGuiKey_F2, false)) { g_paused = !g_paused; g_steps = 0; }
	if (ImGui::IsKeyPressed(ImGuiKey_F3, true) && (g_paused || (port_freecam_on && g_freecamPauses))) g_steps++;
	if (ImGui::IsKeyPressed(ImGuiKey_F4, false)) g_slow = g_slow >= 8 ? 1 : g_slow * 2;
	if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) save(g_slot);
	if (ImGui::IsKeyPressed(ImGuiKey_F6, false)) restore(g_slot);
	if (ImGui::IsKeyPressed(ImGuiKey_F7, false)) port_freecam_on = !port_freecam_on;
	if (ImGui::IsKeyPressed(ImGuiKey_F8, false)) { if (g_recording) recStop(); else recStart(); }
#ifdef ZOOSTERS_WIDESCREEN
	if (ImGui::IsKeyPressed(ImGuiKey_F10, false)) port_wide2d_log_frame(port_frames_presented() + 1);   // HUD layout of the next frame -> port.log
#endif
}

// test aids: PORT_DEBUGUI_PAUSE / _FREECAM / _SAVE / _RESTORE=<frame>: pause, free camera on, save slot 1, restore slot 1 at that frame
static void testAids(void)
{
	static int pauseAt = getenv("PORT_DEBUGUI_PAUSE") ? atoi(getenv("PORT_DEBUGUI_PAUSE")) : -1;
	static int camAt = getenv("PORT_DEBUGUI_FREECAM") ? atoi(getenv("PORT_DEBUGUI_FREECAM")) : -1;
	static int saveAt = getenv("PORT_DEBUGUI_SAVE") ? atoi(getenv("PORT_DEBUGUI_SAVE")) : -1;          // slot 1 at that frame
	static int restoreAt = getenv("PORT_DEBUGUI_RESTORE") ? atoi(getenv("PORT_DEBUGUI_RESTORE")) : -1;
	static const char *rec = getenv("PORT_DEBUGUI_RECORD");                                             // "<from>,<to>": record that span
	static int recFrom = rec ? atoi(rec) : -1, recTo = rec && strchr(rec, ',') ? atoi(strchr(rec, ',') + 1) : -1;
	if (recFrom >= 0 && port_frames_presented() >= recFrom) { recStart(); recFrom = -1; }
	if (recTo >= 0 && g_recording && port_frames_presented() >= recTo) { recStop(); recTo = -1; }
	int f = port_frames_presented();
	if (saveAt >= 0 && f >= saveAt) { save(0); saveAt = -1; }
	if (restoreAt >= 0 && f >= restoreAt) { restore(0); restoreAt = -1; }
	if (pauseAt >= 0 && f >= pauseAt) { g_paused = true; g_steps = 0; pauseAt = -1; }
	if (camAt >= 0 && f >= camAt) {   // PORT_DEBUGUI_FREECAM=<frame>[,<up>,<back>[,live]]: also moves it up / back from the game view
		port_freecam_on = 1;
		camAt = -1;
		const char *e = strchr(getenv("PORT_DEBUGUI_FREECAM"), ',');
		float up = 0.0f, back = 0.0f;
		if (e) sscanf(e + 1, "%f,%f", &up, &back);
		port_debugui_freecam_offset(up, back);
		if (strstr(getenv("PORT_DEBUGUI_FREECAM"), "live")) g_freecamPauses = false;
	}
}

// every frame: hotkeys, free camera movement, the status banner; the panel itself when shown
void port_debugui_practice_tick(int open)
{
	testAids();
	hotkeys();
	recSample();
	port_debugui_freecam_update(open);
	char banner[96] = "";
	if (g_paused) strcat(banner, "PAUSED (F2 resume, F3 step)  ");
	else if (g_slow > 1) _snprintf(banner + strlen(banner), 40, "SLOW 1/%d (F4)  ", g_slow);
	if (g_recording) _snprintf(banner + strlen(banner), 40, "REC %d (F8)  ", port_frames_presented() - 1 - g_recStart);
	if (port_freecam_on) strcat(banner, g_freecamPauses ? "FREE CAMERA, paused (F7)" : "FREE CAMERA (F7; random numbers diverge)");
	if (banner[0]) {
		ImDrawList *dl = ImGui::GetForegroundDrawList();
		ImVec2 sz = ImGui::CalcTextSize(banner);
		ImVec2 p((ImGui::GetIO().DisplaySize.x - sz.x) * 0.5f, 6.0f);
		dl->AddRectFilled(ImVec2(p.x - 6, p.y - 3), ImVec2(p.x + sz.x + 6, p.y + sz.y + 3), IM_COL32(0, 0, 0, 170), 4.0f);
		dl->AddText(p, IM_COL32(255, 220, 80, 255), banner);
	}
}

void port_debugui_practice_panel(int open)
{
	bool held = g_paused || (port_freecam_on && g_freecamPauses);
	ImGui::Text("%s  logic steps run %u, held %u", held ? "PAUSED" : g_slow > 1 ? "SLOW" : "running", g_ranSteps, g_heldSteps);
	if (!open) return;
	ImGui::TextDisabled("These change the game (unlike the other panels). Hotkeys work with the overlay closed.");
	if (ImGui::Button(g_paused ? "resume (F2)" : "pause (F2)")) { g_paused = !g_paused; g_steps = 0; }
	ImGui::SameLine();
	if (ImGui::Button("step (F3)") && g_paused) g_steps++;
	ImGui::SameLine();
	ImGui::SetNextItemWidth(110);
	static const char *const kSlow[] = { "1/1", "1/2", "1/4", "1/8" };
	int si = g_slow >= 8 ? 3 : g_slow >= 4 ? 2 : g_slow >= 2 ? 1 : 0;
	if (ImGui::Combo("speed (F4)", &si, kSlow, 4)) g_slow = 1 << si;
	ImGui::Separator();
	ImGui::Text("saved positions (F5 save, F6 restore the selected slot)");
	for (int k = 0; k < 4; k++) {
		ImGui::PushID(k);
		ImGui::RadioButton("##sel", &g_slot, k);
		ImGui::SameLine();
		if (g_slots[k].used)
			ImGui::Text("%d: level %d  %.1f %.1f %.1f  facing %.0f", k + 1, g_slots[k].level, g_slots[k].pos.x, g_slots[k].pos.y, g_slots[k].pos.z,
			            g_slots[k].facing);
		else
			ImGui::TextDisabled("%d: empty", k + 1);
		ImGui::SameLine();
		if (ImGui::SmallButton("save")) save(k);
		ImGui::SameLine();
		if (ImGui::SmallButton("restore")) restore(k);
		ImGui::PopID();
	}
	if (g_msg[0]) ImGui::TextDisabled("%s", g_msg);
	ImGui::Separator();
	bool fc = port_freecam_on != 0;
	if (ImGui::Checkbox("free camera (F7)", &fc)) port_freecam_on = fc;
	ImGui::SameLine();
	ImGui::Checkbox("pauses the game", &g_freecamPauses);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("The game's logic depends on what is rendered: particle systems only tick (and draw random numbers) while "
		                  "visible. Playing on with another view changes the random sequence of the whole game.");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120);
	ImGui::SliderFloat("speed", &port_freecam_speed, 50.0f, 3000.0f, "%.0f u/s");
	ImGui::TextDisabled("W/A/S/D move, Q/E down/up, Shift x4, arrows or right-drag turn; the game gets no keys meanwhile");
	ImGui::Separator();
	if (ImGui::Button(g_recording ? "stop recording (F8)" : "record input (F8)")) { if (g_recording) recStop(); else recStart(); }
	if (g_recording) { ImGui::SameLine(); ImGui::Text("frame %d, %d events", port_frames_presented() - 1 - g_recStart, g_nrec); }
	if (g_recFile[0]) ImGui::TextDisabled("%s", g_recFile);
}
