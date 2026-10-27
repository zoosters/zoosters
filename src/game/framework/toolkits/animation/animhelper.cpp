#include "framework/toolkits/animation/animhelper.h"
#include "rw/rpplugins.h"

namespace RWS {

// Returns the frame where a hierarchy was found, else 0 to go on with the next sibling.
// 0x449270
__declspec(noinline) RwFrame *FindHAnimHierarchyCallback(RwFrame *pFrame, void *pData)
{
	RpHAnimHierarchy *&pHierarchy = *(RpHAnimHierarchy **)pData;
	pHierarchy = RpHAnimFrameGetHierarchy(pFrame);
	if (0 == pHierarchy)
		RwFrameForAllChildren(pFrame, FindHAnimHierarchyCallback, pData);
	return 0 == pHierarchy ? pFrame : 0;
}

// 0x4492b0
RpAtomic *AttachHAnimHierarchyCallback(RpAtomic *pAtomic, void *pData)
{
	RpSkin *pSkin = RpSkinGeometryGetSkin(RpAtomicGetGeometry(pAtomic));
	if (pSkin)
		RpSkinAtomicSetHAnimHierarchy(pAtomic, (RpHAnimHierarchy *)pData);
	return pAtomic;
}

} // namespace RWS
