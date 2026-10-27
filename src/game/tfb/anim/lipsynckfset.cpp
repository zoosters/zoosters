#include "tfb/anim/kfset.h"

namespace TFB {

static int FirstKeyNotBefore(const KFKey *keys, int n, float t)
{
	int i = 0;
	while (i < n && keys[i].m_time < t)
		i++;
	return i;
}

// GLOBAL: 0x5d5408 TFB::LipSyncKFSet::vftable

// 0x44f3b0
LipSyncKFSet::~LipSyncKFSet() {}

// Before the first and after the last key: that key's shape twice, weight 0. Those end keys are copied into *out first, as in the game.
// 0x45b080
bool LipSyncKFSet::GetValue(float t, KeyValue *out)
{
	int count = m_keyCount;
	if (count <= 0)
		return false;
	float weight;
	int shapeA, shapeB;
	int next = 0;
	if (count == 1) {
		weight = 0.0f;
		shapeA = shapeB = m_keys[0].m_value.i;
	} else {
		next = FirstKeyNotBefore(m_keys, count, t);
		if (next < count) {
			if (next == 0) {
				int *words = (int *)out;
				for (int k = 0; k < 5; k++)
					words[k] = ((const int *)&m_keys[0].m_value)[k];
				weight = 0.0f;
				shapeA = shapeB = m_keys[0].m_value.i;
			} else {
				weight = (t - m_keys[next - 1].m_time) / (m_keys[next].m_time - m_keys[next - 1].m_time);
				shapeA = m_keys[next - 1].m_value.i;
				shapeB = m_keys[next].m_value.i;
			}
		} else {
			int *words = (int *)out;
			for (int k = 0; k < 5; k++)
				words[k] = ((const int *)&m_keys[count - 1].m_value)[k];
			weight = 0.0f;
			shapeA = shapeB = m_keys[count - 1].m_value.i;
		}
	}
	int *result = (int *)out;
	*result++ = KEYVALUE_MOUTH_BLEND;
	*(float *)&result[0] = (float)shapeA;
	*(float *)&result[1] = (float)shapeB;
	*(float *)&result[2] = weight;
	return true;
}

} // namespace TFB

// 0x44f3c0
void *__stdcall KFsetResourceHandler_Load(const char *name, const char *type, const char *path, RwStream *stream, unsigned size,
                                          unsigned *outSize)
{
	if (size > 0) {
		TFB::LipSyncKFSet *keys = new TFB::LipSyncKFSet;
		if (keys) {
			keys->Read(stream);
			*outSize = size;
			return keys;
		}
		*outSize = size;
		return 0;
	}
	*outSize = size;
	return 0;
}

// 0x44f4c0
int __stdcall KFsetResourceHandler_UnLoad(intptr_t type, TFB::LipSyncKFSet *keys)
{
	if (keys)
		delete keys;
	return 1;
}
