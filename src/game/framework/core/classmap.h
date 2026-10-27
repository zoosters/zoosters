#ifndef ZOOSTERS_FRAMEWORK_CORE_CLASSMAP_H
#define ZOOSTERS_FRAMEWORK_CORE_CLASSMAP_H

#include <map>
#include <string.h>

#include "framework/core/cclassfactory.h"

namespace RWS {
namespace CClassFactory {

struct ClassNameLess {
	bool operator()(const char *a, const char *b) const { return strcmp(a, b) < 0; }
};

// The names are not copied.
typedef std::map<const char *, CRegistrar *, ClassNameLess> ClassMap;

// On first use: registrations run during static initialisation.
ClassMap &GetClassMap();

} // namespace CClassFactory
} // namespace RWS

#endif
