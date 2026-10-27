#include "zoosters/base.h"
#include "script/ops_camera.h"
#include "script/types/scriptmessage.h"
#include "script/types/scriptcamera.h"
#include "zoosters/memberrow.h"
using namespace Script;

struct OpRegistrarCb { void RegisterOps(); };   // 0x432390
// GLOBAL: 0x432390 ?RegisterOps@OpRegistrarCb@@QAEXXZ
// GLOBAL: 0x5cdc80 Script::OpCode::vftable

// Hand-written guard, as the game: store only the vptr (m_flags stays 0), run the registration pass once, atexit(0x5c7830).
// A compiler-generated static would call RegisterOps on every call and set m_flags = 1.
extern Script::OpCode g_opcodeProto;       // GLOBAL: 0x61f6e8 g_opcodeProto
extern unsigned g_opcodeProtoGuard;        // GLOBAL: 0x61f6f0 g_opcodeProtoGuard
typedef void (__cdecl *AtExitFn)();
extern "C" {
int GameAtExit(AtExitFn fn);               // 0x558aed, the game's CRT atexit
}
// GLOBAL: 0x558aed ?GameAtExit@@YAHP6AXXZ@Z
// 0x4310c0 TFBType_OpCode
ScriptReference *TFBType_OpCode()
{
	if (!(g_opcodeProtoGuard & 1)) {
		g_opcodeProtoGuard |= 1;
		*(void **)&g_opcodeProto = zoosters::VptrOf<OpCode>();   // vptr only: the OpCode constructor would set m_flags = 1
		OpRegistrarCb reg;
		reg.RegisterOps();
		GameAtExit((AtExitFn)0x5c7830);
	}
	return &g_opcodeProto;
}

struct OpRegistrar { void RegisterOps(); };   // 0x432390
inline void OpRegistrarCb::RegisterOps() { ((OpRegistrar *)this)->RegisterOps(); }
