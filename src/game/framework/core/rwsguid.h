// 128-bit id of entities and resources.
#ifndef ZOOSTERS_FRAMEWORK_CORE_RWSGUID_H
#define ZOOSTERS_FRAMEWORK_CORE_RWSGUID_H

#include "zoosters/base.h"

namespace RWS {

class RWSGUID {
public:
	void Clear();
	bool operator==(const RWSGUID &guid) const;
	unsigned m_Data[4];
};
ZOOSTERS_ASSERT_SIZE(RWSGUID, 16);

inline void RWSGUID::Clear()
{
	m_Data[0] = m_Data[1] = m_Data[2] = m_Data[3] = 0;
}

inline bool RWSGUID::operator==(const RWSGUID &guid) const
{
	return m_Data[0] == guid.m_Data[0] && m_Data[1] == guid.m_Data[1] && m_Data[2] == guid.m_Data[2] &&
	       m_Data[3] == guid.m_Data[3];
}

}  // namespace RWS

#endif
