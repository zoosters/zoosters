// TFB script stream readers: the loader (0x4349e0) advances a global cursor, these read primitives from it.
// All __cdecl, one out-pointer.
#include "zoosters/base.h"

// GLOBAL: 0x61f888 g_tfbReadPos
extern int g_tfbReadPos;
// GLOBAL: 0x61f88c g_tfbReadBuf
extern unsigned char *g_tfbReadBuf;

// 0x4328d0
void TFBRead_u8(unsigned char *out)
{
	*out = g_tfbReadBuf[g_tfbReadPos];
	g_tfbReadPos++;
}

// 0x4328f0
void TFBRead_u16(unsigned short *out)
{
	unsigned char lo = g_tfbReadBuf[g_tfbReadPos++];
	unsigned char hi = g_tfbReadBuf[g_tfbReadPos++];
	*out = (unsigned short)(hi << 8 | lo);
}

// 0x432920
void TFBRead_u32(unsigned int *out)
{
	unsigned char b[4];
	TFBRead_u8(&b[0]);
	TFBRead_u8(&b[1]);
	TFBRead_u8(&b[2]);
	TFBRead_u8(&b[3]);
	*out = ((b[3] << 8 | b[2]) << 8 | b[1]) << 8 | b[0];
}

// 0x4329a0
void TFBRead_pstr(char *out)
{
	char scratch[256];
	unsigned char n = g_tfbReadBuf[g_tfbReadPos++];
	if (out == 0)
		out = scratch;
	for (; n != 0; n--) {
		TFBRead_u8((unsigned char *)out);
		out++;
	}
	if (*out != '\0')
		*out = '\0';
}

void TFBRead_u32_thunk(unsigned *out)   // the game's 5-byte jump to TFBRead_u32 with the caller's argument
{
	TFBRead_u32(out);
}
