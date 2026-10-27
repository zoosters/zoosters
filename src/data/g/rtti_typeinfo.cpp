// type_info type descriptor. The other four live in data_run.cpp. The CRT words around it are not defined here.
#include "rtti_std.h"

// ALONE: g_tdTypeInfo only reached through the RTTI records (rtti_std.cpp); the CRT strings behind it are unnamed
// GLOBAL: 0x6116d4 g_tdTypeInfo
RttiTypeDesc<16> g_tdTypeInfo = { g_typeInfoVftable, 0, ".?AVtype_info@@" };
