// RTTI records of std::exception, logic_error, length_error, out_of_range and type_info, in the game's order (see rtti_std.h).
// Only the W2aLogic, W2aLength and out_of_range vtables point at locators (rows 0x5dc0b4, 0x5dc0c0, 0x5dc0cc).
#include "rtti_std.h"

// BCD base class descriptor, BCA base class array, CHD class hierarchy descriptor, COL complete object locator.
#define BCD(td, n) { &td, n, 0, -1, 0, 0 }

// ALONE: g_rttiBcdException only reached through pointers of the arrays below
// GLOBAL: 0x5fc4f0 g_rttiBcdException
extern const RttiBaseClassDesc g_rttiBcdException = BCD(g_tdException, 0);
// ALONE: g_rttiBcdLogicError only reached through pointers of the arrays below
// GLOBAL: 0x5fc508 g_rttiBcdLogicError
extern const RttiBaseClassDesc g_rttiBcdLogicError = BCD(g_tdLogicError, 1);
// ALONE: g_rttiBcaLogicError array of 2 pointers + null, only read through the hierarchy descriptor
// GLOBAL: 0x5fc520 g_rttiBcaLogicError
extern const RttiBaseClassDesc *const g_rttiBcaLogicError[3] = { &g_rttiBcdLogicError, &g_rttiBcdException, 0 };
// ALONE: g_rttiChdLogicError only reached through the locator
// GLOBAL: 0x5fc52c g_rttiChdLogicError
extern const RttiClassHierarchyDesc g_rttiChdLogicError = { 0, 0, 2, g_rttiBcaLogicError };
// ALONE: g_rttiColW2aLogic vtable[-1] of W2aLogic (0x5dc0b4)
// GLOBAL: 0x5fc53c g_rttiColW2aLogic
extern const RttiCompleteObjectLocator g_rttiColW2aLogic = { 0, 0, 0, &g_tdLogicError, &g_rttiChdLogicError };

// ALONE: g_rttiBcdLengthError only reached through pointers of the arrays below
// GLOBAL: 0x5fc550 g_rttiBcdLengthError
extern const RttiBaseClassDesc g_rttiBcdLengthError = BCD(g_tdLengthError, 2);
// ALONE: g_rttiBcaLengthError array of 3 pointers + null
// GLOBAL: 0x5fc568 g_rttiBcaLengthError
extern const RttiBaseClassDesc *const g_rttiBcaLengthError[4] = { &g_rttiBcdLengthError, &g_rttiBcdLogicError, &g_rttiBcdException, 0 };
// ALONE: g_rttiChdLengthError only reached through the locator
// GLOBAL: 0x5fc578 g_rttiChdLengthError
extern const RttiClassHierarchyDesc g_rttiChdLengthError = { 0, 0, 3, g_rttiBcaLengthError };
// ALONE: g_rttiColW2aLength vtable[-1] of W2aLength (0x5dc0c0)
// GLOBAL: 0x5fc588 g_rttiColW2aLength
extern const RttiCompleteObjectLocator g_rttiColW2aLength = { 0, 0, 0, &g_tdLengthError, &g_rttiChdLengthError };

// ALONE: g_rttiBcdOutOfRange only reached through the array below
// GLOBAL: 0x5fc59c g_rttiBcdOutOfRange
extern const RttiBaseClassDesc g_rttiBcdOutOfRange = BCD(g_tdOutOfRange, 2);
// ALONE: g_rttiBcaOutOfRange array of 3 pointers + null
// GLOBAL: 0x5fc5b4 g_rttiBcaOutOfRange
extern const RttiBaseClassDesc *const g_rttiBcaOutOfRange[4] = { &g_rttiBcdOutOfRange, &g_rttiBcdLogicError, &g_rttiBcdException, 0 };
// ALONE: g_rttiChdOutOfRange only reached through the locator
// GLOBAL: 0x5fc5c4 g_rttiChdOutOfRange
extern const RttiClassHierarchyDesc g_rttiChdOutOfRange = { 0, 0, 3, g_rttiBcaOutOfRange };
// ALONE: g_rttiColOutOfRange vtable[-1] of std::out_of_range (0x5dc0cc)
// GLOBAL: 0x5fc5d4 g_rttiColOutOfRange
extern const RttiCompleteObjectLocator g_rttiColOutOfRange = { 0, 0, 0, &g_tdOutOfRange, &g_rttiChdOutOfRange };

// ALONE: g_rttiBcaException array of 1 pointer + null
// GLOBAL: 0x5fc5e8 g_rttiBcaException
extern const RttiBaseClassDesc *const g_rttiBcaException[2] = { &g_rttiBcdException, 0 };
// ALONE: g_rttiChdException only reached through the locator
// GLOBAL: 0x5fc5f0 g_rttiChdException
extern const RttiClassHierarchyDesc g_rttiChdException = { 0, 0, 1, g_rttiBcaException };
// ALONE: g_rttiColException locator of std::exception (nothing reads it)
// GLOBAL: 0x5fc600 g_rttiColException
extern const RttiCompleteObjectLocator g_rttiColException = { 0, 0, 0, &g_tdException, &g_rttiChdException };

// ALONE: g_rttiBcdTypeInfo only reached through the array below
// GLOBAL: 0x5fc614 g_rttiBcdTypeInfo
extern const RttiBaseClassDesc g_rttiBcdTypeInfo = BCD(g_tdTypeInfo, 0);
// ALONE: g_rttiBcaTypeInfo array of 1 pointer + null
// GLOBAL: 0x5fc62c g_rttiBcaTypeInfo
extern const RttiBaseClassDesc *const g_rttiBcaTypeInfo[2] = { &g_rttiBcdTypeInfo, 0 };
// ALONE: g_rttiChdTypeInfo only reached through the locator
// GLOBAL: 0x5fc634 g_rttiChdTypeInfo
extern const RttiClassHierarchyDesc g_rttiChdTypeInfo = { 0, 0, 1, g_rttiBcaTypeInfo };
// ALONE: g_rttiColTypeInfo locator of type_info (nothing reads it)
// GLOBAL: 0x5fc644 g_rttiColTypeInfo
extern const RttiCompleteObjectLocator g_rttiColTypeInfo = { 0, 0, 0, &g_tdTypeInfo, &g_rttiChdTypeInfo };
