/*
 * Bink 1 container, video and audio decoding (binkdec.cpp).
 * Derived from FFmpeg's Bink demuxer and decoders; licence and copyrights in binkdec.cpp.
 *
 * This file is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.
 */
#ifndef ZOOSTERS_PORT_BINKDEC_H
#define ZOOSTERS_PORT_BINKDEC_H
#include <stddef.h>
#include <stdint.h>

namespace binkdec {

enum { kPadding = 16 };              // zero bytes every packet buffer needs after its data (the bit reader looks ahead)
enum { kHeaderMax = 48 + 12 * 256 }; // bytes from the file start that always hold the whole header

struct Header {
	uint32_t tag;                    // "BIK" + revision letter in the top byte
	uint32_t fileSize, frames, largestFrame, width, height, fpsNum, fpsDen, flags;
	uint32_t tracks;
	uint16_t trackRate[256], trackFlags[256];
	uint32_t indexOffset;            // file offset of the frame index: `frames` words, bit 0 = key frame
};
bool ParseHeader(const uint8_t *p, size_t n, Header *h);
inline int Revision(const Header &h) { return (int)(h.tag >> 24); }

struct Video;
Video *VideoOpen(const Header &h);   // null: not Bink 1 'c'..'k', or a bad size
bool VideoDecode(Video *v, const uint8_t *data, size_t n);   // data + n .. + kPadding readable
const uint8_t *VideoPlane(const Video *v, int plane, int *stride);   // last decoded frame: 0 Y, 1 Cb, 2 Cr (4:2:0)
void VideoClose(Video *v);

struct Audio;
Audio *AudioOpen(const Header &h, unsigned track);   // null: RDFT tracks, more than two channels
unsigned AudioRate(const Audio *a);
unsigned AudioChannels(const Audio *a);
// One track's packet of a frame (after the container's 4-byte length): *out = interleaved 16-bit samples (valid until the next
// call), returns sample frames (of a damaged packet: the blocks before the damage).
int AudioDecode(Audio *a, const uint8_t *data, size_t n, const int16_t **out);
void AudioClose(Audio *a);

}
#endif
