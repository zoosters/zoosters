// Scalar deleting destructors of Script::Placement and of the folded copy 0x403d20 the game shares between ScriptRumble / ScriptLight / GameLight.
// Constructors are not written: stand-in factory functions.
#include "script/actor.h"

namespace Script {

// 0x402c00
Placement::~Placement()
{
}

// GLOBAL: 0x5ca7d8 Script::Placement::vftable
// 0x403a30
Placement *g_makePlacement() { return new Placement; }

// Stand-in for the classes that share 0x403d20 (their ~X is the jmp thunk 0x414790 to ~Placement 0x402c00).
// NOVTABLE: Script::PlacementSharedDtor
class PlacementSharedDtor : public Placement {
public:
	~PlacementSharedDtor();
};
// 0x403d20
// GLOBAL: 0x414790 ??1PlacementSharedDtor@Script@@UAE@XZ
Placement *g_makeSharedDtor() { return new PlacementSharedDtor; }

}


// FOLDED: 0x414790 Script::PlacementSharedDtor::~PlacementSharedDtor
Script::PlacementSharedDtor::~PlacementSharedDtor() {}   // the game folded it into GnThis_rest::Placement400_Dtor_thunk
