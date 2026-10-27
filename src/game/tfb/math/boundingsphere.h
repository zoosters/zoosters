#ifndef ZOOSTERS_TFB_MATH_BOUNDINGSPHERE_H
#define ZOOSTERS_TFB_MATH_BOUNDINGSPHERE_H

#include "rw/rpcollis.h"

// Grows `sphere` to also hold the atomic's world bounding sphere (radius 0: takes it over). RpClumpForAllAtomics callback.
RpAtomic *AtomicGrowSphereCB(RpAtomic *atomic, RwSphere *sphere);

#endif
