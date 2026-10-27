#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_ATOMICHELPER_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_ATOMICHELPER_H
#include "rw/rpworld.h"
#include "framework/core/cattributehandler.h"

namespace RWS {
// CSystemCommands attribute command ids.
enum SystemCommand { CMD_LoadMatrix = 1, CMD_SetSolidFlag = 2, CMD_SetCollisionFlag = 3, CMD_SetInvisibleFlag = 4 };
namespace AtomicHelper {
RwBool Destroy(RpAtomic *pAtomic);
void SetIsVisible(RpAtomic &pAtomic, bool isVisible);
void SetCanCollide(RpAtomic &pAtomic, bool canCollide);
void HandleSystemCommands(RpAtomic &rAtomic, const CAttributePacket &rAttr);
}
}
#endif
