#include "script/script.h"
#include "script/script.h"
#include "script/types/scriptcamera.h"
using namespace Script;

Script::ScriptReference *TFBType_Camera();       // 0x4389d0
Script::ScriptReference *TFBMember_Camera_CameraLocation();        // 0x439320
Script::ScriptReference *OpProto_CameraUse();        // 0x438970

namespace Script {

class CameraInit {
public:
	CameraInit() {}
	void Run();       // 0x439360
};

// The prototype is a real ScriptCamera. The game's constructor leaves everything behind +0x3c zero (static storage); ScriptCamera
// sets m_horzFollow = 30, and Reset (CameraInit::Run, vf10) sets it to 30 again.
class CameraProto : public ScriptCamera {
public:
	CameraProto() { m_horzFollow = 0; }
};
// 0x439360
void CameraInit::Run()
{
	((ScriptReference *)TFBMember_Camera_CameraLocation())->ResolveSymbol(::TFBType_Camera(), 0, 0);
	((ScriptReference *)::TFBType_Camera())->ResolveSymbol(::TFBType_Camera(), "DEFAULT", 0);
	((ScriptReference *)OpProto_CameraUse())->ResolveSymbol(::TFBType_Camera(), 0, 0);
	((CameraProto *)::TFBType_Camera())->vf10();
}

}  // namespace Script

// 0x4389d0
Script::ScriptReference *TFBType_Camera()
{
	static CameraProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(CameraInit().Run());
	return (Script::ScriptReference *)&s;
}
