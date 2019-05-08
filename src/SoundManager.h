#pragma once
#include "SDL.h"
class SoundManager {
    static void audio_callback(void* , Uint8* , int);
    explicit SoundManager(int samplesPerSecond);
    void InitSound(int samplesPerSecond);
    static int _bufferCount;
    static double GetAmplitude(float sample);

   public:
    static void copyToSoundBuffer(float*, int);
    static SoundManager& getInstance() {
        static SoundManager instance(48000);
        return instance;
    }

    static const int soundBuffSize = 104 * 1024;
    static float soundBuff[soundBuffSize];
    static int lastSimulationSampleIndex;
    static int lastSampleIndex;
    static const short ToneVolume = 2000;
    static void addBuffer() {
        ++_bufferCount;
    }
};
