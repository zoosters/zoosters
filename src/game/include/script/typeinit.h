// TypeInitHost: the static-initialiser host of the script type registry. Each of the game's TypeRegistry_Init static initialisers
// (src/data/normal/p4b_static_init.cpp) calls Init; the first call registers the built-in script types.
#ifndef ZOOSTERS_SCRIPT_TYPEINIT_H
#define ZOOSTERS_SCRIPT_TYPEINIT_H

struct TypeInitHost {
	TypeInitHost *Init();   // 0x43c870 (src/game/script/typeregistry_init.cpp)
};

#endif
