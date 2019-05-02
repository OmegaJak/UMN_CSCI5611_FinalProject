#pragma once

class SoundManager {
   public:
    explicit SoundManager(int samplesPerSecond);

    void InitSound(int samplesPerSecond);

    static const int soundBuffSize = 104 * 1024;
    static float soundBuff[soundBuffSize];
    static int lastSimulationSampleIndex;
    static int lastSampleIndex;
    static const short ToneVolume = 2000;
};
