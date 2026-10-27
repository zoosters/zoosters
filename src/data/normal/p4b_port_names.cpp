// Data the normal build needs under the port-side names, plus a few typed globals. Normal build only (ZOOSTERS_NORMAL_BUILD); the
// cl 13.10 and address-binding paths see an empty file. Nonzero or constructed globals: g_radarAspect, g_defMemFuncs, g_opcodeProto,
// g_typeInfoVftable, g_ioFn and the two file slots.
#include <stddef.h>
#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "script/opcode.h"
#include "framework/startup/win32/fileio.h"

// ---- data
extern const float g_radarAspect = 0.75f;      // GLOBAL: 0x5caea4 (one word 0.75f; read by tfb/radarrender)

// GLOBAL: 0x5dc164 g_typeInfoVftable: the vtable pointer in the game's type_info descriptors (g_td* of src/data/g/data_run.cpp); the normal
// build has no use for the descriptors, they only need the symbol
const void *g_typeInfoVftable[1] = { 0 };

// GLOBAL: 0x606c90 g_defMemFuncs: the default RenderWare memory functions (MemAllocHint, operator_delete, MemRealloc, MemCalloc;
// the following 36 B at 0x606ca0 are another table, not this struct)
void *MemAllocHint(size_t size, int hint);
void operator_delete(void *p);
void *MemRealloc(void *p, size_t size);
void *MemCalloc(size_t n, size_t size, int hint);
RwMemoryFunctions g_defMemFuncs = {
	(void *(*)(size_t, RwUInt32))&MemAllocHint, (void (*)(void *))&operator_delete,
	(void *(*)(void *, size_t, RwUInt32))&MemRealloc, (void *(*)(size_t, size_t, RwUInt32))&MemCalloc };   // the game's MemRealloc ignores the hint

// GLOBAL: 0x61f6e8 g_opcodeProto: the static OpCode prototype. The game's is zero data whose vptr TFBType_OpCode stores; here the compiler
// constructs it (vptr) and the generated static-init list resets m_flags to the game's 0.
Script::OpCode g_opcodeProto;

// GLOBAL: 0x60d818 g_ioFn: CreateFile / ReadFile / ReadFileEx / WriteFile slots (FileIo_Reset, run first by the static-init list, stores the
// savefile.cpp calls two of them by their own names: separate variables, since C++ cannot name an element of g_ioFn. They are copied
// from g_ioFn by the static-init list and stay equal.
void *g_ioFn[4] = { 0, 0, 0, 0 };
FileCreateFn g_pfnCreateFile = 0;   // 0x60d818 (= g_ioFn[0])
FileRwFn g_pfnFileRead = 0;         // 0x60d81c (= g_ioFn[1])
