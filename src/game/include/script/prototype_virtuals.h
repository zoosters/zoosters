// Script prototype classes whose constructor body calls out (type / member registration). The game's EH table for their static
// getter has ONE state (the static's guard bit), which only comes out when the class has no base or member with a destructor.
// So these are standalone and declare the whole 11-slot ScriptReference vtable; the virtual destructor (slot 3) is
// ~ScriptReference 0x43b3d0, aliased with a `// GLOBAL: 0x43b3d0 ??1X@Script@@UAE@XZ` line.
// Never include together with script/script.h.
#ifndef ZOOSTERS_SCRIPT_PROTOTYPE_VIRTUALS_H
#define ZOOSTERS_SCRIPT_PROTOTYPE_VIRTUALS_H

namespace Script { class ScriptReference; }

#define PROTOTYPE_VIRTUALS(X) \
	virtual const char *GetName();            /* 0 */ \
	virtual const char *GetAltName();         /* 1: 0x436a40 */ \
	virtual int AdjustRefCount(int delta);    /* 2: 0x416530 */ \
	virtual ~X();                             /* 3 */ \
	virtual void *GetType();                  /* 4 */ \
	virtual void *GetBaseType();              /* 5 */ \
	virtual int GetKind();                    /* 6 */ \
	virtual void *GetMembers(int *count);     /* 7 */ \
	virtual void SetReferent(Script::ScriptReference *value, Script::ScriptReference *owner);   /* 8 */ \
	virtual void *Deref(Script::ScriptReference *owner, int flags);   /* 9 */ \
	virtual void vf10();                      /* 10 */

#endif
