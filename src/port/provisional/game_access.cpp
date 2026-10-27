// game_access.h against the game's real headers: a type that drifts from the game's declaration fails to compile here, not at link time.
#include "game_access.h"
#include "rw/rpworld.h"
#include "tfb/level/level.h"
#include "tfb/render/occluder.h"
#include "modules/tfb/gamecamera.h"
#include "modules/tfb/tfbshadowcamera.h"
#include "modules/tfb/shadowrender.h"
#include "script/symbolregistry.h"

RpWorld *&PortGame_OpaqueWorld() { return RWS::CLevel::sm_pOpaqueWorld; }
RpWorld *&PortGame_TranslucentWorld() { return RWS::CLevel::sm_pTranslucentWorld; }
RpWorld *&PortGame_ActiveWorld() { return RWS::CLevel::sm_pActiveWorld; }
RpWorld *&PortGame_SkyWorld() { return RWS::CLevel::sm_pSkyWorld; }
RpWorld *&PortGame_FogWorld() { return RWS::CLevel::sm_pFogWorld; }

RwCamera *&PortGame_CamDst() { return g_gameCamDst; }
RWS::TFB::TFBShadowCamera *&PortGame_ShadowCamera() { return g_shadowCamera; }
ShadowRender *&PortGame_ShadowRender() { return g_shadowRender; }
OccluderNode *&PortGame_PlaneList() { return g_planeList413; }
void *PortGame_SymRegistry() { return g_symRegistryY; }

#include "framework/core/classmap.h"
void *PortGame_ClassMap() { return &RWS::CClassFactory::GetClassMap(); }

// PORT_SYMDUMP=<file>: the script symbol registry (index, name hash, name, type) after the static initialisers (port_run.cpp), to
// compare registration order between builds.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "script/script.h"
extern "C" void port_dump_symbols(const char *path)
{
	FILE *f = fopen(path, "w");
	if (!f) return;
	if (g_symRegistryY)
		for (unsigned i = 0; i < g_symRegistryY->size(); i++) {
			const SymE8 &e = (*g_symRegistryY)[i];
			Script::ScriptReference *o = (Script::ScriptReference *)(e.obj & ~(uintptr_t)1);
			const bool named = i < 103;                       // the start-up symbols are complete objects; later slots may be op prototypes / pools
			Script::ScriptReference *t = o && named ? o->GetType() : 0;
			fprintf(f, "%u\t%08x\t%u\t%s\t%s\n", i, e.key, (unsigned)(e.obj & 1), o && named ? o->GetName() : "-", t ? t->GetName() : "-");
			fflush(f);
		}
	fclose(f);
}
// PORT_SYMDUMP_AT=<file>,<presented frame>: the same dump once that frame is presented (after a level's scripts registered their symbols)
extern "C" void (*port_present_hook)(void);   // port/shim/rwcore.cpp
extern "C" int port_frames_presented(void);
static void (*g_symPrevHook)(void);
static char g_symAtPath[260];
static int g_symAtFrame = -1;
static void symDumpHook(void)
{
	if (g_symPrevHook) g_symPrevHook();
	if (g_symAtFrame >= 0 && port_frames_presented() == g_symAtFrame) { port_dump_symbols(g_symAtPath); g_symAtFrame = -1; }
}
extern "C" void port_symdump_install(void)
{
	const char *e = getenv("PORT_SYMDUMP_AT");
	const char *c = e ? strrchr(e, ',') : 0;
	if (!c || c - e >= (int)sizeof g_symAtPath) return;
	memcpy(g_symAtPath, e, c - e);
	g_symAtPath[c - e] = 0;
	g_symAtFrame = atoi(c + 1);
	g_symPrevHook = port_present_hook;
	port_present_hook = symDumpHook;
}
