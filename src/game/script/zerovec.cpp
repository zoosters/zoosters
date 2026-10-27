namespace Script { extern float g_zeroVec3[3]; }

// 0x43fd10
struct Vec3;
Vec3 *GetZeroVec3()
{
	return (Vec3 *)Script::g_zeroVec3;
}
