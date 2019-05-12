#include <algorithm>
#include <fstream>

#include "ClothManager.h"
#include "RayTracer.h"
#include "SoundManager.h"

int SoundManager::lastSampleIndex = 0;
int SoundManager::lastSimulationSampleIndex = 0;
float SoundManager::soundBuffs[SoundManager::BUfferNumber][SoundManager::SampleNum];
int SoundManager::_SmartBuff[SoundManager::BUfferNumber];
float SoundManager::_playBuff[SoundManager::soundBuffSize];
int SoundManager::_bufferCount = 0;
bool SoundManager::printSamples = false;
std::priority_queue<std::pair<float, unsigned int>> SoundManager::_q;

extern float timePassed;
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
    std::memset(soundBuffs, 0, sizeof(float) * BUfferNumber * SampleNum);
}

double SoundManager::GetAmplitude(float sample) {
    static double tripToneVol = 3 * ToneVolume;
    double amp = ToneVolume * 2 * sample;                                               // amplitude
    //amp = amp > tripToneVol ? tripToneVol : (amp < -tripToneVol ? -tripToneVol : amp);  // clamp
    return amp;
}

void SoundManager::copyToSoundBuffer(float* samples, int numSamplesToGenerate) {
    if (samples != nullptr) {
        int numToCopy = std::min(numSamplesToGenerate, SoundManager::soundBuffSize - SoundManager::lastSimulationSampleIndex);
        memcpy(&SoundManager::_playBuff[SoundManager::lastSimulationSampleIndex], samples, numToCopy * sizeof(float));
        SoundManager::lastSimulationSampleIndex += numToCopy;
    }
}
void SoundManager::sumSoundsOntime() {
	static unsigned char count = 0;
	count++;
	count = count % 16;
	if(count != 0) return;

    static float tmpBuff[SampleNum];
    memset(tmpBuff, 0, sizeof(tmpBuff));
    while (!_q.empty() && -_q.top().first < timePassed) {
        unsigned int index = _q.top().second;
        // printf("Get buffer Index %u\n",  index);
        _q.pop();
        for (int i = 0; i < SampleNum; ++i) tmpBuff[i] += soundBuffs[index][i];
        --_SmartBuff[index];
    }
    // TODO: figure out a decent way for anti distortion
    //for (int i = 0; i < SampleNum; ++i) tmpBuff[i] *= 0.4f;
    copyToSoundBuffer(tmpBuff, SampleNum);
}

unsigned int SoundManager::storeSample(float* samples) {
    unsigned int index = findNextFreeBuff();
    memcpy(soundBuffs[index], samples, SampleNum * sizeof(float));
    return index;
}

unsigned int SoundManager::findNextFreeBuff() {
    static unsigned int lastFound = 0;

    for (int i = 0; i < BUfferNumber; ++i) {
        if (lastFound == BUfferNumber) lastFound = 0;
        if (_SmartBuff[lastFound] == 0) return lastFound;
        lastFound++;
    }

    printf("WTF!!! need more buffer or shorter distinguish time!\n");
    exit(42);
}

void SoundManager::audio_callback(void* beeper_, Uint8* stream_, int len_) {
    short* stream = (short*)stream_;
    int len = len_ / 2;

    for (int i = 0; i < len; i++) {
        int path = 0;
        if (lastSampleIndex < lastSimulationSampleIndex) {
            if (lastSimulationSampleIndex > soundBuffSize) exit(0);
            stream[i] = GetAmplitude(_playBuff[lastSampleIndex]);
            lastSampleIndex++;
            path = 1;
        } else {  // Our simulation has fallen behind the audio rate
            int ind = lastSimulationSampleIndex - 1 < 0 ? 0 : lastSimulationSampleIndex - 1;
            stream[i] = GetAmplitude(_playBuff[ind]);  //... repeat last tone (I'm not sure why this
                                                       // works so well, but it does)
            path = 2;
        }

        if (printSamples) {
            if (i == 0) printf("--------------------\n");
            printf("%i\n", stream[i]);
            if (i == len - 1) printf("--------------------\n");
        }
    }

    if (lastSimulationSampleIndex + len > soundBuffSize) {
        printf("Looping Buffer %d %d %d!\n", lastSimulationSampleIndex - lastSampleIndex, lastSimulationSampleIndex, lastSampleIndex);
        if (lastSimulationSampleIndex > lastSampleIndex) {
            memcpy(&_playBuff[0], &_playBuff[lastSampleIndex], sizeof(float) * (lastSimulationSampleIndex - lastSampleIndex));
            lastSimulationSampleIndex = lastSimulationSampleIndex - lastSampleIndex;
        } else {
            lastSimulationSampleIndex = 0;
        }
        lastSampleIndex = 0;
        memset(&_playBuff[lastSimulationSampleIndex], 0, sizeof(float) * (soundBuffSize - lastSimulationSampleIndex));
    }
}

void SoundManager::addBuffer(float time, unsigned int index) {
    ++_SmartBuff[index];
    _q.push(std::make_pair(-time, index));
}
