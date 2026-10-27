// Debug overlay (Dear ImGui), compiled only into --debugui builds (ZOOSTERS_DEBUGUI). Without the define this header only
// provides PORT_DBG_HIDDEN == 0. Files: debugui_rw.cpp (ImGui context, librw Im2D renderer, window input), debugui_panels.cpp
// (the panels, reading game state through the src/game headers).
#ifndef ZOOSTERS_PORT_DEBUGUI_H
#define ZOOSTERS_PORT_DEBUGUI_H

// Draw categories the Render panel can hide (counted the same either way: the counters say what the game submitted).
enum PortDbgPass {
	PORT_DBG_WORLD,      // world sectors (RpWorld render: the level geometry)
	PORT_DBG_ATOMIC,     // atomics (characters, props: the default atomic render callback)
	PORT_DBG_IM2D,       // Im2D / Rt2d (HUD, menus, fades)
	PORT_DBG_IM3D,       // Im3D (particles, lines, shadows drawn as immediate geometry)
	PORT_DBG_D3D,        // the game's own RwD3D8DrawPrimitive calls
	PORT_DBG_PASSES
};

#ifdef ZOOSTERS_DEBUGUI
extern "C" int port_dbg_hide[PORT_DBG_PASSES];
#define PORT_DBG_HIDDEN(pass) (port_dbg_hide[pass] != 0)

extern "C" void port_debugui_present(void *camera);   // RwCameraShowRaster, before the counters reset and the present (an rw::Camera)
extern "C" int port_debugui_wndproc(void *hwnd, unsigned msg, unsigned long long wp, long long lp, long long *result);   // 1 = eaten
extern "C" int port_debugui_blocks_keyboard(void);    // the overlay is typing: the game's keyboard reads nothing
extern "C" int port_debugui_blocks_mouse(void);       // the overlay is open: the game's relative mouse reads nothing

// debugui_panels.cpp: called between ImGui::NewFrame and ImGui::Render; `open` = interactive overlay (F1), else the pinned HUD only
extern "C" void port_debugui_panels(int open);
void port_debugui_panels_settings(void);
// debugui_physics.cpp: the physics timeline and the collision overlay (both called from port_debugui_panels)
void port_debugui_physics_tick(void);                        // every frame: the physics timeline's sample
void port_debugui_timeline_panel(int open);
void port_debugui_collision_draw(void);                      // queues this frame's collision lines (world draws run whether or not the overlay is open)
void port_debugui_collision_panel(int open);
void port_debugui_triggers_draw(int panelVisible);           // debugui_triggers.cpp: queues the range spheres; collects the ranges the panel lists too
void port_debugui_triggers_panel(int open);                  // script range checks
// debugui_practice.cpp: pause / frame advance / slow motion (the game's logic step is held), free camera, saved positions
void port_debugui_practice_panel(int open);
void port_debugui_practice_tick(int open);                  // every frame: hotkeys, free camera movement, status banner
extern "C" void port_debugui_camera_begin(void *camera);    // rwcore.cpp camBegin: swap in the free camera (game camera only)
void port_debugui_camera_restore(void);                     // after the overlay: the game camera's own frame back
extern "C" void port_debugui_camera_end(void *camera);      // rwcore.cpp camEnd: the game camera's own frame back after each update
bool port_debugui_view(float *pos, float *fwd, float *right, float *up, float *vw, float *nearFar);   // the rendered view (picking, gizmo)
void port_debugui_freecam_focus(const float *target, float dist);
// debugui_studio.cpp: Studio mode (F9): select, move and edit the level's actors at run time
void port_debugui_studio_tick(int open);
int port_debugui_studio_on(void);
void port_debugui_set_open(int open);
// debugui_vars.cpp: script variables (watch / edit / freeze) and the sandbox presets
void port_debugui_vars_panel(int open);
void port_debugui_sandbox_panel(int open);
void port_debugui_vars_apply(void);                        // just before every logic step: writes the frozen values
void port_debugui_camtour_tick(void);                      // debugui_camtour.cpp: PORT_CAMLOG cut log, PORT_CAMTOUR camera tour
const char *port_debugui_object_name(const void *object);   // the name the object was registered under (0 if none)
// debugui_panels.cpp: the live GameActors (fed by the GameActor constructor / destructor in ZOOSTERS_DEBUGUI builds). The overlay
// reads actors only through these: g_cameraFollowActor keeps pointing at the last level's freed player through level switches and
// menus until the camera follows someone new (ASan: use-after-free in port_debugui_physics_tick on a loading screen).
namespace RWS { namespace TFB { class GameActor; } }
void port_debugui_actor_life(const void *actor, int alive);
int port_debugui_actor_alive(const void *actor);
int port_debugui_named_actors(RWS::TFB::GameActor **actors, const char **names, int max);   // debugui_triggers.cpp: live actors with a registered name
RWS::TFB::GameActor *port_debugui_player(void);              // g_cameraFollowActor while it is alive, else 0
int port_debugui_freecam_keys(void);                        // the free camera takes the keyboard
// 3D lines in world space for this frame (rgba = 0xRRGGBBAA), drawn before the 2D overlay; x-ray = ignore the depth buffer
extern "C" void port_debugui_line(const float *a, const float *b, unsigned rgba);
extern "C" int port_debugui_xray;
#else
#define PORT_DBG_HIDDEN(pass) 0
#endif

#endif
