#include <vector>

namespace Script { class ScriptReference; }

// 0x43c810
std::vector<Script::ScriptReference *> *TFB_GetTypeList()
{
	static std::vector<Script::ScriptReference *> v;
	return &v;
}

int SymbolRegistry_GetSize();   // 0x432cb0: number of entries in the symbol registry
// GLOBAL: 0x621f38 g_typeSymCount
extern int g_typeSymCount;

// Registers a type prototype in the global type list and remembers the symbol count after it
// 0x439bc0
void TFB_RegisterType(Script::ScriptReference *type)
{
	TFB_GetTypeList()->push_back(type);
	g_typeSymCount = SymbolRegistry_GetSize();
}

// The out-of-line vector<ScriptReference *> member templates the push_back above instantiates (identical code is folded across all
// pointer vectors in the game, hence the LinearMemoryPool names in some call targets).
// 0x4399e0
// 0x439970
// GLOBAL: 0x439700 ?_Ufill@?$vector@PAVScriptReference@Script@@V?$allocator@PAVScriptReference@Script@@@std@@@std@@IAEPAPAVScriptReference@Script@@PAPAV34@IABQAV34@@Z
// GLOBAL: 0x551580 ??$_Ucopy@PAPAVScriptReference@Script@@@?$vector@PAVScriptReference@Script@@V?$allocator@PAVScriptReference@Script@@@std@@@std@@IAEPAPAVScriptReference@Script@@PAPAV23@00@Z
// GLOBAL: 0x551550 ??$_Copy_backward_opt@PAPAVScriptReference@Script@@PAPAV12@@std@@YAPAPAVScriptReference@Script@@PAPAV12@00U_Scalar_ptr_iterator_tag@0@@Z
