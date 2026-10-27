// RtAnim: animation data, interpolators (the playback state a skeleton carries), the compressed key frame type.
// Partial layouts: never allocate or copy.
#ifndef ZOOSTERS_RW_RTANIM_H
#define ZOOSTERS_RW_RTANIM_H

#include "rw/rwcore.h"
extern "C" {

// The key frame sizes of an animation's interpolation scheme (RtAnimInterpolatorInfo). Partial.
struct RtAnimInterpolatorInfo {
	RwInt32 typeID;
	RwInt32 interpKeyFrameSize;
	RwInt32 animKeyFrameSize;  // read by AnimationIsCompatibleWithHierarchy (0x44a870)
};
ZOOSTERS_ASSERT_OFFSET(RtAnimInterpolatorInfo, animKeyFrameSize, 0x08);

// The header every key frame starts with (RtAnimKeyFrameHeader): the previous key frame of the node and the key frame's time.
struct RtAnimKeyFrameHeader {
	void *prevFrame;
	RwReal time;
};
ZOOSTERS_ASSERT_OFFSET(RtAnimKeyFrameHeader, time, 0x04);

// A key-framed animation (RtAnimAnimation): its interpolation scheme, length in seconds and the key frame array. Partial.
struct RtAnimAnimation {
	RtAnimInterpolatorInfo *interpInfo;
	RwInt32 numFrames;
	RwInt32 flags;
	RwReal duration;
	void *pFrames;  // the key frames, animKeyFrameSize bytes each
};
ZOOSTERS_ASSERT_OFFSET(RtAnimAnimation, interpInfo, 0x00);
ZOOSTERS_ASSERT_OFFSET(RtAnimAnimation, numFrames, 0x04);
ZOOSTERS_ASSERT_OFFSET(RtAnimAnimation, flags, 0x08);
ZOOSTERS_ASSERT_OFFSET(RtAnimAnimation, duration, 0x0c);
ZOOSTERS_ASSERT_OFFSET(RtAnimAnimation, pFrames, 0x10);

// The playback state of one animation over a set of nodes (RtAnimInterpolator). Partial: the members the tree reads.
struct RtAnimInterpolator {
	RtAnimAnimation *pCurrentAnim;
	RwReal currentTime;
	void *pNextFrame;
	void *pAnimCallBack;
	void *pAnimCallBackData;
	RwReal animCallBackTime;
	void *pAnimLoopCallBack;
	void *pAnimLoopCallBackData;
	RwInt32 maxInterpKeyFrameSize;
	RwInt32 currentInterpKeyFrameSize;  // size of one interpolated key frame
	RwInt32 currentAnimKeyFrameSize;
	RwInt32 numNodes;
	RwInt32 isSubInterpolator;
	RwInt32 offsetInParent;
	void *pParentAnimation;
	void *keyFrameApplyCB;  // the key frame callbacks of the scheme (apply, blend, interpolate, add)
	void *keyFrameBlendCB;
	void *keyFrameInterpolateCB;
	void *keyFrameAddCB;
};
ZOOSTERS_ASSERT_SIZE(RtAnimInterpolator, 0x4c);
// The interpolated key frames follow the structure (RW: rtANIMGETINTERPFRAME), one per node, currentInterpKeyFrameSize bytes each.
static inline char *RtAnimInterpolatorGetInterpFrames(RtAnimInterpolator *interpolator)
{
	return (char *)interpolator + sizeof(RtAnimInterpolator);
}
ZOOSTERS_ASSERT_OFFSET(RtAnimInterpolator, pCurrentAnim, 0x00);
ZOOSTERS_ASSERT_OFFSET(RtAnimInterpolator, currentTime, 0x04);
ZOOSTERS_ASSERT_OFFSET(RtAnimInterpolator, maxInterpKeyFrameSize, 0x20);
ZOOSTERS_ASSERT_OFFSET(RtAnimInterpolator, currentInterpKeyFrameSize, 0x24);
ZOOSTERS_ASSERT_OFFSET(RtAnimInterpolator, numNodes, 0x2c);

RwBool RtAnimInitialize(void);
RwBool RtCompressedKeyFrameRegister(void); // registers the compressed key frame type (rtcmpkey)

// Animation data.
RtAnimAnimation *RtAnimAnimationStreamRead(RwStream *stream);
RwBool RtAnimAnimationDestroy(RtAnimAnimation *animation);
RwUInt32 RtAnimAnimationGetNumNodes(const RtAnimAnimation *animation);

// Interpolators: playback of one animation over a set of nodes.
RwBool RtAnimInterpolatorSetCurrentAnim(RtAnimInterpolator *interpolator, RtAnimAnimation *animation);
RwBool RtAnimInterpolatorSetCurrentTime(RtAnimInterpolator *interpolator, RwReal time);
RwBool RtAnimInterpolatorAddAnimTime(RtAnimInterpolator *interpolator, RwReal time);
RwBool RtAnimInterpolatorSubAnimTime(RtAnimInterpolator *interpolator, RwReal time);
// out = blend of in1 and in2 by alpha
RwBool RtAnimInterpolatorBlend(RtAnimInterpolator *outInterpolator, RtAnimInterpolator *in1, RtAnimInterpolator *in2,
                               RwReal alpha);
// An interpolator for `numNodes` nodes of `parent`, starting at node `startNode`.
RtAnimInterpolator *RtAnimInterpolatorCreateSubInterpolator(RtAnimInterpolator *parent, RwInt32 startNode, RwInt32 numNodes,
                                                            RwInt32 maxInterpKeyFrameSize);
void RtAnimInterpolatorDestroy(RtAnimInterpolator *interpolator);

}

#endif
