// One keyframe value as the game's animation streams store it: a kind word, then a payload that depends on it.
#ifndef ZOOSTERS_TFB_RENDER_KEYVALUE_H
#define ZOOSTERS_TFB_RENDER_KEYVALUE_H

struct RwStream;

struct KeyValue {
	int kind;    // 0, 1: a byte; 2: an int; 3: a float; 4: four floats (any other value has no payload)
	union {
		unsigned char b;
		int i;
		float f;
		float v[4];
	};
	int SerialSize();  // bytes the value takes in a stream
	int Read(RwStream *stream);  // returns the bytes read
};

#endif
