#include "framework/toolkits/animation/skinsafe.h"
#include "rw/rpplugins.h"

namespace RWS {

namespace {

// Nodes are in depth-first order: a node without POP is followed by its first child, and every child but the last carries PUSH.
// Returns the index after the subtree.
RwInt32 PoseChildFrames(RpHAnimHierarchy &hierarchy, const RwMatrix *skinToBone, RwInt32 parent)
{
	RwInt32 node = parent + 1;
	if (hierarchy.pNodeInfo[parent].flags & rpHANIMPOPPARENTMATRIX)
		return node;

	while (node < hierarchy.numNodes) {
		const RwInt32 child = node;
		RwFrame *pFrame = hierarchy.pNodeInfo[child].pFrame;
		RwMatrixInvert(RwFrameGetMatrix(pFrame), &skinToBone[child]);
		RwFrameTransform(pFrame, &skinToBone[parent], rwCOMBINEPOSTCONCAT);
		node = PoseChildFrames(hierarchy, skinToBone, child);
		if (!(hierarchy.pNodeInfo[child].flags & rpHANIMPUSHPARENTMATRIX))
			break;
	}
	return node;
}

} // namespace

// 0x44a960
void ApplyDefaultPoseToHierarchy(RpHAnimHierarchy &hierarchy, const RpSkin &skin)
{
	RpSkin *pSkin = const_cast<RpSkin *>(&skin);
	const RwMatrix *skinToBone = RpSkinGetSkinToBoneMatrices(pSkin);

	if (hierarchy.flags & rpHANIMHIERARCHYNOMATRICES) {
		// the hierarchy drives frames: write each bone's bind pose relative to its parent
		RpHAnimHierarchyAttach(&hierarchy);
		RwFrame *pRoot = hierarchy.pNodeInfo[0].pFrame;
		RwMatrixInvert(RwFrameGetMatrix(pRoot), &skinToBone[0]);
		RwFrameUpdateObjects(pRoot);
		PoseChildFrames(hierarchy, skinToBone, 0);
		return;
	}

	// the inverse of a skin-to-bone matrix is the bone's bind pose in skin space
	hierarchy.flags |= rpHANIMHIERARCHYLOCALSPACEMATRICES;
	const RwUInt32 numBones = RpSkinGetNumBones(pSkin);
	for (RwUInt32 i = 0; i < numBones; ++i)
		RwMatrixInvert(&hierarchy.pMatrixArray[i], &skinToBone[i]);
}

// 0x44a870
bool AnimationIsCompatibleWithHierarchy(const RpHAnimAnimation &anim, const RpHAnimHierarchy &hierarchy)
{
	if (!anim.interpInfo)
		return false;

	const RwInt32 frameSize = anim.interpInfo->animKeyFrameSize;
	const char *pFrame = static_cast<const char *>(anim.pFrames);
	RwInt32 numAtZero = 0;
	while (numAtZero < anim.numFrames && reinterpret_cast<const RtAnimKeyFrameHeader *>(pFrame)->time == 0.0f) {
		++numAtZero;
		pFrame += frameSize;
	}
	return numAtZero == hierarchy.numNodes;
}

} // namespace RWS
