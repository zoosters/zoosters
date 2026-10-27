/* <mbstring.h> for a POSIX C library: the game's multibyte string calls. The game ran in a single-byte code page (no DBCS lead
   bytes in its English text), where the _mbs functions compare bytes as unsigned char, as strcmp does. */
#ifndef PORT_COMPAT_POSIX_MBSTRING_H
#define PORT_COMPAT_POSIX_MBSTRING_H
#include <string.h>
static inline int _mbscmp(const unsigned char *a, const unsigned char *b) { return strcmp((const char *)a, (const char *)b); }
#endif
