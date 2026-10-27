// Degrees to radians.

// The 20 B behind it hold the class name "CFXColorLight" (s_CFXColorLight) and a pointer to g_strTfbScript.
// GLOBAL: 0x5caf40 g_degToRad
// ALONE: g_degToRad scalar read by value only (hud2d.cpp: t * g_degToRad); g_xAxis is not reached from it
extern const float g_degToRad = 0.0174532925f;
