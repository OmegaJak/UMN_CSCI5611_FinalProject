#include <algorithm>
#include <fstream>

#include "ClothManager.h"
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

    SDL_zero(desiredSpec);
    desiredSpec.freq = samplesPerSecond;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 2048;
    desiredSpec.callback = audio_callback;

    // Code isn't resilient to the spec being changed so make SDL convert appropriately
    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, false, &desiredSpec, nullptr, 0);
    if (device == 0) {
        printf("Failed to open audio %s\n", SDL_GetError());
    }

    // start playing audio (unpause)
    SDL_PauseAudioDevice(device, 0);

    // Zero things
    lastSimulationSampleIndex = 0;
    for (int i = 0; i < soundBuffSize; i++) {
        soundBuff[i] = 0;
    }
}

double SoundManager::GetAmplitude(float sample) {
    static double tripToneVol = 3 * ToneVolume;
    double amp = ToneVolume * 2 * sample;                                               // amplitude
    amp = amp > tripToneVol ? tripToneVol : (amp < -tripToneVol ? -tripToneVol : amp);  // clamp
    return amp;
}

void audio_callback(void* beeper_, Uint8* stream_, int len_) {
    short* stream = (short*)stream_;
    int len = len_ / 2;

    // TODO: Write what comes from string sim to wav file
    for (int i = 0; i < len; i++) {
        if (SoundManager::lastSampleIndex < SoundManager::lastSimulationSampleIndex) {
            if (SoundManager::lastSimulationSampleIndex > SoundManager::soundBuffSize) exit(0);
            stream[i] = SoundManager::GetAmplitude(SoundManager::soundBuff[SoundManager::lastSampleIndex]);
            SoundManager::lastSampleIndex++;
        } else {  // Our simulation has fallen behind the audio rate
            int ind = SoundManager::lastSimulationSampleIndex - 1 < 0 ? 0 : SoundManager::lastSimulationSampleIndex - 1;
            stream[i] = SoundManager::GetAmplitude(SoundManager::soundBuff[ind]);  //... repeat last tone (I'm not sure why this
                                                                                   // works so well, but it does)
        }
        // printf("stream[%i]: %i\n", i, stream[i]);
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
