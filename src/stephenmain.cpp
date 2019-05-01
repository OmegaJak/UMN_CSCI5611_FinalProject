//// g++ -F/Library/Frameworks -framework SDL2 -framework OpenGL -O3
//// ../glad/glad.c soundSimple.cpp -o sound
//
//// Simple musical string simulation (visualization and aurilzation with SDL)
//// Author: Stephen J. Guy <sjguy@umn.edu>
//
//#include <SDL.h>
//#include <glad.h>
//
//#include <cstdio>  //Include order can matter here
//
//// Shader sources
// const GLchar* vertexSource =
//    "#version 150 core\n"
//    "in vec2 position;"
//    "in vec3 inColor;"
//    "out vec3 Color;"
//    "void main() {"
//    "   Color = inColor;"
//    "   gl_Position = vec4(position, 0.0, 1.0);"
//    "}";
//
// const GLchar* fragmentSource =
//    "#version 150 core\n"
//    "in vec3 Color;"
//    "out vec4 outColor;"
//    "void main() {"
//    "   outColor = vec4(Color, 1.0);"
//    "}";
//
// bool slowMode = false;  // false  true
// int slowRate = 20;
// bool captureVid = false;
//
// const int numV = 30;
//// double k = 50000, kV = 0.3, tension = .848, length = .755;  // Bell  - F4
//// (348 Hz)
// double k = 50000, kV = 0.3, tension = .848, length = .755;
//
// double dt = 0.0001;  // Notice, this is really small =)
//
// GLdouble vertices[numV * 5];
//
// double posX[numV], posY[numV];
// double velX[numV], velY[numV];
// double accX[numV], accY[numV];
//
// double restLen;
//
// const int soundBuffSize = 1 * 104 * 1024;
//
// float soundBuff[soundBuffSize];
// int lastSimulationSampleIndex = 0;
//
//// Useful tutorials
//// Audio Tutorial: https://davidgow.net/handmadepenguin/ch8.html
//// http://stackoverflow.com/questions/10110905/simple-wave-generator-with-sdl-in-c
// int SamplesPerSecond = 48000;
// short ToneVolume = 10000;
// unsigned RunningSampleIndex = 0;
//
// int ToneHz = 500;
// int SquareWavePeriod = SamplesPerSecond / ToneHz;  // Int division may lead to tones being slightly off
// int HalfSquareWavePeriod = SquareWavePeriod / 2;
//
// int lastX = 0;
// static int lastSampleIndex = 0;
//
// void audio_callback_(void*, Uint8*, int);
// void square_wave_callback_(void* _beeper, Uint8* _stream, int _len) {
//    short* stream = (short*)_stream;
//
//    printf("len: %i\n", _len);
//    for (int i = 0; i < _len / 2; i++) {
//        // printf("LastP: %i\n", lastP);
//        int j = lastSampleIndex++ / HalfSquareWavePeriod;
//        stream[i] = (j % 2) ? ToneVolume : -ToneVolume;
//    }
//}
//
// void audio_callback_(void* _beeper, Uint8* _stream, int _len) {
//    short* stream = (short*)_stream;
//    int len = _len / 2;
//
//    // TODO: Write what comes from string sim to wav file
//    for (int i = 0; i < len; i++) {
//        if (lastSampleIndex < lastSimulationSampleIndex) {
//            if (lastSimulationSampleIndex > soundBuffSize) exit(0);
//            lastSampleIndex++;
//            double amp = ToneVolume * 2 * soundBuff[lastSampleIndex];                                     // amplitude
//            amp = amp > 3 * ToneVolume ? 3 * ToneVolume : amp < -3 * ToneVolume ? -3 * ToneVolume : amp;  // clamp
//            stream[i] = amp;
//        } else {                        // Our simulation has fallen behind the audio rate
//            stream[i] = stream[i - 1];  //... repeat last tone (I'm not sure why this works so well, but it does)
//        }
//    }
//
//    if (lastSimulationSampleIndex + len > soundBuffSize) {
//        printf("Looping Buffer %d %d %d!\n", lastSimulationSampleIndex - lastSampleIndex, lastSimulationSampleIndex, lastSampleIndex);
//        if (lastSimulationSampleIndex > lastSampleIndex) {
//            memcpy(&soundBuff[0], &soundBuff[lastSampleIndex], sizeof(float) * (lastSimulationSampleIndex - lastSampleIndex));
//            lastSimulationSampleIndex = lastSimulationSampleIndex - lastSampleIndex;
//        } else {
//            lastSimulationSampleIndex = 0;
//        }
//        lastSampleIndex = 0;
//        memset(&soundBuff[lastSimulationSampleIndex], 0, sizeof(float) * (soundBuffSize - lastSimulationSampleIndex));
//    }
//}
//
// void update(double dt, int numTimesToUpdate = 1, bool audio = false) {
//    if (lastSimulationSampleIndex - lastSampleIndex > 2 * 2048 || lastSimulationSampleIndex + numTimesToUpdate >= soundBuffSize)
//        return;  // Simulate ahead to keep a small buffer of sound
//    for (int z = 0; z < numTimesToUpdate; z++) {
//        for (int q = 0; q < 10; q++) {
//            for (int i = 0; i < numV; i++) {  // Zero out acceleration
//                accX[i] = 0;
//                accY[i] = 0;
//            }
//
//            for (int i = 0; i < numV - 1; i++) {  // Compute new acceleration F=ma + Hook's Law
//                double xlen = posX[i + 1] - posX[i];
//                double ylen = posY[i + 1] - posY[i];
//                double leng = sqrt(xlen * xlen + ylen * ylen);
//                double force = (k / restLen) * (leng - restLen);
//                double forceX = xlen / leng;
//                double forceY = ylen / leng;
//                double aX = forceX * force;
//                double aY = forceY * force;
//                // if i == 0: print(ittr,force,leng,forceX)
//                aX += kV * (velX[i + 1] - velX[i]);
//                aY += kV * (velY[i + 1] - velY[i]);
//                accX[i] += aX / 2;
//                accY[i] += aY / 2;
//                accX[i + 1] += -aX / 2;
//                accY[i + 1] += -aY / 2;
//            }
//
//            // printf("X: %f %f %f\n",posY[numV/2],accY[numV/2],velY[numV/2]);
//
//            for (int i = 1; i < numV - 1; i++) {  // Eulerian integration
//                velX[i] += accX[i] * dt;
//                velY[i] += accY[i] * dt;
//                posX[i] += velX[i] * dt;
//                posY[i] += velY[i] * dt;
//            }
//        }
//
//        if (audio) {  // Allow us to turn off the sound (for debugging)
//            int micPos = numV / 2;
//            int sideDist = numV / 3;
//            SDL_LockAudio();
//            soundBuff[lastSimulationSampleIndex++] = .5 * velY[micPos] + .25 * velY[micPos - sideDist] + .25 * velY[micPos + sideDist];
//            SDL_UnlockAudio();
//        }
//        // printf("X: %f\n",velY[numV/2]); //Print velocity for debugging
//    }
//
//    // Update visuals to match simulation
//    for (int i = 0, v = 0; i < numV * 5; i += 5, v++) {
//        vertices[i] = posX[v];
//        vertices[i + 1] = posY[v];
//        vertices[i + 2] = 1.0f;
//        vertices[i + 3] = 0;
//        vertices[i + 4] = 0;
//    }
//}
//
// void pluck(float strength = 0.1, int location = -1) {
//    double d = strength * restLen;  // scale pluck with string size
//    if (location == -1) location = numV / 2;
//    posY[location] += d;
//    posY[location - 1] += d / 2;
//    posY[location + 1] += d / 2;
//    posY[location - 2] += d / 2;
//    posY[location + 2] += d / 2;
//}
//
// void strike(float strength = 0.1) {
//    double d = strength * 20;
//    velY[numV / 2] += d;
//    velY[numV / 2 - 1] += d / 2;
//    velY[numV / 2 + 1] += d / 2;
//    velY[numV / 2 - 2] += d / 2;
//    velY[numV / 2 + 2] += d / 2;
//}
//
// void printState() {
//    printf("tension: %f   length: %f    K: %f   kV: %f\n", restLen * numV / (posX[numV - 1] - posX[0]), posX[numV - 1] - posX[0], k, kV);
//}
//
// int main(int argc, char* argv[]) {
//    // ==== Init Audio ====
//    SDL_Init(SDL_INIT_AUDIO);
//    SDL_AudioSpec desiredSpec;
//
//    desiredSpec.freq = SamplesPerSecond;
//    desiredSpec.format = AUDIO_S16SYS;
//    desiredSpec.channels = 1;
//    desiredSpec.samples = 2048;
//    desiredSpec.callback = audio_callback_;
//
//    SDL_AudioSpec obtainedSpec;
//
//    // I think this can return error codes we should be checking
//    if (SDL_OpenAudio(&desiredSpec, NULL) < 0) {
//        printf("Failed to open audio\n");
//        return -1;
//    }
//    // TODO: Use newer OpenAudioDevice https://wiki.libsdl.org/SDL_OpenAudioDevice
//
//    // start playing audio (unpause)
//    SDL_PauseAudio(0);
//
//    //==++++++==============
//
//    SDL_Init(SDL_INIT_VIDEO);  // Initialize Graphics (for OpenGL)
//
//    // Ask SDL to get a recent version of OpenGL (3.2 or greater)
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
//
//    // Create a window (offsetx, offsety, width, height, flags)
//    SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, 800, 400, SDL_WINDOW_OPENGL);
//
//    // Create a context to draw in
//    SDL_GLContext context = SDL_GL_CreateContext(window);
//
//    // Load OpenGL Extentions
//    if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
//        printf("\nOpenGL loaded\n");
//        printf("Vendor:   %s\n", glGetString(GL_VENDOR));
//        printf("Renderer: %s\n", glGetString(GL_RENDERER));
//        printf("Version:  %s\n\n", glGetString(GL_VERSION));
//    } else {
//        printf("ERROR: Failed to initialize OpenGL context.\n");
//        return -1;
//    }
//
//    // Build a Vertex Array Object. This stores the VBO and attribute mappings in
//    // one object
//    GLuint vao;
//    glGenVertexArrays(1, &vao);  // Create a VAO
//    glBindVertexArray(vao);      // Bind the above created VAO to the current context
//
//    for (int i = 0; i < numV; i++) {
//        // posX[i] = (length*(i/(numV+0.f) - .5 + .5/(numV+0.f)));
//        posX[i] = (length * (i / (numV + 0.f))) - length / 2;
//        // printf("%f\n",posX[i]);  //Node positions
//        posY[i] = .5;
//        velX[i] = 0;
//        velY[i] = 0;
//    }
//
//    // Make sure the string is under tension
//    restLen = tension * ((posX[numV - 1] - posX[0]) / (numV - 1));
//
//    lastSimulationSampleIndex = 0;
//    for (int i = 0; i < soundBuffSize; i++) {
//        soundBuff[i] = 0;
//    }
//
//    // Allocate memory on the graphics card to store geometry (vertex buffer
//    // object)
//    GLuint vbo;
//    glGenBuffers(1, &vbo);  // Create 1 buffer called vbo
//
//    // Load the vertex Shader
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertexSource, NULL);
//    glCompileShader(vertexShader);
//
//    // Let's double check the shader compiled
//    GLint status;
//    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
//    if (!status) {
//        char buffer[512];
//        glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
//        printf("Vertex Shader Compile Failed. Info:\n\n%s\n", buffer);
//    }
//
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
//    glCompileShader(fragmentShader);
//
//    // Double check the shader compiled
//    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
//    if (!status) {
//        char buffer[512];
//        glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
//        printf("Fragment Shader Compile Failed. Info:\n\n%s\n", buffer);
//    }
//
//    // Join the vertex and fragment shaders together into one program
//    GLuint shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glBindFragDataLocation(shaderProgram, 0, "outColor");  // set output
//    glLinkProgram(shaderProgram);                          // run the linker
//
//    glUseProgram(shaderProgram);  // Set the active shader (only one can be used at a time)
//
//    glEnable(GL_LINE_SMOOTH);
//
//    float lastTime = SDL_GetTicks() / 1000.f;
//    float newTime = SDL_GetTicks() / 1000.f;
//
//    printf("Press 't' to pluck the virtual string.\n");
//    printf("Things to try:\n");
//    printf("   Strike vs. Pluck\n");
//    printf("   Striking/Plucking at different intensities\n");
//    printf("   Dynamically changing Kv (dampening)\n");
//    printf("   Dynamically changing tension (tuning)\n");
//
//    // Event Loop (Loop forever processing each event as fast as possible)
//    SDL_Event windowEvent;
//    while (true) {
//        if (SDL_PollEvent(&windowEvent)) {
//            if (windowEvent.type == SDL_QUIT) break;
//            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE) break;  // Exit event loop
//
//            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_z) slowMode = !slowMode;
//
//            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_t) pluck(0.6);
//        }
//
//        // Clear the screen for new draw
//        glClearColor(.2f, 0.4f, 0.8f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        newTime = SDL_GetTicks();
//        // dt = (newTime-lastTime)* (1.f/500000); if (dt > .0005) dt = .0005;
//        // //DON'T DO THIS
//        float elapsed_ms = newTime - lastTime;
//        lastTime = SDL_GetTicks();
//        char window_title[30];
//        float fps = 1000.f / elapsed_ms;
//        sprintf(window_title, "Sound Synth - %3.1f fps", fps);
//        // sprintf(window_title, "Sound Synth", fps);
//        // sprintf(window_title, "Sound Synth");
//        SDL_SetWindowTitle(window, window_title);
//
//        // printf("%f FPS\n",dt);
//
//        float preUpdateTime = SDL_GetTicks();
//        update(dt, 2000, true);
//        float postUpdateTime = SDL_GetTicks();
//        printf("Update time: %fms\n", postUpdateTime - preUpdateTime);
//
//        glBindBuffer(GL_ARRAY_BUFFER,
//                     vbo);  // Set the vbo as the active array buffer (Only one
//                            // buffer can be active at a time)
//        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
//                     GL_DYNAMIC_DRAW);  // upload vertices to vbo
//
//        // Tell OpenGL how to set fragment shader input
//        GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
//        glVertexAttribPointer(posAttrib, 2, GL_DOUBLE, GL_FALSE, 5 * sizeof(double), 0);
//        // Attribute, vals/attrib., type, normalized?, stride, offset
//        // Binds to VBO current GL_ARRAY_BUFFER
//        glEnableVertexAttribArray(posAttrib);
//
//        GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
//        glVertexAttribPointer(colAttrib, 3, GL_DOUBLE, GL_FALSE, 5 * sizeof(double), (void*)(2 * sizeof(double)));
//        glEnableVertexAttribArray(colAttrib);
//
//        glLineWidth(5);
//        glDrawArrays(GL_LINE_STRIP, 0, numV);
//
//        SDL_GL_SwapWindow(window);  // Double buffering
//    }
//
//    glDeleteProgram(shaderProgram);
//    glDeleteShader(fragmentShader);
//    glDeleteShader(vertexShader);
//    glDeleteBuffers(1, &vbo);
//    glDeleteVertexArrays(1, &vao);
//
//    // Clean Up
//    SDL_CloseAudio();
//    SDL_GL_DeleteContext(context);
//    SDL_Quit();
//    return 0;
//}
