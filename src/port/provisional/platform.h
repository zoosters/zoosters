// The port's platform seam: what the input layer (game_stubs.cpp, mouse.cpp, dinput_port.cpp), the resolution policy (port/fixes/) and the harness (port_run.cpp) ask of the window system.
// One backend per build (--window), like re3's skel/win/win.cpp and skel/glfw/glfw.cpp:
//   port/provisional/window_w32.cpp   Win32 (the game's own window code path: GameWndProc gets real window messages)
//   port/provisional/window_sdl.cpp   SDL3 (one SDL window; SDL events are translated into the same GameWndProc messages)
//   port/provisional/window.cpp       headless (no display, never focused)
// The backends also define CreateGameWindow (0x519f00); the SDL one replaces the game's PumpMessages (0x51a100) as well.
#ifndef ZOOSTERS_PORT_PLATFORM_H
#define ZOOSTERS_PORT_PLATFORM_H
#ifdef __cplusplus
extern "C" {
#endif

// Display queries for the resolution policy (physical pixels). Each returns 1 on success, 0 when unknown.
void port_platform_dpi_aware(void);                                         // before any window exists
int port_platform_desktop_size(int *w, int *h);                             // the primary display's current mode
int port_platform_work_area(int *w, int *h, int *frameW, int *frameH);      // usable area + what the window frame adds to a client size
int port_platform_screen_size(int *w, int *h);                              // the primary monitor's full rectangle

// Display changes at run time (qol options). Modes are unique "w,h" pairs in physical pixels (SDL backend only; 0 modes elsewhere).
// apply: mode 0 windowed (client w x h), 1 borderless full screen (desktop size), 2 exclusive full screen (w x h); returns 1 when
// the window is in that state, else the previous state is kept as far as the system allows.
int port_platform_display_modes(int *wh, int max);
int port_platform_apply_display(int mode, int w, int h);

// Input. Real input is read only while the game window has the keyboard focus.
int port_platform_focused(void);
void port_platform_keyboard(unsigned char dik[256]);                        // keys held now, DirectInput codes (0x80 = down)
int port_platform_pointer(int *x, int *y, int *leftDown);                   // 1 = pointer inside the client area (client pixels)
int port_platform_client_size(int *w, int *h);                              // 0 when there is no window (then 800x600)

// Synthetic events for scripted runs: a pointer event the game's window procedure sees in the next PumpMessages (msg = 0x200
// WM_MOUSEMOVE, 0x201 WM_LBUTTONDOWN, 0x202 WM_LBUTTONUP), and a close request (WM_CLOSE: the frame limit / watchdog).
void port_platform_post_pointer(unsigned msg, int x, int y, int leftDown);
void port_platform_request_close(void);

// User folder (userdir.cpp; per-platform roots there). Paths use the native separator. Each returns 1 on success, 0 when no
// folder is known or the text does not fit.
#ifdef _WIN32
#define PORT_PATH_SEP '\\'
#else
#define PORT_PATH_SEP '/'
#endif
#define PORT_USER_SUBDIR "Activision\\Madagascar"                         // the game's own folder below the root (its save sections)
int port_user_root(char *buf, int size);                                     // PORT_USER_DIR, else the platform's per-user data folder
int port_user_file(const char *name, char *buf, int size);                  // <root>/Activision/Madagascar/<name>
int port_make_dirs(const char *path);                                       // creates every directory of `path` (the last part too)

// Harness (port/loader/port_run.cpp): frame limit (n <= 0: play mode, no limit) and the number of presented frames.
void port_win_set_frames(int n);
int port_win_pumps(void);

#ifdef __cplusplus
}
#endif
#endif
