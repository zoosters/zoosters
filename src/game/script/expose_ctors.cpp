// Vptr-only constructors of the Expose* members. The class declares the constructor and every override of its game vtable;
// ValueInterface/Measurement carry GetName/GetType and NOVTABLE so intermediate levels need no game address.
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "script/types/actorturret.h"

namespace Script {

// GLOBAL: 0x5cdad8 Script::ActorTurret::ExposeFOV::vftable
// 0x42df50
ActorTurret::ExposeFOV::ExposeFOV() {}

}  // namespace Script
