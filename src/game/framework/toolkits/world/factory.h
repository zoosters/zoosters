#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_FACTORY_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_FACTORY_H
#include "rw/rpworld.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/ceventhandler.h"

namespace RWS {
RpClump *CreateClumpFromResource(const CAttributePacket &attr, CEventHandler *eventHandler);
RpAtomic *CreateAtomicFromResource(const CAttributePacket &attr, CEventHandler *eventHandler);
RpAtomic *CreateAtomicInWorldFromResource(const CAttributePacket &attr, CEventHandler *eventHandler);
RpClump *CreateClumpInWorldFromResource(const CAttributePacket &attr, CEventHandler *eventHandler);
}
void ScanSystemCommandChunksThunk(const RWS::CAttributePacket &attr, int arg);   // 0x464e70, thunk to 0x464d50
#endif
