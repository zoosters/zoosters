#include "framework/toolkits/animation/skinhelper.h"
#include "rw/rpworld.h"
#include "rw/rpplugins.h"

namespace RWS {
namespace SkinHelper {

// 0x44f180
int FindFirstSkinCallback(void *skin, void **out)
{
	*out = skin;
	return 0;
}

// 0x44f150
RpAtomic *ForAllSkinsCallback(RpAtomic *atomic, SkinCallbackData *cbData)
{
	void *skin = RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic));
	if (skin) {
		if (!cbData->m_callback(skin, cbData->m_pData))
			return 0;
	}
	return atomic;
}

}   // namespace SkinHelper
}   // namespace RWS
