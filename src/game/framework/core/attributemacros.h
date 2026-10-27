// Goes in the public part of behaviour class X: declares its class factory entry points.
#ifndef ZOOSTERS_FRAMEWORK_CORE_ATTRIBUTEMACROS_H
#define ZOOSTERS_FRAMEWORK_CORE_ATTRIBUTEMACROS_H

#include "framework/core/cattributehandler.h"
#include "framework/core/cclassfactory.h"

// MakeNew hands RegisteredSize to KeepRegisteredSize so that RegisteredSize is emitted wherever MakeNew is.
#define RWS_MAKENEWCLASS(X)                                                                                                      \
	static size_t RegisteredSize()                                                                                               \
	{                                                                                                                            \
		return sizeof(X);                                                                                                        \
	}                                                                                                                            \
	static RWS::CAttributeHandler *MakeNew(const RWS::CAttributePacket &attr)                                                    \
	{                                                                                                                            \
		RWS::CClassFactory::KeepRegisteredSize(&RegisteredSize);                                                                 \
		return new X(attr);                                                                                                      \
	}

#endif
