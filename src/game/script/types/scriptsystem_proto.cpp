// ScriptSystem prototype getter (0x43fa20).
#include "script/types/scriptsystem.h"

namespace Script {

ScriptReference *ScriptSystem_RegisterMembers();   // 0x43f890 (scriptsystem_register.cpp)

class ScriptSystemP : public ScriptSystem {
public:
	ScriptSystemP() {}
};

}

using namespace Script;

// 0x43fa20
Script::ScriptReference *TFBType_System()
{
	static ScriptSystemP s;
	ZOOSTERS_PROTO_REGISTER_AFTER(ScriptSystem_RegisterMembers());
	return (Script::ScriptReference *)&s;
}
