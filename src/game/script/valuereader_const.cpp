// ScriptValueReader constant-operand readers (ConstU32 / ConstU8 / ConstU16): the typed-by-void* definitions; reader.cpp calls them.
#include "script/ops_message.h"
using namespace Script;

#define UF(T, off) (*(T *)((char *)this + (off)))

void TFBRead_u32_thunk(unsigned *out);       // 0x432990: 5-byte jump to TFBRead_u32

// The three constant-operand readers (ScriptRefReader stack object = `this`): store the tag, kind pointer and the
// "reading a constant" state, then read the constant through the byte-stream reader.
// 0x43fc50
ScriptValueReader *ScriptValueReader::ConstU32(void *dest, unsigned char *kind, ScriptReference *type, unsigned tag)
{
	UF(int, 0x100) = -1;
	UF(unsigned, 0x108) = tag;
	UF(int, 0x10c) = 3;
	UF(void *, 0x118) = kind;
	TFBRead_u32_thunk((unsigned *)dest);
	return this;
}

// 0x43fc90
ScriptValueReader *ScriptValueReader::ConstU8(void *dest, unsigned char *kind, ScriptReference *type, unsigned tag)
{
	UF(int, 0x100) = -1;
	UF(unsigned, 0x108) = tag;
	UF(int, 0x10c) = 3;
	UF(void *, 0x118) = kind;
	TFBRead_u8((unsigned char *)dest);
	return this;
}

// 0x43fcd0
ScriptValueReader *ScriptValueReader::ConstU16(void *dest, unsigned char *kind, ScriptReference *type, unsigned tag)
{
	UF(int, 0x100) = -1;
	UF(unsigned, 0x108) = tag;
	UF(int, 0x10c) = 3;
	UF(void *, 0x118) = kind;
	TFBRead_u16((unsigned short *)dest);
	return this;
}

