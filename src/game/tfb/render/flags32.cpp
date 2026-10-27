#include "tfb/render/flags32.h"

// GLOBAL: 0x54fec0 ?Get@Flags32@@QBEIXZ
// GLOBAL: 0x45ffe0 ??0Flags32@@QAE@XZ

// 0x45ff60
void Flags32::Set(unsigned bits)
{
	m_bits = bits;
}

// 0x45ff70
unsigned Flags32::Or(unsigned bits)
{
	return m_bits |= bits;
}

// 0x45ff80
unsigned Flags32::AndNot(unsigned bits)
{
	return m_bits &= ~bits;
}

// 0x45ffa0
bool Flags32::Test(unsigned bits)
{
	if (m_bits & bits)
		return true;
	return false;
}
