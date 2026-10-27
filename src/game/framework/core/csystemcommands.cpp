#include "framework/core/csystemcommands.h"
#include "rw/rwcore.h"
#include "rw/rpworld.h"

#include <string.h>

namespace RWS {

namespace {

enum { SYSTEM_COMMAND_ATTACH_RESOURCE = 0 };

} // namespace

// 0x550d80
const RWSGUID *CSystemCommands::FindFirstResource(const CAttributePacket &attr)
{
	for (CAttributeCommandIterator it(attr, pCSystemCommandsClassID); !it.IsFinished(); ++it) {
		if (it->GetCommandId() == SYSTEM_COMMAND_ATTACH_RESOURCE) {
			const RWSGUID *resourceId;
			it->GetCommandData(&resourceId);
			return resourceId;
		}
	}
	return 0;
}

// Copies all 64 bytes, flags and pad words included.
// 0x550c70
void CSystemCommands::UpdateFrame(RwFrame &frame, const CAttributeCommand &attrCmd)
{
	RwMatrix *matrix = RwFrameGetMatrix(&frame);
	memcpy(matrix, attrCmd.GetAs_void_ptr(), sizeof(RwMatrix));
	RwMatrixOptimize(matrix, 0);
	RwFrameUpdateObjects(&frame);
}

namespace RpAtomicCollisionProperties {
namespace {

const RwUInt32 PLUGIN_ID = 0x7FE; // vendor 7, plugin 0xFE
const RwUInt32 SOLID = 1;

// GLOBAL: 0x62fee0 RWS::RpAtomicCollisionProperties::anon::iPlugin_Offset
RwInt32 g_pluginOffset;

inline RwUInt32 &Flags(void *atomic, RwInt32 offset)
{
	return *reinterpret_cast<RwUInt32 *>(static_cast<char *>(atomic) + offset);
}

// 0x550ca0
void *Plugin_Constructor(void *object, RwInt32 offset, RwInt32 size)
{
	Flags(object, offset) |= SOLID;
	return object;
}

// 0x550cb0
void *Plugin_Copy(void *dst, const void *src, RwInt32 offset, RwInt32 size)
{
	Flags(dst, offset) = Flags(const_cast<void *>(src), offset);
	return dst;
}

} // namespace

// 0x550cd0
int PluginAttach(void)
{
	g_pluginOffset = RpAtomicRegisterPlugin(sizeof(RwUInt32), PLUGIN_ID, Plugin_Constructor, 0, Plugin_Copy);
	return g_pluginOffset != 0;
}

// 0x550d00
void SetIsSolid(RpAtomic &atomic, bool isSolid)
{
	RwUInt32 &flags = Flags(&atomic, g_pluginOffset);
	flags = (flags & ~SOLID) | (isSolid ? SOLID : 0);
}

// 0x550d30
RpAtomic *SetIsSolidCallback(RpAtomic *pAtomic, void *pData)
{
	SetIsSolid(*pAtomic, pData != 0);
	return pAtomic;
}

// 0x550d60
void SetIsSolid(RpClump &clump, bool isSolid)
{
	RpClumpForAllAtomics(&clump, SetIsSolidCallback, isSolid ? reinterpret_cast<void *>(1) : 0);
}

} // namespace RpAtomicCollisionProperties

} // namespace RWS
