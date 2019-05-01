#include <SDL.h>
#include <SDL_audio.h>
#include <iostream>

using namespace std;

// Generates new samples and outputs them to the audio card
void Callback(void* userdata, Uint8* stream, int len) {
  // the format of stream depends on actual.format in main()
  // we're assuming it's AUDIO_S16SYS
  short* samples = reinterpret_cast<short*>(stream);
  size_t numSamples = len / sizeof(short);

  const unsigned int phase_delta = 600;
  static unsigned int phase = 0;

  // loop over all our samples
  for (size_t i = 0; i < numSamples; ++i) {
    phase += phase_delta;
    short out = 0;
    if ((phase >> 8) < 127)
      out = SHRT_MAX;
    else
      out = 0;

    samples[i] = out;
  }
}

int main(int argc, char* argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return -1;
  atexit(SDL_Quit);

  SDL_Window* screen =
      SDL_CreateWindow("Audio Window", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
  if (screen == NULL) return -1;

  SDL_AudioSpec spec;
  spec.freq = 22050;
  spec.format = AUDIO_S16LSB;
  spec.channels = 1;
  spec.samples = 4096;
  spec.callback = Callback;
  spec.userdata = NULL;
  SDL_AudioSpec actual;
  if (SDL_OpenAudio(&spec, &actual) < 0) {
    cerr << "I don't think you like this: " << SDL_GetError() << endl;
    return -1;
  }
  if (spec.format != actual.format) {
    cerr << "format mismatch!" << endl;
    return -1;
  }

  SDL_PauseAudio(0);

  SDL_Event ev;
  while (SDL_WaitEvent(&ev)) {
    if (ev.type == SDL_QUIT) break;
  }

  SDL_CloseAudio();
  SDL_Quit();
  return 0;
}
