// Tiny game functions that game data points at (the data blocks copied from Game.exe reference them by address), named FUN_<addr>
// for the linker's alias rule.
void __stdcall FUN_00410380(int) {}     // 0x410380: `ret 4`, the shared empty __stdcall(int) (COMDAT-folded; TFBStreamPump's initial value)

// 0x5c5e40: static initialiser of the script-asset table (Script::g_scriptAssets, 0x320 entries of 0x24 bytes: resource, size,
// embedded OpContext whose vtable 0x5cd2bc sits at +8). The game's thunk is a loop the linker's thunk decoder cannot parse.
// OpContext is a real polymorphic class here, so its implicit constructor writes the vptr. Called by the generated static-init
// list (src/data/normal/p4b_static_init.cpp).
#include <new>
#include <string.h>
#include "script/runtime.h"
#include "script/scriptassets.h"
namespace Script { extern AssetEntry g_scriptAssets[]; }
extern "C" void port_script_assets_init(void)
{
	for (int i = 0; i < 0x320; i++) {
		Script::AssetEntry *e = &Script::g_scriptAssets[i];
		memset((void *)e, 0, sizeof(*e));   // the entry is 0x24 bytes on x86, more on x64 (the OpContext holds pointers)
		::new ((void *)&e->m_ctx) Script::OpContext;
	}
}
