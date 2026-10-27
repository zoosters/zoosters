// Keyframe set: keys of (time, KeyValue) sampled at a time. LipSyncKFSet picks mouth shapes.
#ifndef ZOOSTERS_TFB_ANIM_KFSET_H
#define ZOOSTERS_TFB_ANIM_KFSET_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "tfb/render/flags32.h"
#include "tfb/render/keyvalue.h"
#include "tfb/render/tfbstring.h"

namespace TFB {

// KeyValue kind 4: colour or vector.
struct Vec4 {
	float x, y, z, w;
	void Lerp(const Vec4 *a, const Vec4 *b, float t);
};
ZOOSTERS_ASSERT_SIZE(Vec4, 0x10);

// v[0], v[1]: mouth shape indices as floats, v[2]: weight of the second.
enum { KEYVALUE_MOUTH_BLEND = 5 };

struct KFKey {
	float m_time;
	KeyValue m_value;
};
ZOOSTERS_ASSERT_SIZE(KFKey, 0x18);
ZOOSTERS_ASSERT_OFFSET(KFKey, m_value, 0x04);

class KFSet {
public:
	KFSet();
	~KFSet();  // not virtual

	// Kinds 2 / 3 / 4; always true.
	virtual bool Interpolate(const KeyValue *a, const KeyValue *b, float t, KeyValue *out);
	// The first key's value before it, the last key's after the end, else Interpolate between the keys around t. False without keys.
	virtual bool GetValue(float t, KeyValue *out);

	int Read(RwStream *stream);
	int ReadFormat1(RwStream *stream);  // id, name, flags, m_unk10, keys
	int ReadFormat2(RwStream *stream);  // format 1 plus the length
	int SerialSize();

protected:
	// First key not before t (a NaN time counts as not before), or m_keyCount.
	int FindKey(float t) const;

public:
	int m_format;        // stream format id; starts as g_animDefaultB
	int m_id;            // id the stream gives the set
	Flags32 m_keyFlags;  // unused
	int m_unk10;         // 3 after construction; nothing reads it
	TFBString m_name;
	float m_length;      // seconds the keys cover; AnimMouth::Play starts its clock with it
	int m_keyCount;
	KFKey *m_keys;       // sorted by time
};
ZOOSTERS_ASSERT_SIZE(KFSet, 0x24);
ZOOSTERS_ASSERT_OFFSET(KFSet, m_keyFlags, 0x0c);
ZOOSTERS_ASSERT_OFFSET(KFSet, m_name, 0x14);
ZOOSTERS_ASSERT_OFFSET(KFSet, m_length, 0x18);
ZOOSTERS_ASSERT_OFFSET(KFSet, m_keys, 0x20);

// Mouth-shape keys (key word 1 = shape index): GetValue returns the two shapes around t and the blend weight.
class LipSyncKFSet : public KFSet {
public:
	LipSyncKFSet() {}
	~LipSyncKFSet();
	virtual bool GetValue(float t, KeyValue *out);
	static void *operator new(size_t size) { return MemAlloc((unsigned)size); }
};

} // namespace TFB

#endif
