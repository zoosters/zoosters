// Owns an RpClump; ClumpHelper::Destroy when released or replaced.
#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_SMARTPTRS_CCLUMPPTR_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_SMARTPTRS_CCLUMPPTR_H

#include "rw/rpworld.h"
#include "framework/toolkits/world/smartptrs/crwobjectptr.h"
#include "framework/toolkits/world/clumphelper.h"

namespace RWS {

class CClumpPtr : public CRwObjectPtr<RpClump> {
public:
	CClumpPtr() {}
	CClumpPtr(RpClump *pClump) : CRwObjectPtr<RpClump>(pClump) {}
	~CClumpPtr()
	{
		if (m_ptr)
			ClumpHelper::Destroy(m_ptr);
	}

	CClumpPtr &operator=(RpClump *pClump)
	{
		if (pClump != m_ptr) {
			if (m_ptr)
				ClumpHelper::Destroy(m_ptr);
			m_ptr = pClump;
		}
		return *this;
	}

	CClumpPtr(const CClumpPtr &) = delete;
	CClumpPtr &operator=(const CClumpPtr &) = delete;
};

} // namespace RWS

// Exact-match overload, see RpLightDestroy in clightptr.h.
inline RwBool RpClumpDestroy(RWS::CClumpPtr &rClumpPtr)
{
	RWS::ClumpHelper::Destroy(rClumpPtr.ptr());
	rClumpPtr.setptr(0);
	return 1;
}

#endif
