#pragma once
#include "Model.h"
#include "glad.h"

class Environment;

struct stringParams {
    GLfloat dt;
    GLfloat ks;
    GLfloat kd;
    GLfloat restLength;
    GLuint micPosition;
    GLuint micSpread;
};

struct simParams {
    GLint numSamplesToGenerate;
};

struct position {
    GLfloat x, y, z, w;
};

struct velocity {
    GLfloat vx, vy, vz, vw;
};

struct massConnections {
    GLuint left, right;
};

struct massParams {
    GLboolean isFixed;
    GLfloat mass;
    massConnections connections;
};

class ClothManager {
   public:
    ClothManager();

    void RenderParticles(float dt);
    void InitGL();
    void UpdateComputeParameters() const;
    void ExecuteComputeShader();
    void Pluck(int stringIndex, float strength = 0.1, int location = -1);
    void CopySamplesToAudioBuffer();

    static const unsigned int NUM_STRINGS = 8;
    static const unsigned int WORK_GROUP_SIZE = 32;
    static const unsigned int MASSES_PER_STRING = 32;
    static const unsigned int TOTAL_NUM_MASSES = NUM_STRINGS * MASSES_PER_STRING;
    static const unsigned int SAMPLES_PER_FRAME = 800;
    static const unsigned int GPU_SAMPLES_BUFFER_SIZE = NUM_STRINGS * SAMPLES_PER_FRAME;
    constexpr static const float BASE_HEIGHT = 20.0f;

    static GLuint posSSbo;
    static GLuint velSSbo;
    static GLuint accelSSbo;
    static GLuint samplesSSbo;
    static GLuint massSSbo;
    static GLuint stringParamSSbo;
    static GLuint globalSimParamsSSbo;

    stringParams stringParameters[NUM_STRINGS];
    simParams globalSimParameters{};
    float samplesBuffer[SAMPLES_PER_FRAME];
    static bool ready;
};
