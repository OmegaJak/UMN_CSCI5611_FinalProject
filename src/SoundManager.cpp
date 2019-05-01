#include <fstream>

#include "SDL.h"
#include "SoundManager.h"

int SoundManager::lastSampleIndex = 0;
int SoundManager::lastSimulationSampleIndex = 0;
float SoundManager::soundBuff[SoundManager::soundBuffSize];

SoundManager::SoundManager(int samplesPerSecond) {
    InitSound(samplesPerSecond);
}

void audio_callback(void*, Uint8*, int);
void square_wave_callback(void* _beeper, Uint8* _stream, int _len) {
    int ToneHz = 500;
    int SquareWavePeriod = 48000 / ToneHz;  // Int division may lead to tones being slightly off
    int HalfSquareWavePeriod = SquareWavePeriod / 2;

    short* stream = (short*)_stream;

    printf("len: %i\n", _len);
    for (int i = 0; i < _len / 2; i++) {
        // printf("LastP: %i\n", lastP);
        int j = SoundManager::lastSampleIndex++ / HalfSquareWavePeriod;
        stream[i] = (j % 2) ? SoundManager::ToneVolume : -SoundManager::ToneVolume;
    }
}

void SoundManager::InitSound(int samplesPerSecond) {
    SDL_AudioSpec desiredSpec;

    desiredSpec.freq = samplesPerSecond;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 2048;
    desiredSpec.callback = audio_callback;

    SDL_AudioSpec obtainedSpec;

    // I think this can return error codes we should be checking
    if (SDL_OpenAudio(&desiredSpec, NULL) < 0) {
        printf("Failed to open audio\n");
        // return -1;
    }
    // TODO: Use newer OpenAudioDevice https://wiki.libsdl.org/SDL_OpenAudioDevice

    // start playing audio (unpause)
    SDL_PauseAudio(0);

    // Zero things
    lastSimulationSampleIndex = 0;
    for (int i = 0; i < soundBuffSize; i++) {
        soundBuff[i] = 0;
    }
}

void audio_callback(void* beeper_, Uint8* stream_, int len_) {
    short* stream = (short*)stream_;
    int len = len_ / 2;

    // TODO: Write what comes from string sim to wav file
    for (int i = 0; i < len; i++) {
        if (SoundManager::lastSampleIndex < SoundManager::lastSimulationSampleIndex) {
            if (SoundManager::lastSimulationSampleIndex > SoundManager::soundBuffSize) exit(0);
            SoundManager::lastSampleIndex++;
            double amp = SoundManager::ToneVolume * 2 * SoundManager::soundBuff[SoundManager::lastSampleIndex];  // amplitude
            amp = amp > 3 * SoundManager::ToneVolume ? 3 * SoundManager::ToneVolume
                                                     : amp < -3 * SoundManager::ToneVolume ? -3 * SoundManager::ToneVolume : amp;  // clamp
            stream[i] = amp;
        } else {                        // Our simulation has fallen behind the audio rate
            stream[i] = stream[i - 1];  //... repeat last tone (I'm not sure why this works so well, but it does)
        }
    }

    if (SoundManager::lastSimulationSampleIndex + len > SoundManager::soundBuffSize) {
        printf("Looping Buffer %d %d %d!\n", SoundManager::lastSimulationSampleIndex - SoundManager::lastSampleIndex,
               SoundManager::lastSimulationSampleIndex, SoundManager::lastSampleIndex);
        if (SoundManager::lastSimulationSampleIndex > SoundManager::lastSampleIndex) {
            memcpy(&SoundManager::soundBuff[0], &SoundManager::soundBuff[SoundManager::lastSampleIndex],
                   sizeof(float) * (SoundManager::lastSimulationSampleIndex - SoundManager::lastSampleIndex));
            SoundManager::lastSimulationSampleIndex = SoundManager::lastSimulationSampleIndex - SoundManager::lastSampleIndex;
        } else {
            SoundManager::lastSimulationSampleIndex = 0;
        }
        SoundManager::lastSampleIndex = 0;
        memset(&SoundManager::soundBuff[SoundManager::lastSimulationSampleIndex], 0,
               sizeof(float) * (SoundManager::soundBuffSize - SoundManager::lastSimulationSampleIndex));
    }
}
