#include <string.h>
#include <stdlib.h>
#include "framework/core/resourceref.h"

namespace RWS {

// 0x4500b0
bool ResourceRef::GetGuid(RWSGUID *out) const
{
	char buf[9];
	char *end;
	const char *s = (const char *)this;   // '{' at +4, the first field at +5
	buf[8] = 0;
	// "<none>" (an unset reference) fails to parse; the original leaves stack garbage in the words. Zeroed here to stay deterministic.
	out->m_Data[0] = out->m_Data[1] = out->m_Data[2] = out->m_Data[3] = 0;
	strncpy(buf, s + 5, 8);
	out->m_Data[0] = strtoul(buf, &end, 16);
	if (end != buf + 8)
		return false;
	strncpy(buf + 4, s + 0xe, 4);
	strncpy(buf, s + 0x13, 4);
	out->m_Data[1] = strtoul(buf, &end, 16);
	if (end != buf + 8)
		return false;
	strncpy(buf, s + 0x1f, 2);
	strncpy(buf + 2, s + 0x1d, 2);
	strncpy(buf + 4, s + 0x1a, 2);
	strncpy(buf + 6, s + 0x18, 2);
	out->m_Data[2] = strtoul(buf, &end, 16);
	if (end != buf + 8)
		return false;
	strncpy(buf, s + 0x27, 2);
	strncpy(buf + 2, s + 0x25, 2);
	strncpy(buf + 4, s + 0x23, 2);
	strncpy(buf + 6, s + 0x21, 2);
	out->m_Data[3] = strtoul(buf, &end, 16);
	return end == buf + 8;
}

}  // namespace RWS
