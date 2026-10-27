#pragma auto_inline(off)
#include <stdexcept>

// GLOBAL: 0x5dc0b8 std::logic_error::vftable
// GLOBAL: 0x5dc0c4 std::length_error::vftable
// GLOBAL: 0x5dc0d0 std::out_of_range::vftable
// 0x4338b0
// GLOBAL: 0x401cb0 ??0logic_error@std@@QAE@ABV01@@Z
void w2cThrow(const char *s) { throw std::out_of_range(s); }
