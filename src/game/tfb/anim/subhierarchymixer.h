// Plays animations on a sub-tree of a skeleton (jaw, eyelid) and copies the result into the skeleton's interpolator.
#ifndef ZOOSTERS_TFB_ANIM_SUBHIERARCHYMIXER_H
#define ZOOSTERS_TFB_ANIM_SUBHIERARCHYMIXER_H

#include "zoosters/base.h"
#include "rw/rphanim.h"
#include "rw/rtanim.h"
#include "tfb/anim/clock.h"
#include "tfb/render/flags32.h"

namespace TFB {

struct MixerEntry {
	int m_index;                       // set by Init
	const RtAnimAnimation *m_animation; // 0 for an unused slot
	RtAnimInterpolator *m_interpolator; // created by Init for a used slot
	MixerEntry();
	~MixerEntry();
};
ZOOSTERS_ASSERT_SIZE(MixerEntry, 0xc);

enum MixerState {
	MIXER_IDLE = 0,    // nothing playing
	MIXER_PLAYING = 2  // Update advances the clock and writes the nodes (1 is never set, behaves like idle)
};

enum MixerFlags {
	MIXER_EMOTING = 1 // AnimMouth::Update blends entry 8 over the mouth shapes (debug path)
};

class SubHierarchyMixer {
public:
	SubHierarchyMixer();
	~SubHierarchyMixer();  // not virtual: the owners call the derived destructors

	// Binds to the sub-tree rooted at nodeId, with a sub-interpolator per non-null animation. Fails if an animation's node count differs from the sub-tree's.
	virtual unsigned char Init(RpHAnimHierarchy *hierarchy, int nodeId, int count, const RtAnimAnimation **anims);
	// False when the node does not exist.
	virtual unsigned char AttachHierarchy(RpHAnimHierarchy *hierarchy, int nodeId);
	virtual void Update(float t, bool absoluteTime) = 0;

	RpHAnimHierarchy *m_hierarchy;   // not owned
	RpHAnimHierarchy *m_subHierarchy; // owned
	int m_nodeId;                    // -1 before AttachHierarchy
	int m_firstNode;
	int m_nodeCount;
	MixerEntry *m_entries;           // owned
	unsigned m_entryCount;
	Clock m_clock;
	Flags32 m_flags;
	int m_state;                     // MixerState; the derived constructors clear it

protected:
	// Copies the nodes first.. of source into the skeleton's own interpolator.
	__forceinline void CopyFrames(RtAnimInterpolator *source, int first)
	{
		for (int i = first; i < source->numNodes; i++) {
			RtAnimInterpolator *skeleton = m_hierarchy->currentAnim;
			RpHAnimInterpFrame *to = (RpHAnimInterpFrame *)(RtAnimInterpolatorGetInterpFrames(skeleton) +
			                                                (m_firstNode + i) * skeleton->currentInterpKeyFrameSize);
			RpHAnimInterpFrame *from = (RpHAnimInterpFrame *)(RtAnimInterpolatorGetInterpFrames(source) +
			                                                  source->currentInterpKeyFrameSize * i);
			to->orientation = from->orientation;
			to->translation = from->translation;
		}
	}
};
ZOOSTERS_ASSERT_SIZE(SubHierarchyMixer, 0x50);
ZOOSTERS_ASSERT_OFFSET(SubHierarchyMixer, m_entries, 0x18);
ZOOSTERS_ASSERT_OFFSET(SubHierarchyMixer, m_clock, 0x20);
ZOOSTERS_ASSERT_OFFSET(SubHierarchyMixer, m_flags, 0x48);
ZOOSTERS_ASSERT_OFFSET(SubHierarchyMixer, m_state, 0x4c);

} // namespace TFB

#endif
