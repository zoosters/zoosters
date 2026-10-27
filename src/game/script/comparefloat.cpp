// GLOBAL: 0x5d4d0c g_negEpsilon
extern const float g_negEpsilon;

// 0x43fff0
bool TFB_CompareFloat(float a, int op, float b)
{
	float d = a - b;
	switch (op) {
	case 3:
		return d < g_negEpsilon;
	case 0:
		return d <= 0.0001f;
	case 1:
		return d <= 0.0001f && d >= g_negEpsilon;
	case 2:
		return d >= g_negEpsilon;
	case 4:
		return d > 0.0001f;
	case 5:
		return !(d <= 0.0001f) || d < g_negEpsilon;
	}
	return false;
}
