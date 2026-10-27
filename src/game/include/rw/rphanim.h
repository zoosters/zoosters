// RpHAnim: the skeleton a frame hierarchy carries, its creation and update. Partial layouts: never allocate or copy.
#ifndef ZOOSTERS_RW_RPHANIM_H
#define ZOOSTERS_RW_RPHANIM_H

#include "rw/rwcore.h"
#include "rw/rtanim.h"
extern "C" {

// RW names an animation of a hierarchy RpHAnimAnimation; it is the RtAnim animation.
typedef RtAnimAnimation RpHAnimAnimation;

// Per node: the matrix push/pop flags of the hierarchy topology and the frame the node is attached to.
enum { rpHANIMPOPPARENTMATRIX = 0x01, rpHANIMPUSHPARENTMATRIX = 0x02 };
struct RpHAnimNodeInfo {
	RwInt32 nodeID;
	RwInt32 nodeIndex;
	RwInt32 flags;
	RwFrame *pFrame;
};
ZOOSTERS_ASSERT_SIZE(RpHAnimNodeInfo, 0x10);
ZOOSTERS_ASSERT_OFFSET(RpHAnimNodeInfo, flags, 0x08);
ZOOSTERS_ASSERT_OFFSET(RpHAnimNodeInfo, pFrame, 0x0c);

// The interpolated key frame of one node (RW: RpHAnimInterpFrame, rtANIMGETINTERPFRAME): the two key frames it lies between, the
// rotation as a quaternion and the translation. One per node after an RtAnimInterpolator, currentInterpKeyFrameSize bytes apart
// (0x24 for the plain key frame scheme).
struct RtQuat {
	RwV3d imag;
	RwReal real;
};
struct RpHAnimInterpFrame {
	void *keyFrame1;
	void *keyFrame2;
	RtQuat orientation;
	RwV3d translation;
};
ZOOSTERS_ASSERT_SIZE(RpHAnimInterpFrame, 0x24);

// A skeleton: node topology, node matrices and the interpolator that plays an animation on it. Partial.
struct RpHAnimHierarchy {
	RwUInt32 flags;  // RpHAnimHierarchyFlag bits (RW declares it RwInt32)
	RwInt32 numNodes;
	RwMatrix *pMatrixArray;
	void *pMatrixArrayUnaligned;
	RpHAnimNodeInfo *pNodeInfo;
	RwFrame *parentFrame;  // root frame
	void *parentHierarchy;
	RwInt32 rootParentOffset;
	RtAnimInterpolator *currentAnim;
};
ZOOSTERS_ASSERT_OFFSET(RpHAnimHierarchy, flags, 0x00);
ZOOSTERS_ASSERT_OFFSET(RpHAnimHierarchy, numNodes, 0x04);
ZOOSTERS_ASSERT_OFFSET(RpHAnimHierarchy, pMatrixArray, 0x08);
ZOOSTERS_ASSERT_OFFSET(RpHAnimHierarchy, pNodeInfo, 0x10);
ZOOSTERS_ASSERT_OFFSET(RpHAnimHierarchy, currentAnim, 0x20);

// Flags of a hierarchy (combinable).
enum RpHAnimHierarchyFlag {
	rpHANIMHIERARCHYSUBHIERARCHY = 0x01,
	rpHANIMHIERARCHYNOMATRICES = 0x02,
	rpHANIMHIERARCHYUPDATEMODELLINGMATRICES = 0x1000,
	rpHANIMHIERARCHYUPDATELTMS = 0x2000,
	rpHANIMHIERARCHYLOCALSPACEMATRICES = 0x4000
};

// Hierarchy <-> frame link.
RpHAnimHierarchy *RpHAnimFrameGetHierarchy(RwFrame *frame);
RwBool RpHAnimFrameSetHierarchy(RwFrame *frame, RpHAnimHierarchy *hierarchy);
RpHAnimHierarchy *RpHAnimHierarchyAttach(RpHAnimHierarchy *hierarchy);
RpHAnimHierarchy *RpHAnimHierarchyAttachFrameIndex(RpHAnimHierarchy *hierarchy, RwInt32 nodeIndex);
RwInt32 RpHAnimIDGetIndex(RpHAnimHierarchy *hierarchy, RwInt32 nodeId);

// Creation and destruction.
RpHAnimHierarchy *RpHAnimHierarchyCreate(RwInt32 numNodes, RwUInt32 *nodeFlags, RwInt32 *nodeIds, RpHAnimHierarchyFlag flags,
                                         RwInt32 maxInterpKeyFrameSize);
RpHAnimHierarchy *RpHAnimHierarchyCreateFromHierarchy(RpHAnimHierarchy *hierarchy, RpHAnimHierarchyFlag flags,
                                                      RwInt32 maxInterpKeyFrameSize);
RpHAnimHierarchy *RpHAnimHierarchyCreateSubHierarchy(RpHAnimHierarchy *parentHierarchy, RwInt32 startNode,
                                                     RpHAnimHierarchyFlag flags, RwInt32 maxInterpKeyFrameSize);
RpHAnimHierarchy *RpHAnimHierarchyDestroy(RpHAnimHierarchy *hierarchy);

// Writes the node matrices from the current animation time.
RwBool RpHAnimHierarchyUpdateMatrices(RpHAnimHierarchy *hierarchy);

}

#endif
