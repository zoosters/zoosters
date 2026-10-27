// The Bink video API (binkw32.dll) the game uses, implemented by port/bink/bink.cpp (ZOOSTERS_BINK) or port/shim/binkshim.cpp (no
// videos: BinkOpen fails and the game skips the movie).
#ifndef ZOOSTERS_TFB_LEVEL_BINK_H
#define ZOOSTERS_TFB_LEVEL_BINK_H

// The fields the game reads; the implementation keeps its own state behind them.
struct BINK {
	unsigned Width;
	unsigned Height;
	unsigned Frames;
	unsigned FrameNum;        // 1-based: the frame BinkDoFrame decodes next
	unsigned LastFrameNum;
	unsigned FrameRate;
	unsigned FrameRateDiv;
	unsigned ReadError;
};

extern "C" {
BINK *__stdcall BinkOpen(const char *name, unsigned flags);
void __stdcall BinkClose(BINK *bink);
int __stdcall BinkWait(BINK *bink);
void __stdcall BinkDoFrame(BINK *bink);
void __stdcall BinkNextFrame(BINK *bink);
int __stdcall BinkCopyToBuffer(BINK *bink, void *dest, int destpitch, unsigned destheight, unsigned destx, unsigned desty,
                                              unsigned flags);
void *__stdcall BinkOpenDirectSound(void *dsound);   // only its address is used: the sound-system opener for BinkSetSoundSystem
int __stdcall BinkSetSoundSystem(void *open, void *dsound);
}

#endif
