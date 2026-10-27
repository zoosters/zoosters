// Clump / atomic flag helpers: visibility fade, collision flag, plugin flag bits applied to every atomic of a clump.
#ifndef ZOOSTERS_TFB_RENDER_ATOMICFLAGS_H
#define ZOOSTERS_TFB_RENDER_ATOMICFLAGS_H

#include "rw/rpworld.h"

RpClump *ClumpSetIsVisible(RpClump *clump, bool visible, float fadeFrom, float fadeTo);  // (cdecl)
RpAtomic *ClumpSetCanCollideCB(RpAtomic *atomic, void *canCollide);
RpClump *ClumpSetAtomicFlags(RpClump *clump, int flags);
RpClump *ClumpClearAtomicFlags(RpClump *clump, int flags);
void *ClumpFindFirstSkin(RpClump *clump);
void ClumpSetAtomicColour(RpClump *clump, RwRGBA *colour);                                // 0x44ed00
struct RwStream;
RpAtomic *__stdcall ReadAtomic(RwStream *stream);  // stream readers of the resource handlers
RpClump *__stdcall ReadClump(RwStream *stream);
void *__stdcall ReadPITexDictionary(RwStream *stream);

#endif
