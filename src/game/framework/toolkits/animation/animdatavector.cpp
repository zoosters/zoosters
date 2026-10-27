// The game's out-of-line copies of the STL helpers behind std::vector<AnimData>, in the shape of the MSVC 7.1 templates.
#include "framework/toolkits/animation/canimset.h"

using namespace RWS;

// std::fill, called from _Insert_n
// 0x4493d0
void AnimDataFill(AnimData *first, AnimData *last, const AnimData *val)
{
	for (; first != last; ++first)
		*first = *val;
}

// std::copy_backward
// 0x449420
AnimData *AnimDataCopyBackward(AnimData *first, AnimData *last, AnimData *dest)
{
	while (first != last)
		*--dest = *--last;
	return dest;
}

// std::_Uninit_copy: placement copy, null-checked
// 0x4494c0
AnimData *AnimDataUninitCopy(AnimData *first, AnimData *last, AnimData *dest)
{
	for (; first != last; ++dest, ++first)
		if (dest)
			*dest = *first;
	return dest;
}

// std::_Uninit_fill_n
// 0x449510
void AnimDataUninitFillN(AnimData *dest, unsigned count, const AnimData *val)
{
	for (; 0 < count; --count, ++dest)
		if (dest)
			*dest = *val;
}

// std::copy: plain assignment loop, no null check
// 0x44f8f0
AnimData *AnimDataCopy(AnimData *first, AnimData *last, AnimData *dest)
{
	while (first != last) {
		*dest = *first;
		++first;
		++dest;
	}
	return dest;
}
