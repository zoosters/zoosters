// ValueSlide op prototype (constructor 0x42fd20, getter 0x4300f0) and the slide ease curve (0x4350a0).
#include "script/ops_value.h"
using namespace Script;

// GLOBAL: 0x5cde78 Script::ValueSlide::vftable
// 0x42fd20
ValueSlide::ValueSlide() : m_easeOut(0), m_easeIn(0)
{
}

// 0x4300f0
ScriptReference *OpProto_ValueSlide()
{
	static ValueSlide s;
	return &s;
}

// ease curve: t in [0,1], a = ease-in share, b = ease-out share (scaled down when a + b > 1)
// Written this way to match: u and q declared and assigned separately, return value in parentheses (else `fstp st(1)` vs `fxch st(1); fstp st(0)`)
// 0x4350a0
float Math_EaseInOut(float t, float a, float b)
{
	float s = a + b;
	if (s == 0.0f)
		return t;
	if (s > 1.0f) {
		float k = 1.0f / s;
		a *= k;
		b *= k;
	}
	float d = 1.0f / (2.0f - a - b);
	if (t < a) {
		float q = d * (t * t);
		return q / a;
	}
	if (t < 1.0f - b)
		return (t + t - a) * d;
	float u;
	u = 1.0f - t;
	float q;
	q = d * (u * u);
	return (1.0f - q / b);
}
