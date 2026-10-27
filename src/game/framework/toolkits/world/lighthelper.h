#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_LIGHTHELPER_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_LIGHTHELPER_H
#include "rw/rpworld.h"
#include "framework/core/cattributehandler.h"

namespace RWS {
namespace LightHelper {
RwBool Destroy(RpLight *pLight);
void HandleSystemCommands(RpLight &rLight, const CAttributePacket &rAttr);
}
}
#endif
