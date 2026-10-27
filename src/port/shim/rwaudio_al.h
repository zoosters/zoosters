// OpenAL Soft output for the RWA stand-ins (rwaudio.cpp), loaded at run time (OpenAL32.dll next to the exe; LGPL, never linked, no AL
// headers). The game-visible audio state stays in rwaudio.cpp's model; once per RwaCoreUpdate tick it hands this layer a snapshot and
// the device follows. Nothing flows back. No library or PORT_AUDIO=0 = silent, same behaviour.
#ifndef PORT_RWAUDIO_AL_H
#define PORT_RWAUDIO_AL_H

struct WdRws;

bool alaOpen(void);                    // device + context; false = stay silent (logged)
void alaClose(void);
bool alaActive(void);

struct AlaVoice {                      // one virtual voice as the logic model sees it this tick
	const char *name;                  // wave name (PORT_ALTRACE)
	const void *wave;                  // key of the sample data (0 = none)
	const unsigned char *data; unsigned dataLen;
	unsigned rate, bits, channels;     // stored format of `data` (PCM)
	bool is3d;                         // wave created with the 3D flag (mono)
	bool playing;                      // logic clock running
	unsigned stamp;                    // trigger stamp (+0x54): a new value = (re)started at posSamples
	double posSamples;                 // logic position now
	bool loop;
	float freq, gain, pan;             // +0x3c Hz, +0x40, +0x44
	float pos[3], vel[3], minDist, maxDist;   // +0x58, +0x64, +0x70, +0x74 (metres)
};
void alaVoiceSync(void *key, const AlaVoice &v);
void alaVoiceDrop(void *key);
void alaWaveDrop(const void *wave);    // before the sample data is freed

struct AlaStream {                     // one RWA stream (.rws segment) as the logic model sees it
	const char *path; const WdRws *rws; int seg;
	int state;                         // observed play state (6 playing, 5 paused, other = silent)
	unsigned gen;                      // bumps on every (re)start from the logic side: the device restarts at posSec
	double posSec;
	bool loop;
	float pitch;
	float gain[2][2];                  // [pair][side] (_rwaStreamSetStreamGain)
};
void alaStreamSync(void *key, const AlaStream &s);
void alaStreamDrop(void *key);

struct AlaListener { float pos[3], up[3], at[3], right[3]; float distanceFactor, doppler, rolloff; };   // +0x28, +0x40, +0x4c, +0x58, +0x64..+0x6c
void alaListenerSync(const AlaListener &l);
void alaOutputSync(float masterGain, const int *env12, bool envChanged);   // env: output +0x44 (DSFXI3DL2Reverb layout)

// Movie sound (port/bink): one push-fed 16-bit source outside the RWA model, at full gain (the listener's mute / focus gain applies).
bool alaPcmOpen(unsigned rate, unsigned channels);   // false = silent (no device, or the PORT_AUDIO_WAV loopback: it has no clock)
bool alaPcmPush(const short *pcm, unsigned frames);  // false = every buffer queued
void alaPcmPlay();                                   // start, or restart after an underrun
double alaPcmQueuedSec();                            // pushed and not yet played
unsigned long long alaPcmPlayedFrames();             // since alaPcmOpen
void alaPcmClose();

void alaTickBegin(int frame);          // before the syncs of a tick (frame number for PORT_ALTRACE)
void alaTickEnd(double nowSec);        // refill stream queues; PORT_AUDIO_WAV: render the loopback mix up to nowSec
#endif
