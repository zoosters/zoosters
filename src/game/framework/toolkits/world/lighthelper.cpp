#include "framework/toolkits/world/lighthelper.h"
#include "framework/toolkits/world/atomichelper.h"
#include "framework/toolkits/world/framehelper.h"
#include "framework/core/csystemcommands.h"

namespace RWS {
namespace LightHelper {

// 0x44f060
RwBool Destroy(RpLight *light)
{
	RwFrame *frame = RpLightGetFrame(light);
	if (frame) {
		RwFrameForAllChildren(frame, FrameHelper::RemoveChildFrame, 0);
		if (RwFrameGetParent(frame))
			RwFrameRemoveChild(frame);
		RpLightSetFrame(light, 0);
		FrameHelper::FrameDestroy(frame);
	}

	RpWorld *world = RpLightGetWorld(light);
	if (world)
		RpWorldRemoveLight(world, light);

	return RpLightDestroy(light);
}

// 0x44f0d0
void HandleSystemCommands(RpLight &light, const CAttributePacket &attr)
{
	for (CAttributeCommandIterator it(attr, CSystemCommands::pCSystemCommandsClassID); !it.IsFinished(); ++it) {
		if (it->GetCommandId() != CMD_LoadMatrix)
			continue;
		RwFrame *frame = RpLightGetFrame(&light);
		if (frame)
			CSystemCommands::UpdateFrame(*frame, *it.operator->());
	}
}

} // namespace LightHelper
} // namespace RWS
