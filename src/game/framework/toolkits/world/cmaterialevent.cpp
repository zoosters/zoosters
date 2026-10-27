#include "framework/toolkits/world/cmaterialevent.h"

namespace RWS {
namespace MaterialHelper {

// GLOBAL: 0x629758 RWS::MaterialHelper::iCMaterialEventPlugin_Offset
RwUInt32 iCMaterialEventPlugin_Offset;

namespace {

const RwUInt32 kPluginId = (7u << 8) | 0xFE; // vendor 7, plugin 0xFE

IMaterialEventReceiver *&ReceiverOf(const void *pMaterial)
{
	char *pSlot = static_cast<char *>(const_cast<void *>(pMaterial)) + iCMaterialEventPlugin_Offset;
	return *reinterpret_cast<IMaterialEventReceiver **>(pSlot);
}

} // namespace

// 0x44dd00
void *CMaterialEvent::MaterialEventPlugin_Constructor(void *pObject, RwInt32, RwInt32)
{
	ReceiverOf(pObject) = 0;
	return pObject;
}

// 0x44dd20
void *CMaterialEvent::MaterialEventPlugin_Copy(void *pDestination, const void *pSource, RwInt32, RwInt32)
{
	IMaterialEventReceiver *pReceiver = ReceiverOf(pSource);
	ReceiverOf(pDestination) = pReceiver;
	if (pReceiver)
		pReceiver->OnMaterialCopied(static_cast<const RpMaterial *>(pSource), static_cast<RpMaterial *>(pDestination));
	return pDestination;
}

// 0x44dd50
void *CMaterialEvent::MaterialEventPlugin_Destroy(void *pObject, RwInt32, RwInt32)
{
	IMaterialEventReceiver *pReceiver = ReceiverOf(pObject);
	if (pReceiver)
		pReceiver->OnMaterialDestroyed(static_cast<RpMaterial *>(pObject));
	return pObject;
}

// A non-zero offset counts as success, also RW's negative failure value.
// 0x44dd70
int CMaterialEvent::MaterialEventPluginAttach(void)
{
	const RwInt32 size = sizeof(IMaterialEventReceiver *);
	iCMaterialEventPlugin_Offset = RpMaterialRegisterPlugin(size, kPluginId, MaterialEventPlugin_Constructor,
	                                                        MaterialEventPlugin_Destroy, MaterialEventPlugin_Copy);
	return iCMaterialEventPlugin_Offset != 0;
}

} // namespace MaterialHelper
} // namespace RWS
