#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_CLUMPHELPER_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_CLUMPHELPER_H
#include "rw/rpworld.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/ceventhandler.h"

namespace RWS {
namespace ClumpHelper {
void Open();
void Close();
RwBool Destroy(RpClump *clump);
void SetEventHandler(RpClump &clump, CEventHandler *handler);
void HandleSystemCommands(RpClump &clump, const CAttributePacket &attributes);
}
}
#endif
