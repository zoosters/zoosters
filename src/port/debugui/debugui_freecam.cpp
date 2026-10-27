// Free camera (ZOOSTERS_DEBUGUI). While on, every BeginUpdate of the game camera (g_gameCamDst) renders through the free matrix
// (rwcore.cpp camBegin -> port_debugui_camera_begin) and the camera frame gets its own matrix back after the overlay
// (port_debugui_camera_restore), so the game's logic never sees it. Culling and level-of-detail follow the free view. Keys:
// W/A/S/D move, Q/E down/up, Shift faster, arrows turn; right-drag turns while the overlay is open. The game gets no keyboard
// meanwhile (port_debugui_freecam_keys).
#ifndef ZOOSTERS_DEBUGUI
#error debugui_freecam.cpp is built only with ZOOSTERS_DEBUGUI (build option --debugui)
#endif
#include <math.h>
#include <string.h>
#ifdef RW_D3D9
#include <d3d9.h>
#endif
#include <rw.h>
#include "imgui.h"
#include "debugui.h"

#include "../provisional/game_access.h"
#define g_gameCamDst (PortGame_CamDst())   // GLOBAL: 0x6182d0 g_gameCamDst
extern bool g_gameCamValid;   // GLOBAL: 0x6182d4 g_gameCamValid

int port_freecam_on;
float port_freecam_speed = 300.0f;     // world units per second
static float g_pos[3], g_yaw, g_pitch;
static float g_handed = 1.0f;          // sign that gives the game camera's own handedness of right x up
static bool g_inited;
static rw::Camera *g_swapCam;
static rw::Matrix g_saved;

static rw::Camera *gameCamera(void) { return g_gameCamDst && g_gameCamValid ? (rw::Camera *)g_gameCamDst : 0; }

// starts from the game camera's current view
static void initFromGame(void)
{
	rw::Camera *cam = gameCamera();
	if (!cam || !cam->getFrame()) return;
	rw::Matrix *m = cam->getFrame()->getLTM();
	g_pos[0] = m->pos.x; g_pos[1] = m->pos.y; g_pos[2] = m->pos.z;
	g_yaw = atan2f(m->at.x, m->at.z);
	g_pitch = asinf(m->at.y < -1.0f ? -1.0f : m->at.y > 1.0f ? 1.0f : m->at.y);
	rw::V3d upw = rw::makeV3d(0.0f, 1.0f, 0.0f);
	g_handed = rw::dot(rw::cross(upw, m->at), m->right) < 0.0f ? -1.0f : 1.0f;
	g_inited = true;
}

static void buildMatrix(rw::Matrix *out)
{
	rw::V3d f = rw::makeV3d(sinf(g_yaw) * cosf(g_pitch), sinf(g_pitch), cosf(g_yaw) * cosf(g_pitch));
	rw::V3d upw = rw::makeV3d(0.0f, 1.0f, 0.0f);
	rw::V3d r = rw::scale(rw::normalize(rw::cross(upw, f)), g_handed);
	rw::V3d u = rw::normalize(rw::cross(f, r));
	if (u.y < 0.0f) u = rw::neg(u);
	out->right = r; out->up = u; out->at = f;
	out->pos = rw::makeV3d(g_pos[0], g_pos[1], g_pos[2]);
	out->flags = 0;
	out->update();
}

// Every BeginUpdate swaps the free view in and the matching EndUpdate (port_debugui_camera_end, rwcore.cpp camEnd) swaps the
// camera's own matrix back, so code between two updates (the logic, CTFBWorld::RenderSkyPass's own save / restore of the
// position) sees the game's matrix. The sky pass moves the camera to the origin for its update: the free view keeps that.
extern "C" void port_debugui_camera_begin(void *camera)
{
	rw::Camera *cam = (rw::Camera *)camera;
	if (!port_freecam_on || !cam || cam != gameCamera() || g_swapCam == cam) return;
	rw::Frame *fr = cam->getFrame();
	if (!fr) return;
	if (!g_inited) initFromGame();
	g_saved = fr->matrix;
	rw::Matrix world, local;
	buildMatrix(&world);
	if (g_saved.pos.x == 0.0f && g_saved.pos.y == 0.0f && g_saved.pos.z == 0.0f)   // the sky pass: the free rotation at the origin
		world.pos = rw::makeV3d(0.0f, 0.0f, 0.0f);
	if (rw::Frame *parent = fr->getParent()) {                 // the frame's matrix is local to its parent
		rw::Matrix inv;
		rw::Matrix::invert(&inv, parent->getLTM());
		rw::Matrix::mult(&local, &world, &inv);
	} else {
		local = world;
	}
	fr->matrix = local;
	fr->updateObjects();
	g_swapCam = cam;
}

void port_debugui_camera_restore(void)
{
	if (!g_swapCam) return;
	rw::Frame *fr = g_swapCam->getFrame();
	if (fr) { fr->matrix = g_saved; fr->updateObjects(); }
	g_swapCam = 0;
}
extern "C" void port_debugui_camera_end(void *camera)
{
	if (g_swapCam && g_swapCam == (rw::Camera *)camera) port_debugui_camera_restore();
}

// The view the frame is rendered with (free camera, else game camera), for picking and the gizmo: position, forward, screen-right
// (forward x world up), screen-up, the view window (half extents at distance 1) and the clip planes. False when there is no game
// camera.
bool port_debugui_view(float *pos, float *fwd, float *right, float *up, float *vw, float *nearFar)
{
	rw::Camera *cam = gameCamera();
	if (!cam || !cam->getFrame()) return false;
	rw::Matrix m;
	if (port_freecam_on && g_inited) buildMatrix(&m);
	else m = *cam->getFrame()->getLTM();
	rw::V3d f = rw::normalize(m.at);
	rw::V3d r = rw::normalize(rw::cross(f, rw::makeV3d(0.0f, 1.0f, 0.0f)));
	rw::V3d u = rw::cross(r, f);
	pos[0] = m.pos.x; pos[1] = m.pos.y; pos[2] = m.pos.z;
	fwd[0] = f.x; fwd[1] = f.y; fwd[2] = f.z;
	right[0] = r.x; right[1] = r.y; right[2] = r.z;
	up[0] = u.x; up[1] = u.y; up[2] = u.z;
	vw[0] = cam->viewWindow.x; vw[1] = cam->viewWindow.y;
	nearFar[0] = cam->nearPlane; nearFar[1] = cam->farPlane;
	return true;
}

// puts the free camera `dist` units in front of a point, looking at it slightly from above (the outliner's "focus")
void port_debugui_freecam_focus(const float *target, float dist)
{
	if (!g_inited) initFromGame();
	g_pitch = -0.4f;
	float f[3] = { sinf(g_yaw) * cosf(g_pitch), sinf(g_pitch), cosf(g_yaw) * cosf(g_pitch) };
	for (int k = 0; k < 3; k++) g_pos[k] = target[k] - f[k] * dist;
}

int port_debugui_freecam_keys(void) { return port_freecam_on; }

static void applyPendingOffset(void);

// once per frame (from the practice panel): moves and turns with the keys / mouse
void port_debugui_freecam_update(int open)
{
	if (!port_freecam_on) { g_inited = false; return; }
	if (!g_inited) initFromGame();
	if (g_inited) applyPendingOffset();
	ImGuiIO &io = ImGui::GetIO();
	float dt = io.DeltaTime > 0.1f ? 0.1f : io.DeltaTime;
	float turn = 1.6f * dt;
	if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) g_yaw += turn * g_handed;
	if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) g_yaw -= turn * g_handed;
	if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) g_pitch += turn;
	if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) g_pitch -= turn;
	if (open && ImGui::IsMouseDown(ImGuiMouseButton_Right) && !io.WantCaptureMouse) {
		g_yaw -= io.MouseDelta.x * 0.005f * g_handed;
		g_pitch -= io.MouseDelta.y * 0.005f;
	}
	if (g_pitch > 1.55f) g_pitch = 1.55f;
	if (g_pitch < -1.55f) g_pitch = -1.55f;
	float v = port_freecam_speed * dt * (ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 4.0f : 1.0f);
	rw::Matrix m;
	buildMatrix(&m);
	float mv[3] = { 0, 0, 0 };
	if (ImGui::IsKeyDown(ImGuiKey_W)) { mv[0] += m.at.x; mv[1] += m.at.y; mv[2] += m.at.z; }
	if (ImGui::IsKeyDown(ImGuiKey_S)) { mv[0] -= m.at.x; mv[1] -= m.at.y; mv[2] -= m.at.z; }
	if (ImGui::IsKeyDown(ImGuiKey_D)) { mv[0] -= m.right.x * g_handed; mv[2] -= m.right.z * g_handed; }
	if (ImGui::IsKeyDown(ImGuiKey_A)) { mv[0] += m.right.x * g_handed; mv[2] += m.right.z * g_handed; }
	if (ImGui::IsKeyDown(ImGuiKey_E)) mv[1] += 1.0f;
	if (ImGui::IsKeyDown(ImGuiKey_Q)) mv[1] -= 1.0f;
	for (int k = 0; k < 3; k++) g_pos[k] += mv[k] * v;
}

// test aid: move the free camera `up` world units up and `back` units back along its view (from the game view on first use)
static float g_pendUp, g_pendBack;
void port_debugui_freecam_offset(float up, float back) { g_pendUp = up; g_pendBack = back; }
static void applyPendingOffset(void)
{
	if (g_pendUp == 0.0f && g_pendBack == 0.0f) return;
	rw::Matrix m;
	buildMatrix(&m);
	g_pos[0] -= m.at.x * g_pendBack; g_pos[1] -= m.at.y * g_pendBack - g_pendUp; g_pos[2] -= m.at.z * g_pendBack;
	g_pitch -= 0.35f;                                       // look down a little more
	g_pendUp = g_pendBack = 0.0f;
}

void port_debugui_freecam_pos(float *out) { out[0] = g_pos[0]; out[1] = g_pos[1]; out[2] = g_pos[2]; }
