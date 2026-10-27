// Script value ops: inc/dec/set/check/loop/slide.
#include "script/ops_value.h"
#include "script/value.h"
#include <float.h>

// GLOBAL: 0x602f98 g_compareOpNames
extern const char *g_compareOpNames[7];
#ifdef ZOOSTERS_DEBUGUI
void port_dbg_value_check_begin(void *op);
void port_dbg_value_check_end(void *op, float lhs, int compare, float rhs, bool result);
#endif

// GLOBAL: 0x42f7b0 Script::SlideEntry::SlideEntry

// 0x432990: 5-byte jump to TFBRead_u32 (ValueSlide::Parse reads its two u32 members through it; same as ops_camera.h)
void TFBRead_u32_thunk(unsigned *out);

namespace Script {

// 0x42f970
const char *ValueInc::GetName()
{
	return "inc value";
}

// 0x4303f0
OpCode *ValueInc::Clone()
{
	return CloneToPool(this);
}

// 0x430400
ValueInc *ValueInc::CloneToPool(const ValueInc *src)
{
	void *mem = g_scriptPool;
	ValueInc *r = new (mem) ValueInc(*src);
	g_scriptPool += sizeof(ValueInc);
	return r;
}

// 0x42fa60
const char *ValueDec::GetName()
{
	return "dec value";
}

// 0x430440
OpCode *ValueDec::Clone()
{
	return CloneToPool(this);
}

// 0x430450
ValueDec *ValueDec::CloneToPool(const ValueDec *src)
{
	void *mem = g_scriptPool;
	ValueDec *r = new (mem) ValueDec(*src);
	g_scriptPool += sizeof(ValueDec);
	return r;
}

// 0x42fb20
const char *ValueSet::GetName()
{
	return "set value";
}

// 0x430490
OpCode *ValueSet::Clone()
{
	return CloneToPool(this);
}

// 0x4304a0
ValueSet *ValueSet::CloneToPool(const ValueSet *src)
{
	void *mem = g_scriptPool;
	ValueSet *r = new (mem) ValueSet(*src);
	g_scriptPool += sizeof(ValueSet);
	return r;
}

// 0x42fba0
const char *ValueCheck::GetName()
{
	return "check value";
}

// 0x430b00
OpCode *ValueCheck::Clone()
{
	return CloneToPool(this);
}

// 0x430b10
ValueCheck *ValueCheck::CloneToPool(const ValueCheck *src)
{
	void *mem = g_scriptPool;
	ValueCheck *r = new (mem) ValueCheck(*src);
	g_scriptPool += sizeof(ValueCheck);
	return r;
}

// 0x42fbb0
const char *ValueLoop::GetName()
{
	return "loop value";
}

// 0x42fbc0
int ValueLoop::IsBlock()
{
	return 100;
}

// 0x430b90
OpCode *ValueLoop::Clone()
{
	return CloneToPool(this);
}

// 0x430ba0
ValueLoop *ValueLoop::CloneToPool(const ValueLoop *src)
{
	void *mem = g_scriptPool;
	ValueLoop *r = new (mem) ValueLoop(*src);
	g_scriptPool += sizeof(ValueLoop);
	return r;
}

// 0x42fd80
const char *ValueSlide::GetName()
{
	return "slide value";
}

// 0x430d40
OpCode *ValueSlide::Clone()
{
	return CloneToPool(this);
}

// 0x464ef0 (a bare ret, folded with other empty functions): not annotated, a 5-byte hook does not fit a 1-byte
void ValueSlide::operator delete(void *, void *)
{
}

// 0x430db0
ValueSlide::ValueSlide(const ValueSlide &o)
	: OpBlock(o), ValueSlideEhTag(), m_target(o.m_target), m_value(o.m_value), m_seconds(o.m_seconds), m_easeOut(o.m_easeOut), m_easeIn(o.m_easeIn), m_slides(o.m_slides)
{
}

// 0x430d50
ValueSlide *ValueSlide::CloneToPool(const ValueSlide *src)
{
	ValueSlide *r = new (g_scriptPool) ValueSlide(*src);
	g_scriptPool += sizeof(ValueSlide);
	return r;
}

// 0x42fec0
int ValueInc::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Int());
	refs.Read(&m_target);
	((ValueInterface *)g_scriptLastType)->CheckParsedType();
	return sizeof(ValueInc);
}

// 0x42f980
int ValueInc::Execute()
{
	ScriptReference *owner;
	ScriptReference *set;
	Measurement *m = (Measurement *)m_target.Resolve(&owner, &set, true);
	if (m != 0) {
		if (m->GetValueType() == 0) {
			int i = m->GetInt(owner);
			int max = m->GetMax();
			if (i++ < max || max == 0) {
				m->SetValue(i, owner);
			}
		} else {
			float v = m->GetFloat(owner);
			float max = (float)m->GetMax();
			v = v + 1.0f;
			if (v > max && max != 0.0f) {
				v = max;
			}
			m->SetValue(v, owner);
		}
	}
	return 1;
}

// 0x42fa70
int ValueDec::Execute()
{
	ScriptReference *owner;
	ScriptReference *set;
	Measurement *m = (Measurement *)m_target.Resolve(&owner, &set, true);
	if (m != 0) {
		if (m->GetValueType() == 0) {
			int i = m->GetInt(owner);
			if (i-- != 0 || m->GetMax() == 0) {
				m->SetValue(i, owner);
			}
		} else {
			float v = m->GetFloat(owner);
			v = v - 1.0f;
			if (v < 0.0f && m->GetMax() != 0) {
				v = 0.0f;
			}
			m->SetValue(v, owner);
		}
	}
	return 1;
}

// 0x42ff10
int ValueDec::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Int());
	refs.Read(&m_target);
	((ValueInterface *)g_scriptLastType)->GetValueType();
	return sizeof(ValueDec);
}

// 0x42fc00
// (shared with Script::SetFindSub::Parse)
int ValueCheck::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Value());
	refs.Read(&m_target);
	ScriptEnumReader en(&m_value.opArg, g_compareOpNames, 0);
	ScriptValueReader(&m_value, g_scriptLastType, 0, 4);
	return sizeof(ValueCheck);
}

// 0x430580
int ValueCheck::Execute()
{
	ScriptReference *owner;
	ScriptReference *set;
	float rhs;
	Measurement *m = (Measurement *)m_target.Resolve(&owner, &set, true);
	bool ok = false;
#ifdef ZOOSTERS_DEBUGUI   // debug overlay only (port/debugui/debugui_triggers.cpp)
	port_dbg_value_check_begin(this);
	if (m != 0 && m_value.EvalFloat(&rhs)) {
		float lhs = m->GetFloat(owner);
		ok = TFB_CompareFloat(lhs, m_value.opArg, rhs);
		port_dbg_value_check_end(this, lhs, m_value.opArg, rhs, ok);
	}
#else
	if (m != 0 && m_value.EvalFloat(&rhs) && TFB_CompareFloat(m->GetFloat(owner), m_value.opArg, rhs)) {
		ok = true;
	}
#endif
	if (!ok) {
		SkipBlock();
		return 1;
	}
	return OpCode::Execute();
}

// 0x42fbd0
int ValueLoop::Parse()
{
	GetName();
	ScriptValueReader(&m_times, 0, "times", 4);
	return sizeof(ValueLoop);
}

// 0x430500
int ValueLoop::Execute()
{
	int count;
	bool ok = m_times.EvalInt(&count);
	int n = count;
	if (!ok) {
		n = 0;
	}
	count = g_scriptPC;
	for (;;) {
		if (n != 0) {
			int r = OpCode::Execute();
			if (r != 1) {
				return r;
			}
			if (!(m_flags & 0x80)) {
				return 1;
			}
			if (n-- == 0) {
				return 1;
			}
			g_scriptPC = count;
			continue;
		}
		SkipBlock();
		return 1;
	}
}

// 0x430210
int ValueSlide::Parse()
{
	SlideParseTag tag;
	GetName();
	ScriptRefReader refs(TFBType_Value());
	refs.Read(&m_target);
	ScriptValueReader(&m_value, g_scriptLastType, 0, 4), m_value.opArg = 1, ScriptValueReader(&m_seconds, 0, "seconds", 4);
	TFBRead_u32_thunk(&m_easeIn);
	TFBRead_u32_thunk(&m_easeOut);
	return sizeof(ValueSlide);
}

// 0x430820
int ValueSlide::Execute()
{
	int targetBits;
	int vtype;
	float targetFloat;
	ScriptReference *owner;
	float seconds;
	union {
		float f;
		int i;
	} cur;
	ScriptReference *set;
	Measurement *m = (Measurement *)m_target.Resolve(&owner, &set, true);
	if (m != 0) {
		ScriptValue *sv = &m_seconds;
		if (sv->EvalFloat(&seconds)) {
			if (seconds == 0.0f) {
				m_value.ApplyTo(&m_target);
			} else {
				switch (m_value.Evaluate((float *)&targetBits, m->GetMax())) {
				case 0:
					targetFloat = (float)targetBits;
					vtype = 0;
					break;
				case 1:
					targetFloat = *(float *)&targetBits;
					vtype = 0x10;
					break;
				}
				if (m->GetType() != TFBType_User()) {
					if (vtype == 0x10) {
						if (m->GetValueType() != 0x10) {
							targetBits = (int)targetFloat;
							vtype = 0;
						}
					} else if (m->GetValueType() == 0x10) {
						*(float *)&targetBits = targetFloat;
						vtype = 0x10;
					}
				}
				bool same;
				if (m->GetValueType() <= 0x10) {
					cur.f = m->GetFloat(owner);
					same = TFB_CompareFloat(cur.f, 1, targetFloat);
				} else {
					cur.i = m->GetInt(owner);
					same = cur.i == targetBits;
				}
				if (same) {
					m_value.ApplyTo(&m_target);
				} else {
					bool found = false;
					for (SetNode *n = m_slides.m_head; n != 0; n = n->m_next) {
						SlideEntry *entry = (SlideEntry *)n->m_object;
						if (m == entry->m_measurement && owner == entry->m_owner) {
							if (vtype == 0x10) {
								if (*(float *)&targetBits == *(float *)&entry->m_target) {
									found = true;
									break;
								}
							} else if (targetBits == entry->m_target) {
								found = true;
								break;
							}
						}
					}
					if (!found) {
						SlideEntry *entry = new SlideEntry(m, owner, &targetBits, seconds, vtype);
						if (entry != 0) {
							entry->m_current = cur.i;
							entry->m_start = cur.i;
							if (m->GetValueType() <= 0x10) {
								entry->m_end = *(int *)&targetFloat;
							} else {
								entry->m_end = targetBits;
							}
							if (m_slides.m_count == 0) {
								GetSlideSet()->Insert(this, FLT_MAX);
							}
							m_slides.Insert(entry, FLT_MAX);
						}
					}
					SkipBlock();
					return 1;
				}
			}
		}
	}
	return OpCode::Execute();
}

// 0x42fb30
int ValueSet::Execute()
{
	m_value.ApplyTo(&m_target);
	return 1;
}

// 0x42fb50
int ValueSet::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Value());
	refs.Read(&m_target);
	ScriptValueReader(&m_value, g_scriptLastType, 0, 4);
	return sizeof(ValueSet);
}

}  // namespace Script
