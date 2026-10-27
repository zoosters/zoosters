// TFB script object model: base classes. Class names from the PS2 build's RTTI; slot numbers and defaults from the PC vtables.
//
//   NamedObject (0x5ca418: 2 slots) > ScriptReference (0x5ca3ec: 11 slots) > OpCode (0x5cdc80: 15) > ops
//                                                                          > ValueInterface > Measurement ...
#ifndef ZOOSTERS_SCRIPT_SCRIPT_H
#define ZOOSTERS_SCRIPT_SCRIPT_H

#include "zoosters/base.h"   // size_t, ZOOSTERS_ASSERT_SIZE

// RwV3d-shaped vector (12 bytes, copied as a struct). Placeholder until the RW types are shared with the rest of the tree.
namespace Script { class ScriptReference; }
struct Vec3 {
	float x, y, z;
	bool LineOfSightTo(Script::ScriptReference *obj);  // 0x42d1f0 (ret 4): line of sight from this point to obj (CheckFOV)
};

// <new>'s placement operator new (MSVC 7.1). Declared here to avoid including <new> (and the toolkit's STL headers).
#if defined(_MSC_VER)
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void *operator new(size_t, void *p) { return p; }
#endif
#else
#include <new>
#endif

namespace Script {

class OpContext;

class NamedObject {
public:
	virtual const char *GetName() = 0;  // 0: _purecall here
	virtual const char *GetAltName();   // 1: 0x436a40, default calls GetName
};

// What GetKind (slot 6) reports: a plain value / measurement, an operation, a reference to a script object.
// NOTE: 0x43e640 (the ScriptSystem family) returns -1, which no enumerator names.
enum ScriptKind { SCRIPT_KIND_VALUE = 0, SCRIPT_KIND_OP = 1, SCRIPT_KIND_OBJECT = 2 };

// Slot roles. vf10 is not named yet.
class ScriptReference : public NamedObject {
public:
	const char *GetName();                   // 0: 0x43e6f0 (returns 0)
	virtual int AdjustRefCount(int delta);  // 2: default 0x416530 returns 0
	// 3: the virtual destructor (0x43b3d0 ~ScriptReference, also the folded ~OpCode; 0x4025c0 the scalar deleting destructor).
	// (Most files use a placeholder instead.)
	virtual ~ScriptReference() {}
	ScriptReference() {}
	ScriptReference(const ScriptReference &o);   // 0x403bb0 (out of line; ScriptSubset::ScriptSubset 0x43d710 copies the vptr)
	virtual ScriptReference *GetType();      // 4: the class's prototype instance (a function-local static)
	virtual ScriptReference *GetBaseType();  // 5: default 0
	virtual int GetKind();                   // 6: ScriptKind: OpCode 0x43efe0 returns 1, ScriptReference 0x42cbe0 returns 2, the measurements 0x43e6f0 return 0
	virtual void *GetMembers(int *count);    // 7: static member array; default sets *count = 0, returns 0
	virtual void SetReferent(ScriptReference *value, ScriptReference *owner);  // 8: store value as the referent (member of owner); default does nothing
	virtual ScriptReference *Deref(ScriptReference *owner, int flags);          // 9: the referent for owner; default returns this
	virtual void vf10();                     // 10: default does nothing
	int IsKindOf(ScriptReference *type);    // 0x43bab0 (not virtual): same type 1, base type -1, else 0
	// The script symbol registry. `this` is the object being registered (a type prototype, member or variable); `script` the
	// script (an OpContext) whose locals get the new object, or another named object (a type prototype: the registry itself;
	// null while the registry builds itself; the type getters pass their own prototype); `other` a second type that is part of
	// the name.
	const char *FormatTypedName(const char *name, ScriptReference *other);   // 0x432af0 (not virtual): "name::type[::othertype]"
	int ResolveSymbol(NamedObject *script, const char *name, ScriptReference *other);   // 0x434350 (not virtual; thiscall, ret 12)
};

}  // namespace Script

// Bump allocator for cloned script nodes (CloneToPool). Null while a script is only measured.
extern char *g_scriptPool;

#endif
