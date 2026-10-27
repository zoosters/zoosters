#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_ANIMATION_ANIMHELPER_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_ANIMATION_ANIMHELPER_H

#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rphanim.h"

namespace RWS {

// pData is an RpHAnimHierarchy ** receiving the first hierarchy found.
RwFrame *FindHAnimHierarchyCallback(RwFrame *pFrame, void *pData);
// pData is the hierarchy.
RpAtomic *AttachHAnimHierarchyCallback(RpAtomic *pAtomic, void *pData);

// The root itself first. Null if none.
inline RpHAnimHierarchy *FindHAnimHierarchy(RwFrame &root)
{
	RpHAnimHierarchy *pHierarchy = 0;
	FindHAnimHierarchyCallback(&root, &pHierarchy);
	return pHierarchy;
}

// Null if none, also without a frame.
inline RpHAnimHierarchy *FindHAnimHierarchy(RpClump &clump)
{
	RwFrame *pFrame = RpClumpGetFrame(&clump);
	return pFrame ? FindHAnimHierarchy(*pFrame) : 0;
}

inline void AttachHAnimHierarchy(RpClump &clump, RpHAnimHierarchy &hierarchy)
{
	RpClumpForAllAtomics(&clump, AttachHAnimHierarchyCallback, &hierarchy);
}

} // namespace RWS

#endif
