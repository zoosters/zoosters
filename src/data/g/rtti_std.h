// MSVC 7.1 RTTI records for the exception classes the game throws. Type descriptors are in .data, the rest in .rdata
// (rtti_std.cpp). Names are ours.
#pragma once

template<int N> struct RttiTypeDesc { const void *m_vftable; void *m_spare; char m_name[N]; };
struct RttiBaseClassDesc { const void *m_typeDesc; unsigned m_numContainedBases; int m_mdisp; int m_pdisp; int m_vdisp; unsigned m_attributes; };
struct RttiClassHierarchyDesc { unsigned m_signature; unsigned m_attributes; unsigned m_numBaseClasses; const void *m_baseClassArray; };
struct RttiCompleteObjectLocator { unsigned m_signature; int m_offset; int m_cdOffset; const void *m_typeDesc; const void *m_classHierarchy; };

// The type_info vtable (1 slot: scalar deleting destructor); the first word of every type descriptor.
extern const void *g_typeInfoVftable[1];   // GLOBAL: 0x5dc164 g_typeInfoVftable

extern RttiTypeDesc<16> g_tdException;     // .?AVexception@@
extern RttiTypeDesc<24> g_tdLogicError;    // .?AVlogic_error@std@@
extern RttiTypeDesc<24> g_tdLengthError;   // .?AVlength_error@std@@
extern RttiTypeDesc<24> g_tdOutOfRange;    // .?AVout_of_range@std@@
extern RttiTypeDesc<16> g_tdTypeInfo;      // .?AVtype_info@@
