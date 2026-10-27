#include "tfb/render/tfbatomic.h"

void AtomicApplyUserDataFlags(RpAtomic *a);

// 0x44ed00
__declspec(noinline) void ClumpSetAtomicColour(RpClump *clump, RwRGBA *colour)
{
	RwLLLink *cur = clump->atomicList.link.next;
	RwLLLink *end = &clump->atomicList.link;
	if (cur != end) {
		do {
			RwLLLink *next = cur->next;
			RpAtomic *atomic = RpAtomicFromClumpLink(cur);
			TFBAtomicGetData(atomic)->m_color = *colour;
			if (!atomic)
				break;
			cur = next;
		} while (cur != end);
	}
}

// 0x44ed40
void ClumpSetColourARGB(RpClump *clump, unsigned argb)
{
	RwRGBA c;
	c.alpha = (RwUInt8)(argb >> 24);
	c.red = (RwUInt8)(argb >> 16);
	c.green = (RwUInt8)(argb >> 8);
	c.blue = (RwUInt8)argb;
	ClumpSetAtomicColour(clump, &c);
}

// 0x44ee90
RpAtomic *AtomicApplyUserDataFlagsCB(RpAtomic *atomic, void *)
{
	AtomicApplyUserDataFlags(atomic);
	return atomic;
}

// 0x44eeb0
void ClumpApplyUserDataFlags(RpClump *clump)
{
	RpClumpForAllAtomics(clump, AtomicApplyUserDataFlagsCB, 0);
}
