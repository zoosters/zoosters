// Script::ScriptMedia (vtable 0x5cca08): the script type of media objects (movies); only its name and type getter so far.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTMEDIA_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTMEDIA_H

#include "zoosters/base.h"
#include "script/script.h"

namespace Script {

class ScriptMedia : public ScriptReference {
public:
	const char *GetName();
	ScriptReference *GetType();                 // 4: 0x418b70 (thunk to TFBType_Media)
	virtual void vf11(void *a);    // 11: 0x410380 (the shared empty function, ret 4)
	virtual void vf12(void *a);    // 12: 0x410380
};

}  // namespace Script

#endif
