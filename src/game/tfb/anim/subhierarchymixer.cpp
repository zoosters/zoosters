#include "tfb/anim/subhierarchymixer.h"

// Sub-hierarchy of the last node with this id; 0 when none.
// 0x45aff0
RpHAnimHierarchy *CreateNodeSubHierarchy(RpHAnimHierarchy *hierarchy, int nodeId)
{
	unsigned flags = hierarchy->flags;
	int nodeCount = hierarchy->numNodes;
	int index = -1;
	for (int i = 0; i < nodeCount; i++) {
		if (nodeId == hierarchy->pNodeInfo[i].nodeID)
			index = i;
	}
	if (index < 0)
		return 0;
	return RpHAnimHierarchyCreateSubHierarchy(hierarchy, index, (RpHAnimHierarchyFlag)flags, -1);
}

// Nodes from `start` to the end of its sub-tree: the push / pop flags give the nesting depth.
// 0x45b040
static __declspec(noinline) int HAnimSubTreeNodeCount(RpHAnimHierarchy *hierarchy, int start)
{
	int i = start;
	if (i > -1 && i < hierarchy->numNodes) {
		RpHAnimNodeInfo *node = hierarchy->pNodeInfo + start;
		int depth = 0;
		if (node->flags & rpHANIMPUSHPARENTMATRIX)
			depth = -1;
		do {
			if (node->flags & rpHANIMPUSHPARENTMATRIX)
				depth++;
			if (node->flags & rpHANIMPOPPARENTMATRIX)
				depth--;
			i++;
			node++;
		} while (depth >= 0);
	}
	return i - start;
}

namespace TFB {

// GLOBAL: 0x5d5758 TFB::SubHierarchyMixer::vftable

// 0x45b220
MixerEntry::MixerEntry()
{
	m_index = 0;
	m_animation = 0;
	m_interpolator = 0;
}

// 0x45b230
MixerEntry::~MixerEntry()
{
	if (m_interpolator)
		RtAnimInterpolatorDestroy(m_interpolator);
}

// 0x45b240
__declspec(noinline) SubHierarchyMixer::SubHierarchyMixer()
{
	m_hierarchy = 0;
	m_subHierarchy = 0;
	m_nodeId = -1;
	m_entryCount = 0;
	m_entries = 0;
	m_clock.Rewind(0);
	m_clock.m_playMode = CLOCK_MODE_CLAMP;
	m_flags.Set(0);
}

// 0x45b600
__declspec(noinline) SubHierarchyMixer::~SubHierarchyMixer()
{
	if (m_subHierarchy)
		RpHAnimHierarchyDestroy(m_subHierarchy);
	if (m_entries)
		delete[] m_entries;
}

// 0x45b2c0
unsigned char SubHierarchyMixer::AttachHierarchy(RpHAnimHierarchy *hierarchy, int nodeId)
{
	m_hierarchy = hierarchy;
	m_nodeId = nodeId;
	m_subHierarchy = CreateNodeSubHierarchy(hierarchy, nodeId);
	if (!m_subHierarchy)
		return false;
	m_firstNode = RpHAnimIDGetIndex(m_hierarchy, nodeId);
	m_nodeCount = HAnimSubTreeNodeCount(m_hierarchy, m_firstNode);
	return true;
}

// 0x45b310
unsigned char SubHierarchyMixer::Init(RpHAnimHierarchy *hierarchy, int nodeId, int count, const RtAnimAnimation **anims)
{
	if (!AttachHierarchy(hierarchy, nodeId))
		return false;
	m_entryCount = count;
	m_entries = new MixerEntry[count];
	if (!m_entries)
		return false;
	m_entries->m_index = 0; // redundant, the game has the store
	for (unsigned i = 0; i < m_entryCount; i++) {
		m_entries[i].m_index = i;
		m_entries[i].m_animation = anims[i];
		if (m_entries[i].m_animation) {
			RtAnimInterpolator *parent = m_hierarchy->currentAnim;
			m_entries[i].m_interpolator =
			    RtAnimInterpolatorCreateSubInterpolator(parent, m_firstNode, m_nodeCount, parent->maxInterpKeyFrameSize);
			if (!m_entries[i].m_interpolator)
				return false;
			if (RtAnimAnimationGetNumNodes(m_entries[i].m_animation) != m_entries[i].m_interpolator->numNodes) {
				delete[] m_entries;
				m_entries = 0;
				return false;
			}
			RtAnimInterpolatorSetCurrentAnim(m_entries[i].m_interpolator, const_cast<RtAnimAnimation *>(m_entries[i].m_animation));
		}
	}
	return true;
}

} // namespace TFB
