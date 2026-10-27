extern const float g_floatOne;    // GLOBAL: 0x5ca668 g_floatOne

// 0x464b10
float LerpFloat(float t, float a, float b)
{
	return (g_floatOne - t) * b + t * a;
}
