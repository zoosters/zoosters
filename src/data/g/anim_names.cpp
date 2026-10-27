// Anim slot names as the script editor shows them: 78 names, a "<<" group-end entry, then null. A leading ">>group|" or
// "<<>>group|" opens a group (the second form closes the previous one first); the reader skips it. The names live in
// g_animNamePool, in reverse order.
#include "anim_name_pool.h"

// "complex" and "current" sit in front of the table.
// GLOBAL: 0x5ce728 g_animIndexNamesHead
extern __declspec(align(4)) const char *const g_animIndexNamesHead[2] = { g_animNamePool.m_complex, g_animNamePool.m_current };

// GLOBAL: 0x5ce730 g_animIndexNames
// ALONE: g_animIndexNames indexed 0..79 only (script/big_last_b.cpp: by member index, null-terminated); its neighbour g_animNamePool lives in
// anim_name_pool.cpp because cl emits a later-defined object that an initialiser names ahead of the table (data-globals pitfall)
extern __declspec(align(4)) const char *const g_animIndexNames[80] = {
	g_animNamePool.m_ambient, g_animNamePool.m_slowestMove, g_animNamePool.m_slowMove, g_animNamePool.m_fastMove,   // 0
	g_animNamePool.m_jump, g_animNamePool.m_fall, g_animNamePool.m_youch, g_animNamePool.m_slide,   // 4
	g_animNamePool.m_attack1, g_animNamePool.m_attack2, g_animNamePool.m_tumble, g_animNamePool.m_extra1,   // 8
	g_animNamePool.m_extra2, g_animNamePool.m_extra3, g_animNamePool.m_extra4, g_animNamePool.m_extra5,   // 12
	g_animNamePool.m_extra6, g_animNamePool.m_extra7, g_animNamePool.m_extra8, g_animNamePool.m_extra9,   // 16
	g_animNamePool.m_extra10, g_animNamePool.m_extra11, g_animNamePool.m_extra12, g_animNamePool.m_extra13,   // 20
	g_animNamePool.m_extra14, g_animNamePool.m_extra15, g_animNamePool.m_extra16, g_animNamePool.m_extra17,   // 24
	g_animNamePool.m_extra18, g_animNamePool.m_extra19, g_animNamePool.m_extra20, g_animNamePool.m_extra21,   // 28
	g_animNamePool.m_extra22, g_animNamePool.m_extra23, g_animNamePool.m_extra24, g_animNamePool.m_extra25,   // 32
	g_animNamePool.m_extra26, g_animNamePool.m_extra27, g_animNamePool.m_extra28, g_animNamePool.m_extra29,   // 36
	g_animNamePool.m_extra30, g_animNamePool.m_extra31, g_animNamePool.m_extra32, g_animNamePool.m_leftTurn,   // 40
	g_animNamePool.m_rightTurn, g_animNamePool.m_jumpLandSlow, g_animNamePool.m_jumpLandRunning, g_animNamePool.m_jumpLandHard,   // 44
	g_animNamePool.m_stopFromFullSpeed, g_animNamePool.m_stopFromSlowSpeed, g_animNamePool.m_running180Change, g_animNamePool.m_player9,   // 48
	g_animNamePool.m_player10, g_animNamePool.m_player11, g_animNamePool.m_player12, g_animNamePool.m_player13,   // 52
	g_animNamePool.m_player14, g_animNamePool.m_leanLeftSlow, g_animNamePool.m_leanLeftMedium, g_animNamePool.m_leanLeftFast,   // 56
	g_animNamePool.m_leanRightSlow, g_animNamePool.m_leanRightMedium, g_animNamePool.m_leanRightFast, g_animNamePool.m_idleCycle1A,   // 60
	g_animNamePool.m_idleCycle1B, g_animNamePool.m_idleCycle1C, g_animNamePool.m_idleTrans12, g_animNamePool.m_idleTrans13,   // 64
	g_animNamePool.m_idleCycle2A, g_animNamePool.m_idleCycle2B, g_animNamePool.m_idleCycle2C, g_animNamePool.m_idleTrans23,   // 68
	g_animNamePool.m_idleTrans21, g_animNamePool.m_idleCycle3A, g_animNamePool.m_idleCycle3B, g_animNamePool.m_idleCycle3C,   // 72
	g_animNamePool.m_idleTrans31, g_animNamePool.m_idleTrans32, g_animNamePool.m_groupEnd, 0,   // 76
};
