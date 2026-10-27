// Bink without videos: BinkOpen fails and the game skips the movie.
#include <stdio.h>
extern "C" FILE *port_log;
#define WLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); } } while (0)
// The game declares Bink without dllimport (bink.h), so these are plain functions by name.
#include "tfb/level/bink.h"
extern "C" {
BINK *__stdcall BinkOpen(const char *name, unsigned flags) { WLOG("win32: BinkOpen %s (%x) -> skipped\n", name ? name : "", flags); return 0; }
void __stdcall BinkClose(BINK *) {}
int __stdcall BinkWait(BINK *) { return 0; }
void __stdcall BinkDoFrame(BINK *) {}
void __stdcall BinkNextFrame(BINK *) {}
int __stdcall BinkCopyToBuffer(BINK *, void *, int, unsigned, unsigned, unsigned, unsigned) { return 0; }
void *__stdcall BinkOpenDirectSound(void *) { return 0; }
int __stdcall BinkSetSoundSystem(void *, void *) { return 0; }
}
