#ifndef ZOOSTERS_FRAMEWORK_CORE_CCLASSFACTORY_H
#define ZOOSTERS_FRAMEWORK_CORE_CCLASSFACTORY_H

#include "zoosters/base.h"

namespace RWS {

class CAttributeHandler;
class CAttributePacket;

// RWS_MAKENEWCLASS
typedef CAttributeHandler *(*MakeNewPtr)(const CAttributePacket &);

namespace CClassFactory {

CAttributeHandler *MakeNew(const char *pClassName, const CAttributePacket &rAttr);
unsigned GetRegisteredClassSize(const char *pClassName);
// Does nothing; RWS_MAKENEWCLASS passes RegisteredSize here so that it is emitted.
void KeepRegisteredSize(size_t (*fn)());

// Linear-pool allocations are rounded up to this.
const unsigned m_AlignmentSize = 64;

class CRegistrar {
public:
	CRegistrar(const char *pClassName, MakeNewPtr makeNewFn, unsigned size, int UseLinearMemoryManager);
	~CRegistrar();
	MakeNewPtr MakeNewFunc;
	unsigned size;   // rounded up to m_AlignmentSize; 0 when the class does not use the linear memory manager
};
ZOOSTERS_ASSERT_SIZE(CRegistrar, 8);

}  // namespace CClassFactory
}  // namespace RWS

#endif
