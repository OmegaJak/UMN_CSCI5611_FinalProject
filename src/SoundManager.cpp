#include <algorithm>
#include <fstream>

#include "ClothManager.h"
#include "SoundManager.h"

int SoundManager::lastSampleIndex = 0;
int SoundManager::lastSimulationSampleIndex = 0;
float SoundManager::soundBuffs[SoundManager::BUfferNumber][SoundManager::soundBuffSize];
int SoundManager::_SmartBuff[SoundManager::BUfferNumber];
int SoundManager::_bufferCount = 0;
std::priority_queue<std::pair<float, unsigned int>> SoundManager::_q;

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
    std::memset(_SmartBuff, 0, sizeof _SmartBuff);
    std::memset(soundBuffs, 0, sizeof(float) * BUfferNumber * soundBuffSize);

}

double SoundManager::GetAmplitude(float sample) {
    static double tripToneVol = 3 * ToneVolume;
    double amp = ToneVolume * 2 * sample;                                               // amplitude
    amp = amp > tripToneVol ? tripToneVol : (amp < -tripToneVol ? -tripToneVol : amp);  // clamp
    return amp;
}

unsigned int SoundManager::copyToSoundBuffer(float* samples, int numSamplesToGenerate) {
    unsigned int index = findNextFreeBuff();
    if (samples != nullptr) {
        int numToCopy = std::min(numSamplesToGenerate, SoundManager::soundBuffSize - SoundManager::lastSimulationSampleIndex);
        memcpy(&SoundManager::soundBuffs[index][SoundManager::lastSimulationSampleIndex], samples, numToCopy * sizeof(float));
        SoundManager::lastSimulationSampleIndex += numToCopy;
    }
    return index;
}

void SoundManager::audio_callback(void* beeper_, Uint8* stream_, int len_) {
    short* stream = (short*)stream_;
    int len = len_ / 2;
    //printf("_count = %d\n", _bufferCount);
    static float playBuff[SoundManager::soundBuffSize];
    
    for (int i = 0; i < soundBuffSize; ++i) {
        soundBuffs[0][i] *= _bufferCount * 0.01;//TODO: try to deal with distortion
    

    for (int i = 0; i < len; i++) {
        if (lastSampleIndex < lastSimulationSampleIndex) {
            if (lastSimulationSampleIndex > soundBuffSize) exit(0);
            stream[i] = GetAmplitude(soundBuffs[0][lastSampleIndex]);
            lastSampleIndex++;
        } else {  // Our simulation has fallen behind the audio rate
            int ind = lastSimulationSampleIndex - 1 < 0 ? 0 : lastSimulationSampleIndex - 1;
            stream[i] = GetAmplitude(soundBuffs[0][ind]);  //... repeat last tone (I'm not sure why this
                                                       // works so well, but it does)
        }
        // printf("stream[%i]: %i\n", i, stream[i]);
    }
    _bufferCount = 0;
    if (lastSimulationSampleIndex + len > soundBuffSize) {
        printf("Looping Buffer %d %d %d!\n", lastSimulationSampleIndex - lastSampleIndex, lastSimulationSampleIndex, lastSampleIndex);
        if (lastSimulationSampleIndex > lastSampleIndex) {
            memcpy(&soundBuffs[0][0], &soundBuffs[0][lastSampleIndex], sizeof(float) * (lastSimulationSampleIndex - lastSampleIndex));
            lastSimulationSampleIndex = lastSimulationSampleIndex - lastSampleIndex;
        } else {
            lastSimulationSampleIndex = 0;
        }
        lastSampleIndex = 0;
        memset(&playBuff[lastSimulationSampleIndex], 0, sizeof(float) * (soundBuffSize - lastSimulationSampleIndex));
    }
}
