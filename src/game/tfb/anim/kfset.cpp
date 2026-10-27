#include <stddef.h>
#include "tfb/anim/kfset.h"

// no unwind state: the game's ~KFSet treats the free as nothrow
void operator_delete(void *ptr); // 0x5500f0

extern int g_animDefaultB; // GLOBAL: 0x6066f4 g_animDefaultB (the latest KFSet stream format id)

namespace TFB {

// GLOBAL: 0x5d5694 TFB::KFSet::vftable

static int FirstKeyNotBefore(const KFKey *keys, int n, float t)
{
	int i = 0;
	while (i < n && keys[i].m_time < t)
		i++;
	return i;
}

static float LerpFloat(float a, float b, float t)
{
	return a + (b - a) * t;
}

// 0x459370
void Vec4::Lerp(const Vec4 *a, const Vec4 *b, float t)
{
	w = LerpFloat(a->w, b->w, t);
	x = LerpFloat(a->x, b->x, t);
	y = LerpFloat(a->y, b->y, t);
	z = LerpFloat(a->z, b->z, t);
}

// 0x4593d0
KFSet::KFSet()
{
	m_format = g_animDefaultB;
	m_id = 0;
	m_keyFlags.Set(0);
	m_keyCount = 0;
	m_keys = 0;
	m_unk10 = 3;
	m_length = 1.0f;
}

// 0x459450
KFSet::~KFSet()
{
	if (m_keys)
		operator_delete(m_keys);
}

// 0x4595e0
bool KFSet::Interpolate(const KeyValue *a, const KeyValue *b, float t, KeyValue *out)
{
	switch (a->kind) {
	case 2: {
		int value = (int)LerpFloat((float)a->i, (float)b->i, t);
		out->kind = 2;
		out->i = value;
		break;
	}
	case 3: {
		float value = LerpFloat(a->f, b->f, t);
		out->kind = 3;
		out->f = value;
		break;
	}
	case 4:
		// kind is not written for a Vec4 (out keeps its old kind)
		((Vec4 *)out->v)->Lerp((const Vec4 *)a->v, (const Vec4 *)b->v, t);
		break;
	}
	return true;
}

int KFSet::FindKey(float t) const
{
	return FirstKeyNotBefore(m_keys, m_keyCount, t);
}

// 0x459480
bool KFSet::GetValue(float t, KeyValue *out)
{
	int count = m_keyCount;
	if (count <= 0)
		return false;
	if (count == 1) {
		KFKey *key = m_keys;
		*out = key->m_value;
		return true;
	}
	int i = FirstKeyNotBefore(m_keys, count, t);
	if (i >= count) {
		KFKey *key = &m_keys[count - 1];
		*out = key->m_value;
		return true;
	}
	if (i == 0) {
		KFKey *key = m_keys;
		*out = key->m_value;
		return true;
	}
	KFKey *after = &m_keys[i];
	float fraction = (t - m_keys[i - 1].m_time) / (after->m_time - after[-1].m_time);
	Interpolate(&after[-1].m_value, &after->m_value, fraction, out);
	return true;
}

// 0x45a5d0
int KFSet::ReadFormat1(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &m_id, 4))
		return 0;
	int size = m_name.Read(stream) + 4;
	if (!RwStreamReadInt32(stream, (int *)&m_keyFlags.m_bits, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_unk10, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_keyCount, 4))
		return 0;
	size += 0xc;
	m_keys = (KFKey *)MemAlloc(m_keyCount * sizeof(KFKey));
	for (int i = 0; i < m_keyCount; i++) {
		char *entry = (char *)m_keys + i * 0x18;
		// a key whose time cannot be read adds 0 and the loop goes on, as in the game
		size += RwStreamReadReal(stream, (float *)entry, 4) ? ((KeyValue *)(entry + offsetof(KFKey, m_value)))->Read(stream) + 4 : 0;
	}
	return size;
}

// 0x45a6e0
int KFSet::ReadFormat2(RwStream *stream)
{
	int size = ReadFormat1(stream);
	if (!RwStreamReadReal(stream, &m_length, 4))
		return 0;
	return size + 4;
}

// 0x45a580
int KFSet::SerialSize()
{
	int size = m_name.SerialSize() + 0x14;
	for (int i = 0; i < m_keyCount; i++) {
		char *entry = (char *)m_keys + i * 0x18;
		size += ((KeyValue *)(entry + 4))->SerialSize() + 4;
	}
	return size + 4;
}

// 0x45ac70
int KFSet::Read(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &m_format, 4))
		return 0;
	int size = 4;
	switch (m_format) {
	case 0x0F0F0001:
		size = ReadFormat1(stream) + 4;
		break;
	case 0x0F0F0002:
		size = ReadFormat2(stream) + 4;
		break;
	}
	return size;
}

} // namespace TFB
