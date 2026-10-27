// The unsuffixed <math.h> functions in double, as MSVC compiles the game (clang builds of the normal tree; --cc clang force-includes it).
// MSVC declares sin, cos, atan2 ... for double only, so `sin(f)` with a float argument promotes. libc++ adds float overloads to the
// global namespace, which turned the call into sinf and differed in the last bit for some inputs. The macros send every unsuffixed
// call to the double function; explicit sinf / cosf / sqrtf ... are untouched, as are sqrt / floor / ceil / fabs (exact either way).
// Standard headers that use these names are included first, so the macros miss their declarations.
#pragma once
#if !defined(_MSC_VER)
#include <math.h>
#include <cmath>
#include <complex>
#include <random>
#define sin(x) ::sin((double)(x))
#define cos(x) ::cos((double)(x))
#define tan(x) ::tan((double)(x))
#define asin(x) ::asin((double)(x))
#define acos(x) ::acos((double)(x))
#define atan(x) ::atan((double)(x))
#define atan2(y, x) ::atan2((double)(y), (double)(x))
#define exp(x) ::exp((double)(x))
#define log(x) ::log((double)(x))
#define pow(x, y) ::pow((double)(x), (double)(y))
#define fmod(x, y) ::fmod((double)(x), (double)(y))
#endif
