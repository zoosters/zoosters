/* Port lane: compiler portability for the modern C++ of port/ (force-included). PORT_THISCALL_FREE marks free functions with the thiscall convention: clang
   accepts __thiscall on them, MSVC does not (it is spelled __fastcall + a dummy edx argument there, see the users). */
#ifndef PORT_CC_H
#define PORT_CC_H
/* PORT_RETURN_ADDRESS(): the caller's return address (diagnostic logs): MSVC's intrinsic, __builtin_return_address elsewhere */
#if defined(_MSC_VER)
#ifdef __cplusplus
extern "C"
#endif
void *_ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)
#define PORT_RETURN_ADDRESS() _ReturnAddress()
#else
#define PORT_RETURN_ADDRESS() __builtin_return_address(0)
#endif
#if defined(_MSC_VER) && !defined(__clang__) && defined(_M_X64)
/* x64: one calling convention; `this` is the first argument, there is no dummy edx argument */
#define PORT_MSVC 1
#define PORT_THISCALL_FREE
#define PORT_EDX_PARAM
#define PORT_EDX_ARG
#elif defined(_MSC_VER) && !defined(__clang__)
#define PORT_MSVC 1
#define PORT_THISCALL_FREE __fastcall          /* ecx = this, edx = the dummy argument, the rest on the stack, callee cleans */
#define PORT_EDX_PARAM , void *port_edx_
#define PORT_EDX_ARG , 0
#else
#define PORT_MSVC 0
#define PORT_THISCALL_FREE __thiscall
#define PORT_EDX_PARAM
#define PORT_EDX_ARG
#endif
#endif
