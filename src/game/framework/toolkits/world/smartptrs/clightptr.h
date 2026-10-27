// Owns an RpLight; LightHelper::Destroy when released or replaced.
#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_SMARTPTRS_CLIGHTPTR_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_SMARTPTRS_CLIGHTPTR_H

#include "rw/rpworld.h"
#include "framework/toolkits/world/smartptrs/crwobjectptr.h"
#include "framework/toolkits/world/lighthelper.h"

namespace RWS {

class CLightPtr : public CRwObjectPtr<RpLight> {
public:
	CLightPtr() {}
	CLightPtr(RpLight *pLight) : CRwObjectPtr<RpLight>(pLight) {}
	~CLightPtr()
	{
		if (m_ptr)
			LightHelper::Destroy(m_ptr);
	}

	CLightPtr &operator=(RpLight *pLight)
	{
		if (pLight != m_ptr) {
			if (m_ptr)
				LightHelper::Destroy(m_ptr);
			m_ptr = pLight;
		}
		return *this;
	}

	CLightPtr(const CLightPtr &) = delete;
	CLightPtr &operator=(const CLightPtr &) = delete;
};

} // namespace RWS

// Must stay an exact-match overload: otherwise a CLightPtr argument converts to RpLight * and keeps the destroyed light.
inline RwBool RpLightDestroy(RWS::CLightPtr &rLightPtr)
{
	RWS::LightHelper::Destroy(rLightPtr.ptr());
	rLightPtr.setptr(0);
	return 1;
}

#endif
