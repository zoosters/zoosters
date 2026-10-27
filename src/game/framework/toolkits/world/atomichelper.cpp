#include "framework/toolkits/world/atomichelper.h"
#include "framework/toolkits/world/framehelper.h"
#include "framework/core/csystemcommands.h"

// GLOBAL: 0x550d00 ?SetIsSolid@RpAtomicCollisionProperties@RWS@@YAXAAURpAtomic@@_N@Z

namespace RWS {
namespace AtomicHelper {

namespace {

void SetFlag(RpAtomic &atomic, RwUInt32 flag, bool on)
{
	RwUInt32 flags = RpAtomicGetFlags(&atomic);
	if (on)
		flags |= flag;
	else
		flags &= ~flag;
	RpAtomicSetFlags(&atomic, flags);
}

} // namespace

// 0x44e260
RwBool Destroy(RpAtomic *atomic)
{
	RwFrame *frame = RpAtomicGetFrame(atomic);
	if (frame) {
		RwFrameForAllChildren(frame, FrameHelper::RemoveChildFrame, 0);
		if (RwFrameGetParent(frame))
			RwFrameRemoveChild(frame);
		RpAtomicSetFrame(atomic, 0);
		FrameHelper::FrameDestroy(frame);
	}

	RpClump *clump = RpAtomicGetClump(atomic);
	if (clump)
		RpClumpRemoveAtomic(clump, atomic);

	RpWorld *world = RpAtomicGetWorld(atomic);
	if (world)
		RpWorldRemoveAtomic(world, atomic);

	return RpAtomicDestroy(atomic);
}

void SetIsVisible(RpAtomic &atomic, bool isVisible)
{
	SetFlag(atomic, rpATOMICRENDER, isVisible);
}

void SetCanCollide(RpAtomic &atomic, bool canCollide)
{
	SetFlag(atomic, rpATOMICCOLLISIONTEST, canCollide);
}

// 0x44e750
void HandleSystemCommands(RpAtomic &atomic, const CAttributePacket &attr)
{
	for (CAttributeCommandIterator it(attr, CSystemCommands::pCSystemCommandsClassID); !it.IsFinished(); ++it) {
		const CAttributeCommand &command = *it.operator->();
		unsigned int value;
		switch (command.GetCommandId()) {
		case CMD_LoadMatrix: {
			RwFrame *frame = RpAtomicGetFrame(&atomic);
			if (frame)
				CSystemCommands::UpdateFrame(*frame, command);
			break;
		}
		case CMD_SetSolidFlag:
			command.GetCommandData(value);
			RpAtomicCollisionProperties::SetIsSolid(atomic, value != 0);
			break;
		case CMD_SetInvisibleFlag:
			command.GetCommandData(value);
			SetIsVisible(atomic, value == 0);
			break;
		case CMD_SetCollisionFlag:
			command.GetCommandData(value);
			SetCanCollide(atomic, value != 0);
			break;
		default:
			break;
		}
	}
}

} // namespace AtomicHelper
} // namespace RWS
