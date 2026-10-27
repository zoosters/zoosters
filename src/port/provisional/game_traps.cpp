// Game functions that src/game calls but nothing defines yet, as traps by name. A call exits with 3 ("unimplemented external
// called: <name>"), or with harness option "k" logs "MISSING <name>" once and returns. A definition in src/game replaces its trap
// (delete the entry; a duplicate symbol makes the link say so).
extern "C" void port_trap_named(const char *name);       // port/runtime.cpp

#define PORT_TRAP_VOID(decl, label) decl { port_trap_named(label); }

// RenderWare skin plugin (rpskin: Open/Close and the atomic / geometry plugin callbacks, tfb/render/rpskin.cpp calls them by these names)
PORT_TRAP_VOID(void __cdecl SkinOpen(void), "SkinOpen")
PORT_TRAP_VOID(void __cdecl SkinClose(void), "SkinClose")
PORT_TRAP_VOID(void __cdecl SkinAtomicConstructor(void), "SkinAtomicConstructor")
PORT_TRAP_VOID(void __cdecl SkinAtomicCopy(void), "SkinAtomicCopy")
PORT_TRAP_VOID(void __cdecl SkinAtomicDestructor(void), "SkinAtomicDestructor")
PORT_TRAP_VOID(void __cdecl SkinAtomicGetSize(void), "SkinAtomicGetSize")
PORT_TRAP_VOID(void __cdecl SkinAtomicRead(void), "SkinAtomicRead")
PORT_TRAP_VOID(void __cdecl SkinAtomicWrite(void), "SkinAtomicWrite")
PORT_TRAP_VOID(void __cdecl SkinGeometryConstructor(void), "SkinGeometryConstructor")
PORT_TRAP_VOID(void __cdecl SkinGeometryCopy(void), "SkinGeometryCopy")
PORT_TRAP_VOID(void __cdecl SkinGeometryDestructor(void), "SkinGeometryDestructor")
PORT_TRAP_VOID(void __cdecl SkinGeometryRead(void), "SkinGeometryRead")
PORT_TRAP_VOID(void __cdecl SkinGeometryWrite(void), "SkinGeometryWrite")
PORT_TRAP_VOID(void __cdecl rpskin_static_51e210(void), "rpskin_static_51e210")
PORT_TRAP_VOID(void __cdecl rpskin_static_51e310(void), "rpskin_static_51e310")
PORT_TRAP_VOID(void __cdecl rpskin_static_51e340(void), "rpskin_static_51e340")

// RpPVS sector callback (0x4b0e00)
struct RpWorldSector;
int __cdecl rppvs_static_4b0e00(RpWorldSector *) { port_trap_named("rppvs_static_4b0e00"); return 0; }

// ShadowPass::RenderActorShadow (0x4437f0): the member pointer DAT_005d4dc8 of src/data/r440/bounds_shadow.cpp points at it
struct SLink;
struct ShadowPass { void RenderActorShadow(SLink *link); };
void ShadowPass::RenderActorShadow(SLink *) { port_trap_named("ShadowPass::RenderActorShadow"); }

// fileio (framework/startup/win32/fileio.cpp): the pre-NT5 overlapped-I/O emulation's CancelIo replacement and worker thread
// (never used on a modern Windows)
int __stdcall FUN_0051cb20(void *) { port_trap_named("FUN_0051cb20"); return 0; }
unsigned long __stdcall FUN_0051cb70(void *) { port_trap_named("FUN_0051cb70"); return 0; }
