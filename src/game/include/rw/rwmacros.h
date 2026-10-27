// Single-precision trigonometry used by the effects code.
#ifndef ZOOSTERS_RW_RWMACROS_H
#define ZOOSTERS_RW_RWMACROS_H
#include "rw/rwcore.h"
#include <stdlib.h>
#include <math.h>

// Single precision on purpose: the recorded runs used the C library's float functions.
inline RwReal RwSin(RwReal x) { return sinf(x); }
inline RwReal RwCos(RwReal x) { return cosf(x); }
inline RwReal RwFloor(RwReal x) { return floorf(x); }

#endif
