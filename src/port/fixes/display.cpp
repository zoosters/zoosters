// Display mode fix (tiers fixes and qol: ZOOSTERS_FIX_BUGS). Stock WinMain calls ChangeDisplaySettingsA with flags 0 when the stored
// resolution (Settings\Display\Resolution) exceeds the desktop's, restored only at a normal exit: a crash leaves the desktop in
// another mode, or "Display change failed" shows when the mode does not exist.
// Tier vanilla keeps that call; fixes and qol pass the size to port_display_fit instead (winmain.cpp). Windowed (default): the largest
// size of the same aspect whose window (client + frame) fits the monitor's work area; full screen (PORT_FULLSCREEN=1): the largest
// that fits the monitor. A size that already fits is untouched. Queries are the port_platform_* functions; failure means no change.
#include <stdlib.h>
#include <stdio.h>

extern "C" FILE *port_log;
extern "C" int port_platform_work_area(int *w, int *h, int *frameW, int *frameH);
extern "C" int port_platform_screen_size(int *w, int *h);
#ifdef ZOOSTERS_OPTIONS
extern "C" int port_window_mode(void);   // port/options/display.cpp
#endif

extern "C" void port_display_fit(int *w, int *h)
{
	if (*w <= 0 || *h <= 0) return;
#ifdef ZOOSTERS_OPTIONS
	bool full = port_window_mode() != 0;
#else
	const char *fs = getenv("PORT_FULLSCREEN");
	bool full = fs && fs[0] == '1';
#endif
	int aw = 0, ah = 0, ww = 0, wh = 0, fx = 0, fy = 0;
	if (full) {
		if (!port_platform_screen_size(&aw, &ah) || aw <= 0 || ah <= 0) return;
	} else {
		if (!port_platform_work_area(&ww, &wh, &fx, &fy)) return;
		aw = ww - fx; ah = wh - fy;   // the largest client the work area holds
		if (aw < 2) aw = 2;
		if (ah < 2) ah = 2;
	}
	if (*w <= aw && *h <= ah) return;
	int cw, ch;
	if ((long long)aw * *h <= (long long)ah * *w) { cw = aw; ch = (int)((long long)aw * *h / *w); }   // width limits
	else { ch = ah; cw = (int)((long long)ah * *w / *h); }                                          // height limits
	cw &= ~1; ch &= ~1;
	if (cw < 2) cw = 2;
	if (ch < 2) ch = 2;
	if (port_log) {
		if (full) fprintf(port_log, "display: resolution %dx%d fitted to the screen %dx%d (stored %dx%d)\n", cw, ch, aw, ah, *w, *h);
		else fprintf(port_log, "display: resolution %dx%d fitted to the work area %dx%d minus frame +%d+%d (stored %dx%d)\n", cw, ch, ww, wh, fx, fy, *w, *h);
	}
	*w = cw; *h = ch;
}
