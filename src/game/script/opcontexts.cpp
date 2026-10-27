#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include <math.h>
// table of 0x24-byte entries (0x618398), count 0x618378: reset all, then empty it
// GLOBAL: 0x432bc0 ?FreeNodes@TableEntry428@@QAEXXZ
struct TableEntry428 { char m_pad[0x24]; void FreeNodes(); };   // 0x432bc0 = Script::OpContext::FreeNodes
// GLOBAL: 0x618378 Script::g_table428Count
// GLOBAL: 0x618398 g_table428
namespace Script { extern unsigned g_table428Count; }
// g_table428 (0x618398) is g_scriptAssets (0x618390) seen from its +8 member (the OpContext of every 0x24 B asset entry); two
// separately defined objects would not alias, so the view is computed
#include "script/scriptassets.h"
namespace Script { extern AssetEntry g_scriptAssets[]; }

// 0x42b150
void ResetOpContexts()
{
	for (unsigned i = 0; i < Script::g_table428Count; i++)
		Script::g_scriptAssets[i].m_ctx.FreeNodes();     // the stride of the real entry (x64: not 0x24)
	Script::g_table428Count = 0;
}

namespace Script {

// 0x429740
const char *OpContext::GetAltName()
{
	return "TFB Script";
}

// 0x429750
const char *OpContext::GetName()
{
	return "script";
}

}  // namespace Script
