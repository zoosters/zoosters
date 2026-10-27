#include <string.h>
#include "rw/rwcore.h"
#include "tfb/render/tfbstring.h"

// 0x45ffe0
TFBString::TFBString()
{
	m_str = 0;
}

// 0x45fff0
TFBString::~TFBString() throw()
{
	if (m_str)
		operator_delete(m_str);
}

// 0x460000
int TFBString::SerialSize()
{
	if (!m_str)
		return 4;
	return strlen(m_str) + 4;
}

// 0x460020
int TFBString::Read(RwStream *stream)
{
	int length;
	if (!RwStreamReadInt32(stream, &length, 4))
		return 0;
	if (length > 0) {
		m_str = (char *)MemAlloc(length + 1);
		memset(m_str, 0, length + 1);
		RwStreamRead(stream, m_str, length);
	}
	return 4;
}
