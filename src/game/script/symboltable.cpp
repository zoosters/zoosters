// The symbol registries' vectors with the real std::vector (MSVC 7.1 STL), element types E8 (8 B) and E10C (0x10c B).
// cl instantiates exactly the game's out-of-line members.
#include <vector>

#include "script/symbolregistry.h"
#include "script/w2cregistry.h"

// 0x433330
// 0x4339a0
// 0x433e50
// 0x433580
// 0x433290
// 0x4332c0
// 0x4338d0
// 0x434070
// 0x433d30
void w2cPush8(std::vector<SymE8> &v, const SymE8 &e) { v.push_back(e); }
void w2cPush10C(std::vector<SymE10C> &v, const SymE10C &e) { v.push_back(e); }
void w2cReserve8(std::vector<SymE8> &v, unsigned n) { v.reserve(n); }

// 0x433da0
void w2cCtor10C(unsigned n, const SymE10C &e) { std::vector<SymE10C> v(n, e); }


// Near-miss: 0x4341f0 W2cSymTable::Clear: `SymE10C e; vector t(0,e); m_vec.swap(t);` is instruction-identical except cl omits the EH state store
// `mov [esp+124h],0` (-11 B) that the game has before the _Construct_n call; (vector(0) with T zero-fills the temp, wrong).
// 0x4341f0 W2cSymTable::Clear: only hunk is the EH state store (mov [esp+124h],0 before _Construct_n) and its unwind funclet; normal path identical (alloc 0 elements, swap, free old buffer); no behaviour difference outside exception unwinding
void W2cSymTable::Clear()
{
	m_vec.reset();
}

// ---- 0x434100: the script's symbol table: u32 n, then n x (pascal string, u32)
extern int g_tfbReadPos;                 // GLOBAL: 0x61f888 g_tfbReadPos
extern unsigned char *g_tfbReadBuf;      // GLOBAL: 0x61f88c g_tfbReadBuf
void TFBRead_u32(unsigned int *out);     // 0x432920
void TFBRead_pstr(char *out);            // 0x4329a0
unsigned NameHashNoCase(const char *s);    // 0x43bb20
static inline void w2cRead8(unsigned char *out)
{
	*out = g_tfbReadBuf[g_tfbReadPos];
	g_tfbReadPos++;
}
struct W2cSymEnt {
	char name[0x100];
	unsigned id;
	unsigned hash;
	int m_108;
};

// 0x434100
void W2cSymTable::Read()
{
	unsigned left;
	{
		unsigned n;
		TFBRead_u32(&n);
		left = n;
	}
	for (; left > 0; left--) {
		W2cSymEnt e;
		char buf[0x100];
		TFBRead_pstr(buf);
		unsigned char b[4];
		w2cRead8(&b[0]);
		w2cRead8(&b[1]);
		w2cRead8(&b[2]);
		w2cRead8(&b[3]);
		e.id = ((b[3] << 8 | b[2]) << 8 | b[1]) << 8 | b[0];
		e.hash = NameHashNoCase(buf);
		e.m_108 = 0;
		int i = 0;
		char c;
		do {
			c = buf[i];
			e.name[i] = c;
			i++;
		} while (c != 0);
		m_vec.push_back(*(SymE10C *)&e);
	}
}
