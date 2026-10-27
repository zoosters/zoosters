#ifndef ZOOSTERS_FRAMEWORK_CORE_CSYSTEMCOMMANDS_H
#define ZOOSTERS_FRAMEWORK_CORE_CSYSTEMCOMMANDS_H

#include "framework/core/cattributehandler.h"

struct RwFrame;
struct RpAtomic;
struct RpClump;

namespace RWS {

// Static helpers; empty base class of the behaviours.
class CSystemCommands {
public:
	static const RWSGUID *FindFirstResource(const CAttributePacket &attr);
	static const char *pCSystemCommandsClassID;
	static void UpdateFrame(RwFrame &frame, const CAttributeCommand &attrCmd);
};

// RpAtomic plugin: per-atomic "is solid" bit (default solid).
namespace RpAtomicCollisionProperties {
int PluginAttach(void);
void SetIsSolid(RpAtomic &atomic, bool isSolid);
RpAtomic *SetIsSolidCallback(RpAtomic *pAtomic, void *pData);
void SetIsSolid(RpClump &clump, bool isSolid);
}  // namespace RpAtomicCollisionProperties

}  // namespace RWS

#endif
