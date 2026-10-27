// The functions the game's linker folded into one shared body (ICF: 0x464ef0 `ret`, 0x43e6f0 `xor eax,eax; ret`), which call sites declare under
// their own names and signatures: each name gets its own body here. See the GLOBAL lines at the declarations.
namespace RWS { namespace FX { int FUN_0043e6f0(const char *msg) { return 0; } } }
void Fold_Nop() {}                             // 0x464ef0 (an empty destructor call / empty function of the game's source)
void CEventIdDtorBare() {}                     // 0x464ef0
int Fold_Return0() { return 0; }               // 0x43e6f0 (a RenderWare plugin size callback)
int FUN_0043e6f0(const char *) { return 0; }   // 0x43e6f0 (the release build's debug print, empty)
int FUN_0043e6f0(const char *, ...) { return 0; }
void *FUN_0043e6f0() { return 0; }             // 0x43e6f0 (the creator of the shadow caster pipeline in the game's source)
void operator_delete_unwind(void *pMem, unsigned type);   // 0x5500e0 (framework/core/memoryhandler.cpp)
void operator_delete_unwind(void *p) { operator_delete_unwind(p, 0); }   // the unwind funclets pass the block only (the type word is unused by the body)
