// Script::ScriptMessage: the Expose*Message base.
#include "script/ops_camera.h"
#include "script/types/scriptmessage.h"
using namespace Script;

namespace Script {
// 0x432c80
int ScriptMessage::AdjustRefCount(int delta)
{
	return -1;
}
}

namespace Script {

// 0x439d90
const char *ExposeHitWallMessage::GetName()
{
	return "hit wall";
}

// 0x439e50
const char *ExposeHitTriggerMessage::GetName()
{
	return "hit face trigger";
}

// 0x439f10
const char *ExposeWallNormalMessage::GetName()
{
	return "wall normal";
}

// 0x439fd0
const char *ExposeLedgeNearbyMessage::GetName()
{
	return "ledge nearby";
}

// 0x43a090
const char *ExposeFootPrintMessage::GetName()
{
	return "footprint";
}

// 0x43a150
const char *ExposeBounceNormalMessage::GetName()
{
	return "bounce normal";
}

// 0x43a210
const char *ExposeHitGroundMessage::GetName()
{
	return "hit ground";
}

// 0x43a2d0
const char *ExposeLeftGroundMessage::GetName()
{
	return "left ground";
}

// 0x43a390
const char *ExposeJumpStartedMessage::GetName()
{
	return "jump started";
}

// 0x43a450
const char *ExposeActivateMessage::GetName()
{
	return "activating";
}

// 0x43a510
const char *ExposeDeactivateMessage::GetName()
{
	return "deactivating";
}

// 0x43a5d0
const char *ExposeDeathMessage::GetName()
{
	return "dying";
}

// 0x43aff0
const char *ExposeHitActorMessage::GetName()
{
	return "hit actor";
}

}  // namespace Script

// 0x432c90
const char *ScriptMessage_GetName()
{
	return "message";
}
