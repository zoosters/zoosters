// Platform-specific pieces of the Win32 layer that game code cannot express in portable C++.
#ifndef ZOOSTERS_PLATFORM_H
#define ZOOSTERS_PLATFORM_H

// ZOOSTERS_JUMP_THUNK(name, slot): `void name` that only jumps through the function pointer stored at `slot` (an import address
// table entry or one of the game's file-I/O pointer slots), like the linker's import thunks. Callers reach it through a pointer
// table (g_ioFn), never directly. Only the cl 13.10 view has them (the byte-matching build); every other build gets an empty function,
// which nothing may call (FileIo_Reset stores the API functions themselves there), so no build references an import slot by name.
#define ZOOSTERS_JUMP_THUNK(name, slot) \
	void name() {}

#endif
