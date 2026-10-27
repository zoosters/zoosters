/* The MSVC C runtime names the game code calls, for a POSIX C library (macOS / Linux / Android). The build force-includes this
   header into every file of a non-Windows build (with port/compat/posix/ on the include path for <mbstring.h>), so src/game keeps the
   names the game used. Semantics as the MSVC functions, not as the nearest POSIX one where they differ (_vsnprintf). */
#ifndef PORT_COMPAT_POSIX_MSVC_CRT_H
#define PORT_COMPAT_POSIX_MSVC_CRT_H
#if defined(_WIN32)
#error "port/compat/posix/msvc_crt.h is for non-Windows builds"
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

static inline char *_strdup(const char *s) { return strdup(s); }
static inline int _stricmp(const char *a, const char *b) { return strcasecmp(a, b); }
static inline int _strnicmp(const char *a, const char *b, size_t n) { return strncasecmp(a, b, n); }
static inline char *_strlwr(char *s)
{
	for (char *p = s; *p; p++)
		*p = (char)tolower((unsigned char)*p);
	return s;
}
static inline char *_strupr(char *s)
{
	for (char *p = s; *p; p++)
		*p = (char)toupper((unsigned char)*p);
	return s;
}
/* MSVC _vsnprintf: -1 when the output does not fit (MSVC then leaves the buffer unterminated; here it holds the first n - 1 characters
   and a zero, which only makes a truncated result safer), otherwise the length. C99 vsnprintf returns the full length instead. */
static inline int _vsnprintf(char *buf, size_t n, const char *fmt, va_list ap)
{
	int r = vsnprintf(buf, n, fmt, ap);
	return r < 0 || (size_t)r >= n ? -1 : r;
}

static inline int _snprintf(char *buf, size_t n, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = _vsnprintf(buf, n, fmt, ap);
	va_end(ap);
	return r;
}
/* x87 control word (only logged by the port: gtrace_port.cpp): no x87 off Windows x86, reported as 0 */
static inline unsigned _control87(unsigned, unsigned) { return 0; }

#endif
