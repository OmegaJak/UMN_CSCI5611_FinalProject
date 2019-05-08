#include <algorithm>
#include <fstream>

#include "ClothManager.h"
#include "SoundManager.h"

int SoundManager::lastSampleIndex = 0;
int SoundManager::lastSimulationSampleIndex = 0;
float SoundManager::soundBuff[SoundManager::soundBuffSize];
int SoundManager::_bufferCount = 0;

SoundManager::SoundManager(int samplesPerSecond) {
    InitSound(samplesPerSecond);
}
/*
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
*/
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

void SoundManager::copyToSoundBuffer(float* samples, int numSamplesToGenerate) {
    if (samples != nullptr) {
        int numToCopy = std::min(numSamplesToGenerate, SoundManager::soundBuffSize - SoundManager::lastSimulationSampleIndex);
        memcpy(&SoundManager::soundBuff[SoundManager::lastSimulationSampleIndex], samples, numToCopy * sizeof(float));
        SoundManager::lastSimulationSampleIndex += numToCopy;
    }
}

void SoundManager::audio_callback(void* beeper_, Uint8* stream_, int len_) {
    short* stream = (short*)stream_;
    int len = len_ / 2;
    printf("_count = %d\n", _bufferCount);
    for (int i = 0; i < soundBuffSize; ++i) {
        soundBuff[i] *= _bufferCount * 0.01;//TODO: try to deal with distortion
    }
    for (int i = 0; i < len; i++) {
        if (lastSampleIndex < lastSimulationSampleIndex) {
            if (lastSimulationSampleIndex > soundBuffSize) exit(0);
            stream[i] = GetAmplitude(soundBuff[lastSampleIndex]);
            lastSampleIndex++;
        } else {  // Our simulation has fallen behind the audio rate
            int ind = lastSimulationSampleIndex - 1 < 0 ? 0 : lastSimulationSampleIndex - 1;
            stream[i] = GetAmplitude(soundBuff[ind]);  //... repeat last tone (I'm not sure why this
                                                       // works so well, but it does)
        }
        // printf("stream[%i]: %i\n", i, stream[i]);
    }
    _bufferCount = 0;
    if (lastSimulationSampleIndex + len > soundBuffSize) {
        printf("Looping Buffer %d %d %d!\n", lastSimulationSampleIndex - lastSampleIndex, lastSimulationSampleIndex, lastSampleIndex);
        if (lastSimulationSampleIndex > lastSampleIndex) {
            memcpy(&soundBuff[0], &soundBuff[lastSampleIndex], sizeof(float) * (lastSimulationSampleIndex - lastSampleIndex));
            lastSimulationSampleIndex = lastSimulationSampleIndex - lastSampleIndex;
        } else {
            lastSimulationSampleIndex = 0;
        }
        lastSampleIndex = 0;
        memset(&soundBuff[lastSimulationSampleIndex], 0, sizeof(float) * (soundBuffSize - lastSimulationSampleIndex));
    }
}
