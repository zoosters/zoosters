#define ZOOSTERS_RWS_SENDMSG 1
#include "framework/toolkits/world/clumphelper.h"
#include "framework/toolkits/world/atomichelper.h"
#include "framework/core/csystemcommands.h"
#include "tfb/render/atomicflags.h"

RpAtomic *AtomicRemoveFromWorldCB(RpAtomic *atomic, void *data);

namespace RWS {
namespace ClumpHelper {

extern CEventId iMsgPreRpClumpDestroy;

// 0x44ee00
RpAtomic *GiveAtomicEventHandler(RpAtomic *atomic, void *handler)
{
	RpAtomicToEventHandler::SetEventHandler(*atomic, static_cast<CEventHandler *>(handler));
	return atomic;
}

// 0x44eb20
void Open()
{
	CEventHandler::RegisterMsg(iMsgPreRpClumpDestroy, "iMsgPreRpClumpDestroy", "RwFrame*");
}

// 0x44eb40
void Close()
{
	CEventHandler::UnRegisterMsg(iMsgPreRpClumpDestroy);
}

// 0x44eb70
RwBool Destroy(RpClump *clump)
{
	RwFrame *frame = RpClumpGetFrame(clump);
	CMsg msg(iMsgPreRpClumpDestroy, frame);
	SendMsg(msg);

	if (frame && RwFrameGetParent(frame))
		RwFrameRemoveChild(frame);

	if (RpWorld *world = RpClumpGetWorld(clump)) {
		RpWorldRemoveClump(world, clump);
	} else if (clump->inWorldLink.next) {
		// threaded on a world's clump list without a world pointer: unlink by hand, leave the clump's own links
		RwLLLink *next = clump->inWorldLink.next;
		RwLLLink *prev = clump->inWorldLink.prev;
		prev->next = next;
		next->prev = prev;
	} else {
		RpClumpForAllAtomics(clump, AtomicRemoveFromWorldCB, 0);
	}

	return RpClumpDestroy(clump);
}

// 0x44ee20
void SetEventHandler(RpClump &clump, CEventHandler *handler)
{
	RpClumpForAllAtomics(&clump, GiveAtomicEventHandler, handler);
}

// 0x44eed0
void HandleSystemCommands(RpClump &clump, const CAttributePacket &attributes)
{
	for (CAttributeCommandIterator it(attributes, CSystemCommands::pCSystemCommandsClassID); !it.IsFinished(); ++it) {
		const CAttributeCommand &command = *it.operator->();
		RwUInt32 word;
		switch (command.GetCommandId()) {
		case CMD_LoadMatrix:
			if (RwFrame *frame = RpClumpGetFrame(&clump))
				CSystemCommands::UpdateFrame(*frame, command);
			break;
		case CMD_SetSolidFlag:
			command.GetCommandData(word);
			RpAtomicCollisionProperties::SetIsSolid(clump, word != 0);
			break;
		case CMD_SetInvisibleFlag:
			command.GetCommandData(word);
			ClumpSetIsVisible(&clump, word == 0, -1.0f, -1.0f);
			break;
		case CMD_SetCollisionFlag: {
			command.GetCommandData(word);
			void *canCollide = word ? &clump : nullptr;   // only tested for null
			RpClumpForAllAtomics(&clump, ClumpSetCanCollideCB, canCollide);
			break;
		}
		default:
			break;
		}
	}
}

} // namespace ClumpHelper
} // namespace RWS
