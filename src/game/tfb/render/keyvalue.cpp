#include "rw/rwcore.h"
#include "tfb/render/keyvalue.h"

// 0x460090
int KeyValue::SerialSize()
{
	int size = 4;
	switch (kind) {
	case 0:
	case 1:
		size = 5;
		break;
	case 2:
	case 3:
		size = 8;
		break;
	case 4:
		size = 0x14;
		break;
	}
	return size;
}

// 0x4600d0
int KeyValue::Read(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &kind, 4))
		return 0;
	int size = 4;
	switch (kind) {
	case 0:
	case 1:
		RwStreamRead(stream, &b, 1);
		size = 5;
		break;
	case 2:
		RwStreamReadInt32(stream, &i, 4);
		size = 8;
		break;
	case 3:
		RwStreamReadReal(stream, &f, 4);
		size = 8;
		break;
	case 4:
		RwStreamReadReal(stream, &v[0], 4);
		RwStreamReadReal(stream, &v[1], 4);
		RwStreamReadReal(stream, &v[2], 4);
		RwStreamReadReal(stream, &v[3], 4);
		size = 0x14;
		break;
	}
	return size;
}
