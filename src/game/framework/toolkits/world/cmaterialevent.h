// Material event plugin: one receiver pointer per RpMaterial.
#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_CMATERIALEVENT_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_CMATERIALEVENT_H
#include "rw/rpworld.h"

namespace RWS {
namespace MaterialHelper {

// The game never creates one, so every slot stays null. No virtual destructor: the game's vtable has three entries.
class IMaterialEventReceiver {
public:
	virtual void OnMaterialConstructed(RpMaterial *pMaterial) = 0;
	virtual void OnMaterialCopied(const RpMaterial *pSource, RpMaterial *pDestination) = 0;
	virtual void OnMaterialDestroyed(RpMaterial *pMaterial) = 0;
};

// The shim's plugin table (src/port/shim/rwcore.cpp) declares this class with MaterialEventPluginAttach only.
class CMaterialEvent {
public:
	static int MaterialEventPluginAttach(void);

private:
	static void *MaterialEventPlugin_Constructor(void *pObject, RwInt32 offset, RwInt32 size);
	static void *MaterialEventPlugin_Copy(void *pDestination, const void *pSource, RwInt32 offset, RwInt32 size);
	static void *MaterialEventPlugin_Destroy(void *pObject, RwInt32 offset, RwInt32 size);
};

} // namespace MaterialHelper
} // namespace RWS
#endif
