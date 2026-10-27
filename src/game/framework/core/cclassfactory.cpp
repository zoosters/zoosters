#include "framework/core/classmap.h"
#include "framework/core/cattributehandler.h"

namespace RWS {
namespace CClassFactory {

// 0x5522b0
ClassMap &GetClassMap()
{
	static ClassMap classMap;
	return classMap;
}

// 0x552340
unsigned GetRegisteredClassSize(const char *pClassName)
{
	ClassMap &classMap = GetClassMap();
	ClassMap::const_iterator it = classMap.find(pClassName);
	return it != classMap.end() ? it->second->size : 0;
}

// Hands the packet to the new instance's HandleAttributes.
// 0x552380
CAttributeHandler *MakeNew(const char *pClassName, const CAttributePacket &rAttr)
{
	ClassMap &classMap = GetClassMap();
	ClassMap::const_iterator it = classMap.find(pClassName);
	if (it == classMap.end() || !it->second->MakeNewFunc)
		return 0;
	CAttributeHandler *instance = it->second->MakeNewFunc(rAttr);
	if (instance)
		instance->HandleAttributes(rAttr);
	return instance;
}

void KeepRegisteredSize(size_t (*fn)()) {}

// 0x5523e0
CRegistrar::CRegistrar(const char *pClassName, MakeNewPtr makeNewFn, unsigned size, int UseLinearMemoryManager)
    : MakeNewFunc(makeNewFn), size(0)
{
	if (UseLinearMemoryManager)
		this->size = (size + m_AlignmentSize - 1) / m_AlignmentSize * m_AlignmentSize;
	// the first registration of a name is kept
	GetClassMap().insert(ClassMap::value_type(pClassName, this));
}

// Clears the whole map, not only this registration.
// 0x552440
CRegistrar::~CRegistrar()
{
	GetClassMap().clear();
}

} // namespace CClassFactory
} // namespace RWS
