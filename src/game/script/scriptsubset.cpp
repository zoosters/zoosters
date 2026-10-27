#include "script/ops_set.h"
namespace Script {

// GLOBAL: 0x602fb8 Script::g_subsetTableSize
// GLOBAL: 0x61f660 Script::g_subsetTable

// 0x42cbc0
int ScriptSubset::AdjustRefCount(int delta)
{
	m_refCount += delta;
	if (m_refCount == 0) {
		delete this;
		return 0;
	}
	return m_refCount;
}

// 0x42cbf0
ScriptReference *ScriptSubset::GetBaseType()
{
	return zoosters::Opaque(&m_type);   // m_type carries the source's base-type vtable pointer: calls through it must dispatch (zoosters/base.h)
}

// 0x43cfe0
ScriptSubset::~ScriptSubset()
{
	for (int i = 0; i < g_subsetTableSize; i++) {
		if (this == g_subsetTable[i]) {
			g_subsetTable[i] = 0;
			break;
		}
	}
}

// m_type is initialised by the user copy constructor ScriptReference(const ScriptReference &) (copies the vptr).
// 0x43d710
ScriptSubset::ScriptSubset(ReferenceSet *src, int randomKeys)
	: m_name(src->GetName()), m_type(*src->GetBaseType())
{
	Assign(src, (char)randomKeys);
}

}  // namespace Script
