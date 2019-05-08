#pragma once

class SoundManager {
   public:
    explicit SoundManager(int samplesPerSecond);
    static void copyToSoundBuffer(float*, int);

   private:
	//TODO: sigleton if needed
    //static SoundManager* const gSoundManager = nullptr;
    void InitSound(int samplesPerSecond);

    static double GetAmplitude(float sample);
    static void audio_callback(void* beeper_, Uint8* stream_, int len_);

	public:
    static const int soundBuffSize = 104 * 1024;
    static float soundBuff[soundBuffSize];
    static int lastSimulationSampleIndex;
    static int lastSampleIndex;
    static const short ToneVolume = 2000;
};
