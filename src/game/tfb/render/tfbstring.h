// TFBString: a heap-allocated NUL-terminated string read from the animation and material streams (4-byte length, then the characters).
#ifndef ZOOSTERS_TFB_RENDER_TFBSTRING_H
#define ZOOSTERS_TFB_RENDER_TFBSTRING_H

#include "rw/rwcore.h"

void *MemAlloc(size_t size);   // 0x550170
void operator_delete(void *ptr); // 0x5500f0

class TFBString {
public:
	char *m_str;
	TFBString();
	~TFBString() throw();  // (throw: the game's owners need no unwind state for it)
	int SerialSize();  // 4 + the length of the text
	int Read(RwStream *stream);  // returns 4 (the length word); the text is read only when the length is positive
};

#endif
