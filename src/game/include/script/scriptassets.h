// Script::AssetEntry: one loaded script asset (table g_scriptAssets 0x618390, 0x320 entries). Shared so the entry stride is the same
// everywhere (an OpContext holds pointers: the entry is not 0x1c bytes on x64).
#ifndef ZOOSTERS_SCRIPT_SCRIPTASSETS_H
#define ZOOSTERS_SCRIPT_SCRIPTASSETS_H
#include "zoosters/base.h"
#include "script/runtime.h"

namespace Script {

// the loaded script of one asset: key, size, the OpContext parsed from it (+8..+0x23)
struct AssetEntry {
	const void *m_data;    // resource data
	unsigned m_size;       // resource size (CResourceManager::FindById)
	OpContext m_ctx;  // the script parsed from the data (m_nodes 0 until ScriptAssets_Release loads it)
};
ZOOSTERS_ASSERT_SIZE(AssetEntry, 0x24);
ZOOSTERS_ASSERT_OFFSET(AssetEntry, m_size, 0x04);
ZOOSTERS_ASSERT_OFFSET(AssetEntry, m_ctx, 0x08);   // opcontexts.cpp: the game build walks g_table428 = g_scriptAssets + 8 with this stride

}  // namespace Script

#endif
