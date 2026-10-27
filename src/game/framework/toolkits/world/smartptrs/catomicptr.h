// Owns an RpAtomic; AtomicHelper::Destroy when released or replaced.
#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_SMARTPTRS_CATOMICPTR_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_SMARTPTRS_CATOMICPTR_H

#include "rw/rpworld.h"
#include "framework/toolkits/world/smartptrs/crwobjectptr.h"
#include "framework/toolkits/world/atomichelper.h"

namespace RWS {

class CAtomicPtr : public CRwObjectPtr<RpAtomic> {
public:
	CAtomicPtr() {}
	CAtomicPtr(RpAtomic *pAtomic) : CRwObjectPtr<RpAtomic>(pAtomic) {}
	~CAtomicPtr()
	{
		if (m_ptr)
			AtomicHelper::Destroy(m_ptr);
	}

	CAtomicPtr &operator=(RpAtomic *pAtomic)
	{
		if (pAtomic != m_ptr) {
			if (m_ptr)
				AtomicHelper::Destroy(m_ptr);
			m_ptr = pAtomic;
		}
		return *this;
	}

	void HandleSystemCommands(const CAttributePacket &attr)
	{
		if (m_ptr)
			AtomicHelper::HandleSystemCommands(*m_ptr, attr);
	}

	CAtomicPtr(const CAtomicPtr &) = delete;
	CAtomicPtr &operator=(const CAtomicPtr &) = delete;
};

} // namespace RWS

#endif
