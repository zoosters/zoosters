// Headless CreateGameWindow (0x519f00): the window of port/shim/win32shim.cpp instead of the game's RegisterClassExA /
// CreateWindowExA path.
#include <stdio.h>

extern "C" FILE *port_log;
extern "C" void *port_win_create(int w, int h, void *wndProc);
extern void *g_mainWnd;   // GLOBAL: 0x62f138 g_mainWnd

int CreateGameWindow(const char *cls, const char *title, int x, int y, int w, int h, void *wndProc, void *hInst, int icon, bool fullscreen)
{
	(void)x; (void)y; (void)hInst; (void)icon;
	if (port_log) fprintf(port_log, "window: CreateGameWindow(class=%s title=%s %dx%d fullscreen=%d)\n", cls ? cls : "", title ? title : "", w, h, fullscreen);
	g_mainWnd = port_win_create(w, h, wndProc);
	return 1;
}

// Resolution policy queries (widescreen.cpp): no display, so the policy falls back to the game's 800x600.
extern "C" void port_platform_dpi_aware(void) {}
extern "C" int port_platform_display_scale(float *scale) { *scale = 1.0f; return 0; }
extern "C" int port_platform_desktop_size(int *w, int *h) { (void)w; (void)h; return 0; }
extern "C" int port_platform_work_area(int *w, int *h, int *frameW, int *frameH) { (void)w; (void)h; (void)frameW; (void)frameH; return 0; }
extern "C" int port_platform_screen_size(int *w, int *h) { (void)w; (void)h; return 0; }
extern "C" int port_platform_display_modes(int *wh, int max) { (void)wh; (void)max; return 0; }
extern "C" int port_platform_apply_display(int mode, int w, int h) { (void)mode; (void)w; (void)h; return 0; }

// Input queries (platform.h): never focused, so no real input; scripted pointer events go nowhere.
extern "C" int port_platform_focused(void) { return 0; }
extern "C" void port_platform_keyboard(unsigned char *dik) { (void)dik; }
extern "C" int port_platform_pointer(int *x, int *y, int *leftDown) { (void)x; (void)y; *leftDown = 0; return 0; }
extern "C" int port_platform_client_size(int *w, int *h) { *w = 800; *h = 600; return 0; }
extern "C" void port_platform_post_pointer(unsigned msg, int x, int y, int leftDown) { (void)msg; (void)x; (void)y; (void)leftDown; }
extern "C" void port_platform_request_close(void) {}
