// RenderWare Audio core handle.

// Only the first dword is game state: the RWA core pointer, written at open and only tested for non-zero (the port's shim
// returns its own token). The ~8 KB behind it is RWA's own static data, which the port does not contain.
struct RwaCore;
// GLOBAL: 0x60b8c0 g_audioCore
// ALONE: g_audioCore game code reads only this dword (r440/audio.cpp); the RWA tables behind it are read only by RWA code, not in the port
RwaCore * g_audioCore = (RwaCore *)1;
