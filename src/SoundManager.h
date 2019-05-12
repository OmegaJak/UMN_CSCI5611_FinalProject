#pragma once
#include <queue>
#include "Constants.h"

#include "SDL.h"
class SoundManager {
   public:
    enum {
        BUfferNumber = 512,  // nearly 2 seconds sound when fps is about 60
        SampleNum = SAMPLES_PER_FRAME,
    };
    static const int soundBuffSize = 104 * 1024;

   private:
    static void audio_callback(void*, Uint8*, int);
    explicit SoundManager(int samplesPerSecond);
    void InitSound(int samplesPerSecond);
    static int _bufferCount;
    static double GetAmplitude(float sample);

    static unsigned int findNextFreeBuff();
    static std::priority_queue<std::pair<float, unsigned int>> _q;  // too lazy to right a cmp, store negative time instead.
    static int _SmartBuff[BUfferNumber];
    static const short ToneVolume = 1000;
    static float _playBuff[SoundManager::soundBuffSize];

    static void copyToSoundBuffer(float*, int);
    static float soundBuffs[BUfferNumber][SampleNum];
    static int lastSimulationSampleIndex;
    static int lastSampleIndex;
    static bool printSamples;

   public:
    static unsigned int storeSample(float*);
    static void sumSoundsOntime();
    static SoundManager& getInstance() {
        static SoundManager instance(48000);
        return instance;
    }
    static void addBuffer(float time, unsigned int index);

    static void turnOffSound(int micPos, int sideDist, double* velY) {
        _playBuff[lastSimulationSampleIndex++] = .5 * velY[micPos] + .25 * velY[micPos - sideDist] + .25 * velY[micPos + sideDist];
    }
    static bool isSimulationAhead(int numTimesToUpdate) {
        return (lastSimulationSampleIndex - lastSampleIndex > 2 * 2048 || lastSimulationSampleIndex + numTimesToUpdate >= soundBuffSize);
    }

    static void togglePrintSamples() {
        printSamples = !printSamples;
    }
};
