// Game address -> port address. The build emits into the generated stub object the sorted table __port_gaddr_map = { game address,
// size, port address } for every data object with a known game address (src/data definitions, tree-defined data, stand-in / gap /
// synthetic blocks copied from Game.exe), and __port_gaddr_count. port_gaddr resolves an address anywhere in a mapped range, else 0.
// PORT_GADDR_SELFTEST=1 checks it against the symbols' own addresses.
#include <stdio.h>
#include <stdlib.h>

struct PortGaddrEntry { unsigned game, size; unsigned char *port; };
extern "C" FILE *port_log;
// The normal build links data by name, so its table is a file, <exe name>.gaddr.tsv, written after every successful link from the link
// map and the GLOBAL annotations: `game size exe [source symbol]` per line, `#` comments, `# imagebase 0x..` the link base. Loaded on the
// first lookup from PORT_GADDR_FILE (port_run.cpp sets it to the .gaddr.tsv next to the exe); without it the map is empty (PORT_WATCH /
// gtrace / the `set` input line find nothing). The load offset (relocation, ASLR, PIE) comes from one known global, g_nextLevel, so no
// loader API is needed.
#include "tfb/level/levelmanager.h"      // g_nextLevel 0x62a2f8, the relocation anchor
static PortGaddrEntry *g_map; static unsigned g_count; static int g_loaded;
static int cmp_entry(const void *a, const void *b)
{
	unsigned x = ((const PortGaddrEntry *)a)->game, y = ((const PortGaddrEntry *)b)->game;
	return x < y ? -1 : x > y;
}
static void load_map(void)
{
	g_loaded = 1;
	const char *path = getenv("PORT_GADDR_FILE");
	if (!path || !*path) { if (port_log) fprintf(port_log, "gaddr: PORT_GADDR_FILE not set: game addresses do not resolve\n"); return; }
	FILE *f = fopen(path, "r");
	if (!f) { if (port_log) fprintf(port_log, "gaddr: no map file %s: game addresses do not resolve\n", path); return; }
	unsigned cap = 2048;                                     // x64 images sit above 4 GB
	g_map = (PortGaddrEntry *)malloc(cap * sizeof(PortGaddrEntry));
	char line[1024];
	while (fgets(line, sizeof line, f)) {
		if (line[0] == '#') continue;
		unsigned g, sz; unsigned long long e;
		if (sscanf(line, "%x %x %llx", &g, &sz, &e) != 3) continue;
		if (g_count == cap) { cap *= 2; g_map = (PortGaddrEntry *)realloc(g_map, cap * sizeof(PortGaddrEntry)); }
		g_map[g_count].game = g; g_map[g_count].size = sz; g_map[g_count].port = (unsigned char *)(size_t)e;
		g_count++;
	}
	fclose(f);
	qsort(g_map, g_count, sizeof(PortGaddrEntry), cmp_entry);
	unsigned char *anchor = 0;                               // g_nextLevel's link-time address (its row covers 0x62a2f8)
	for (unsigned i = 0; i < g_count; i++)
		if (g_map[i].game <= 0x62a2f8u && 0x62a2f8u - g_map[i].game < g_map[i].size) anchor = g_map[i].port + (0x62a2f8u - g_map[i].game);
	if (!anchor) {
		if (port_log) fprintf(port_log, "gaddr: %s has no row for g_nextLevel (0x62a2f8): map not used\n", path);
		g_count = 0;
		return;
	}
	size_t slide = (size_t)(unsigned char *)&g_nextLevel - (size_t)anchor;   // relocate the table
	if (slide)
		for (unsigned i = 0; i < g_count; i++) g_map[i].port += slide;
	if (port_log) fprintf(port_log, "gaddr: %u rows from %s (load offset %p)\n", g_count, path, (void *)slide);
}
#define GMAP g_map
#define GCOUNT g_count
#define GLOAD() do { if (!g_loaded) load_map(); } while (0)


extern "C" void *port_gaddr(unsigned game_addr)
{
	GLOAD();
	int lo = 0, hi = (int)GCOUNT - 1, best = -1;
	while (lo <= hi) {                                        // last entry with game <= game_addr
		int mid = (lo + hi) / 2;
		if (GMAP[mid].game <= game_addr) { best = mid; lo = mid + 1; } else hi = mid - 1;
	}
	if (best < 0) return 0;
	const PortGaddrEntry &e = GMAP[best];
	if (game_addr - e.game >= e.size) return 0;
	return e.port + (game_addr - e.game);
}

// the game's symbols for a few known globals, bound by name and type
#include "modules/tfb/gamecamera.h"      // g_cameraObject 0x6182c8
namespace RWS { namespace TFB { class GameActor; } }
extern RWS::TFB::GameActor *g_cameraFollowActor;   // GLOBAL: 0x61818c g_cameraFollowActor (declared in the game's .cpp files only)

extern "C" int port_gaddr_selftest(void)
{
	struct T { unsigned game; const char *name; void *expect; } t[] = {
		{ 0x606a40, "g_LevelTable", (void *)&g_LevelTable[0] },
		{ 0x606a40 + 20, "g_LevelTable row 1", (char *)&g_LevelTable[0] + 20 },
		{ 0x62a2f8, "g_nextLevel", &g_nextLevel },
		{ 0x6182c8, "g_cameraObject", &g_cameraObject },
		{ 0x61818c, "g_cameraFollowActor", &g_cameraFollowActor },
	};
	int bad = 0;
	FILE *f = port_log ? port_log : stdout;
	GLOAD();
	fprintf(f, "gaddr: selftest: %u map entries\n", GCOUNT);
	for (unsigned i = 0; i < sizeof(t) / sizeof(t[0]); i++) {
		void *got = port_gaddr(t[i].game);
		int ok = got == t[i].expect;
		bad += !ok;
		fprintf(f, "gaddr: selftest: 0x%06x %-22s -> %p (symbol %p) %s\n", t[i].game, t[i].name, got, t[i].expect, ok ? "ok" : "MISMATCH");
	}
	void *code = port_gaddr(0x401000);                        // code is not data: not mapped
	bad += code != 0;
	fprintf(f, "gaddr: selftest: 0x401000 (code) -> %p %s\n", code, code ? "MISMATCH" : "ok (unmapped)");
	fprintf(f, "gaddr: selftest: %s\n", bad ? "FAILED" : "passed");
	fflush(f);
	if (f != stdout) { printf("gaddr: selftest %s (%d mismatches)\n", bad ? "FAILED" : "passed", bad); fflush(stdout); }
	return bad;
}
