// Own TU: <vector> and the script headers both define placement new.
#include <vector>

#include "script/types/scriptcontroller.h"
#include "script/script.h"
using namespace Script;
// GLOBAL: 0x439460 TFB_GetPlayerRefs
ControllerPlayerRef *TFB_GetPlayerRefs();

// The prototype objects are function-local statics whose class has no base with a destructor; see typeprotos_set_message.cpp
#define PROTO_SLOTS(C) 	virtual const char *GetName(); 	virtual const char *GetAltName(); 	virtual int AdjustRefCount(int delta); 	virtual ~C(); 	virtual void *GetType(); 	virtual void *GetBaseType(); 	virtual int GetKind(); 	virtual void *GetMembers(int *count); 	virtual void SetReferent(Script::ScriptReference *, Script::ScriptReference *); 	virtual void *Deref(Script::ScriptReference *, int); 	virtual void vf10();

Script::ScriptReference *TFBType_Controller();       // 0x4393d0
Script::ScriptReference *TFBMember_Controller_ActiveControllers();             // 0x439730 "active controllers" member prototype
Script::ScriptReference *TFBType_Rumble();             // 0x439770 the controller rumble prototype
std::vector<Script::ScriptReference *> *TFB_GetTypeList();   // 0x43c810 the global type list
int SymbolRegistry_GetSize();                          // 0x432cb0 number of symbol registry entries
// GLOBAL: 0x621f38 g_typeSymCount
extern int g_typeSymCount;


// Registers the controller type's members and the player references, and adds the rumble prototype to the type list
class ControllerInit {
public:
	ControllerInit() {}
	void Run();       // 0x439c30
};

// 0x439c30
void ControllerInit::Run()
{
	((Script::ScriptReference *)TFBMember_Controller_ActiveControllers())->ResolveSymbol(TFBType_Controller(), 0, 0);
	ControllerPlayerRef *p = TFB_GetPlayerRefs();
	int i = 5;
	if (p != 0) {
		do {
			((Script::ScriptReference *)p)->ResolveSymbol(TFBType_Controller(), 0, 0);
			p++;
		} while (--i != 0);
	}
	Script::ScriptReference *rumble = TFBType_Rumble();
	TFB_GetTypeList()->push_back(rumble);
	g_typeSymCount = SymbolRegistry_GetSize();
	((Script::ScriptReference *)TFBType_Rumble())->ResolveSymbol(TFBType_Controller(), 0, 0);
}

// GLOBAL: 0x5d1190 ControllerProto::vftable
// GLOBAL: 0x43b3d0 ??1ControllerProto@@UAE@XZ
class ControllerProto : public Script::ScriptController {
public:
	__forceinline ControllerProto() { }
};

// 0x4393d0
Script::ScriptReference *TFBType_Controller()
{
	static ControllerProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(ControllerInit().Run());
	return (Script::ScriptReference *)&s;
}
