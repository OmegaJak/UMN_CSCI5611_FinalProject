#pragma once
#include <queue>
#include "SDL.h"
class SoundManager {
   public:
    enum {
        BUfferNumber = 128,  // nearly 2 seconds sound when fps is about 60
    };

   private:
    static void audio_callback(void*, Uint8*, int);
    explicit SoundManager(int samplesPerSecond);
    void InitSound(int samplesPerSecond);
    static int _bufferCount;
    static double GetAmplitude(float sample);

    static unsigned int findNextFreeBuff() {
        static unsigned int lastFound = 0;
        for (int i = 0; i < BUfferNumber; ++i) {
            if (lastFound == BUfferNumber) lastFound = 0;
            if (_SmartBuff[lastFound] == 0) return lastFound;
            lastFound++;
        }
        printf("WTF!!! need more buffer or shorter distinguish time!\n");
        exit(10);
    }
    static std::priority_queue<std::pair<float, unsigned int>> _q;
    static int _SmartBuff[BUfferNumber];

    static const short ToneVolume = 2000;

   public:
    static unsigned int copyToSoundBuffer(float*, int);
    static SoundManager& getInstance() {
        static SoundManager instance(48000);
        return instance;
    }
    static void addBuffer(float time, unsigned int index) {
        ++_SmartBuff[index];
        _q.push(std::make_pair(time, index));
    }

    static const int soundBuffSize = 104 * 1024;
    static float soundBuffs[BUfferNumber][soundBuffSize];
    static int lastSimulationSampleIndex;
    static int lastSampleIndex;
};
