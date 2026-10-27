// Flag word of the Studio framework's data classes.
#ifndef ZOOSTERS_TFB_RENDER_FLAGS32_H
#define ZOOSTERS_TFB_RENDER_FLAGS32_H

class Flags32 {
public:
	unsigned m_bits;
	// Inline in most of the game; where cl did not inline it the call goes to 0x45ffe0, shared with TFBString::TFBString.
	Flags32() : m_bits(0) {}
	void Set(unsigned bits);
	unsigned Or(unsigned bits);  // returns the new word
	unsigned AndNot(unsigned bits);  // clears the bits, returns the new word
	bool Test(unsigned bits);  // any of the bits set
	unsigned Get() const { return m_bits; }   // the game's linker folded it with CAttributeCommandIterator::operator->
};

#endif
