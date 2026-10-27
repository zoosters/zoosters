/* <malloc.h> for a POSIX C library (macOS has none): malloc / free and the MSVC spelling _alloca. */
#ifndef PORT_COMPAT_POSIX_MALLOC_H
#define PORT_COMPAT_POSIX_MALLOC_H
#include <stdlib.h>
#include <alloca.h>
#ifndef _alloca
#define _alloca alloca
#endif
/* _heapchk (the port's PORT_HEAPCHK diagnostics): the MSVC CRT heap walk has no POSIX equivalent; reports a sound heap (use ASan instead) */
#ifndef _HEAPOK
#define _HEAPOK (-2)
#endif
static inline int _heapchk(void) { return _HEAPOK; }
#endif
