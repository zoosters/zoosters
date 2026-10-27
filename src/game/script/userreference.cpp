// Script::UserReference: the "user" variable type (name, ref count, base type, members, position accessors).
#include "script/actor.h"
#include "script/runtime.h"
#include "script/ops_message.h"
#include "script/script.h"

namespace Script {

// 0x43c050
const char *UserReference::GetAltName()
{
	return "user";
}

// 0x43c030
int UserReference::AdjustRefCount(int delta)
{
	int &count = m_refCount;
	count += delta;
	if (count == 0) {
		delete this;
		return 0;
	}
	return count;
}

// 0x43ba90
ScriptReference *UserReference::GetBaseType()
{
	return zoosters::Opaque(&m_type);   // m_type carries the base type's vtable pointer: calls through it must dispatch (zoosters/base.h)
}

// 0x43bf30
void *UserReference::GetMembers(int *count)
{
	return GetBaseType()->GetMembers(count);
}

// 0x43c070
Vec3 *UserReference::GetVector(ScriptReference *owner)
{
	return (Vec3 *)&m_data;
}

// 0x43c080
void UserReference::SetVector(const Vec3 *v, ScriptReference *owner)
{
	*(Vec3 *)&m_data = *v;
}

}  // namespace Script

// 0x438950
Script::ScriptReference *Script::UserReference::GetType()
{
	return TFBType_User();
}

#include "script/ops_message.h"
using namespace Script;

ScriptReference *TFBType_Int();          // 0x42fdb0
ScriptReference *TFBType_Angle();        // 0x4302c0
ScriptReference *TFBType_Color();        // 0x42fdf0
ScriptReference *TFBType_TwoD();         // 0x42fe30
ScriptReference *TFBType_Position();     // 0x430300
void *MemAlloc(size_t n);                // 0x550170

// GLOBAL: 0x61f6c4 g_userRefSlots

// 0x43bf50
UserReference::~UserReference()
{
	for (int i = 0; i < MENUVAR_COUNT; i++) {
		if (g_userRefSlots[i] == this)
			g_userRefSlots[i] = 0;
	}
	if (m_object != 0 && m_object != this)
		m_object->AdjustRefCount(-1);
	else if (m_count != 0)
		operator_delete(m_data);
}

// 0x43ba80
const char *UserReference::GetName()
{
	return m_name;
}

// 0x43c060
int UserReference::GetValueType()
{
	return m_valueType;
}

// Bind the variable to an object (SetReferent(value, owner)): positions are copied by value (as a float vector), anything else is held
// by reference
// 0x43bbe0
void UserReference::SetReferent(ScriptReference *value, ScriptReference *owner)
{
	if (m_object != 0) {
		m_object->AdjustRefCount(-1);
		m_object = 0;
	}
	if (value != 0) {
		int posKind = value->IsKindOf(TFBType_Position());   // 1 a position, -1 derived from one
		if (posKind != 0) {
			ScriptReference *vectorOwner = owner != 0 ? owner : value;
			m_valueType = 0x10;
			*(Vec3 *)&m_data = *((PositionMeasurement *)value)->GetVector(vectorOwner);
			if (posKind > 0)
				return;
		}
		m_object = value;
		value->AdjustRefCount(1);
	}
}

// 2 while the variable is its own (scalar) object and its type is not a position, else the type's answer
// 0x43bc60
int UserReference::GetKind()
{
	if (m_object == this && !GetBaseType()->IsKindOf(TFBType_Position()))
		return 2;
	return GetBaseType()->GetKind();
}

// Near miss: same code, `this` lands in edi where the game uses esi (the bit-set tail then uses esi/edi swapped).
// 0x43be20 ?SetValue@UserReference@Script@@UAEXHPAVScriptReference@2@@Z
void UserReference::SetValue(int value, ScriptReference *owner)
{
	if (m_count == 0) {
		if (m_valueType == 0x10)
			m_valueType = 0;
		*(int *)&m_data = value;
		return;
	}
	if ((unsigned)m_index < (unsigned)m_count) {
		if (GetMax() != 1) {
			((int *)m_data)[m_index] = value;
			return;
		}
		unsigned idx = m_index;
		unsigned char *p = (unsigned char *)m_data + ((int)idx >> 3);
		int bit = idx & 7;
		unsigned char cur = *p;
		*p = (cur & ~(1 << bit)) | ((value != 0) << bit);
	}
}

// GLOBAL: 0x5ca5a8 g_floatZero
extern const float g_floatZero;
// Near-miss residual : the game builds the mask `1 << bit` (dl) before it loads the byte and ors at the end; ours loads first.
// 0x43bea0 ?SetValue@UserReference@Script@@UAEXMPAVScriptReference@2@@Z
void UserReference::SetValue(float value, ScriptReference *owner)
{
	if (m_count == 0) {
		m_valueType = 0x10;
		*(float *)&m_data = value;
		return;
	}
	if ((unsigned)m_index < (unsigned)m_count) {
		if (GetMax() != 1) {
			((float *)m_data)[m_index] = value;
			return;
		}
		int on = value != g_floatZero;
		int idx = m_index;
		int bit = idx & 7;
		unsigned char *p = (unsigned char *)m_data + (idx >> 3);
		unsigned char cur = *p;
		*p = (cur & ~(1 << bit)) | (on << bit);
	}
}


// A "user" variable: named, of a declared type, optionally an array (count elements; bit arrays for booleans)
// Near-miss residual : the game stores the EH state byte (state 1) after the four zero stores, copies the type vptr with the
// `ecx=type` load before `xor ebx,ebx`, and recomputes `lea ecx,[n*4]` for m_dataSize after MemAlloc instead of keeping it in ebp.
// 0x43c2a0 Script::UserReference::UserReference
UserReference::UserReference(const char *name, ScriptReference *type, intptr_t owner, int count)
	: m_refCount(0), m_name(name), m_type()
{
	*(void **)&m_type = *(void **)type;
	m_count = count;
	m_object = 0;
	m_valueType = 0;
	m_dataSize = 0;
	m_index = 0;
	m_data = 0;
	if (owner != 0)
		((ScriptReference *)this)->ResolveSymbol((NamedObject *)owner, name, type);
	// m_type carries the vtable pointer of `type` (above): its GetType must dispatch through it, not bind to ScriptReference::GetType (zoosters/base.h)
	ScriptReference *typeView = zoosters::Opaque(&m_type);
	bool isValue = typeView->GetType() == TFBType_Int() || typeView->GetType() == TFBType_Angle() ||
		typeView->GetType() == TFBType_Color() || typeView->GetType() == TFBType_TwoD() ||
		typeView->GetType() == TFBType_Position();
	if (!isValue && m_count == 0)
		return;
	m_object = this;
	if (isValue)
		m_valueType = ((ValueInterface *)zoosters::Opaque(&m_type))->GetValueType();
	if (m_count == 0)
		return;
	unsigned n;
	if (isValue && ((ValueInterface *)((ScriptReference *)this)->GetBaseType())->GetMax() == 1)
		n = ((unsigned)m_count + 31) >> 5;
	else
		n = m_count;
	int *p = (int *)MemAlloc(n * 4);
	m_data = p;
	m_dataSize = n << 2;
	while (n != 0) {
		n--;
		p[n] = 0;
	}
}

// The name hashes of the menu variables (ops_message.h MenuVariable)
static const int kMenuSelectionHash = 0x3e817d38;
static const int kMenuSelectionChangedHash = 0x419fbe2e;
static const int kMenuSliderHash = (int)0xe297e92c;
static const int kMenuToggleHash = (int)0xe702a588;

// Files a menu variable of `count` slots from `first` on: the first free one, else the last (a newer variable replaces it).
static void FileInSlots(Script::UserReference *variable, int first, int count)
{
	for (int i = first; i < first + count - 1; i++) {
		if (g_userRefSlots[i] == 0) {
			g_userRefSlots[i] = variable;
			return;
		}
	}
	g_userRefSlots[first + count - 1] = variable;
}

// 0x430e80
// A user variable the running script just created (OpContext::Instantiate): if its name is one of the menu variables the mouse
// handling works with, it is filed in its slot of g_userRefSlots. GetName returns the name hash for user variables.
void FileMenuVariable(Script::ScriptReference *object)
{
	Script::UserReference *variable = static_cast<Script::UserReference *>(object);
	uintptr_t hash = (uintptr_t)object->GetName();
	if (hash == (unsigned)kMenuSelectionHash)
		FileInSlots(variable, MENUVAR_SELECTION, 3);
	else if (hash == (unsigned)kMenuSelectionChangedHash)
		FileInSlots(variable, MENUVAR_SELECTION_CHANGED, 3);
	else if (hash == (unsigned)kMenuSliderHash)
		g_userRefSlots[MENUVAR_SLIDER] = variable;
	else if (hash == (unsigned)kMenuToggleHash)
		g_userRefSlots[MENUVAR_TOGGLE] = variable;
}

// 0x43bb60
ScriptReference *UserReference::Deref(ScriptReference *owner, int flags)
{
	if ((char)flags == 0)
		return m_object;
	return this;
}

// 0x43bf20
int UserReference::GetMax()
{
	return ((ValueInterface *)GetBaseType())->GetMax();
}

ScriptReference *TFBType_Int();          // 0x42fdb0
ScriptReference *TFBType_Camera();       // 0x4389d0
ScriptReference *TFBUser_Camera();         // 0x438a80

// 0x43c530
ScriptReference *TFBType_User()
{
	static UserReference s("user", TFBType_Int(), 0, 0);
	return &s;
}

// 0x438a80
ScriptReference *TFBUser_Camera()
{
	static UserReference s("camera", TFBType_Camera(), (intptr_t)TFBType_Camera(), 0);
	return &s;
}

// the camera object the "camera" variable names
// 0x438af0
ScriptReference *GetCameraObject()
{
	return TFBUser_Camera()->Deref(0, 0);
}

