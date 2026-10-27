#ifndef ZOOSTERS_FRAMEWORK_CORE_RESOURCEREF_H
#define ZOOSTERS_FRAMEWORK_CORE_RESOURCEREF_H

#include "framework/core/rwsguid.h"

namespace RWS {

// A type word, then the GUID as text with braces, "{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}" (0x26 characters from +4).
struct ResourceRef {
	int m_type;                       // +0x00
	char m_text[0x2c];                // +0x04
	bool GetGuid(RWSGUID *out) const;   // false when a field is not hexadecimal
};

}  // namespace RWS

#endif
