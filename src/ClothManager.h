#pragma once
#include "Model.h"
#include "glad.h"

class Environment;

struct simParams {
    GLfloat dt;
    GLfloat ks;
    GLfloat kd;
    GLfloat restLength;
    GLint numSamplesToGenerate;
    GLuint micPosition;
    GLuint micSpread;
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

    void RenderParticles(float dt, Environment *environment);
    void InitGL();
    void UpdateComputeParameters() const;
    void ExecuteComputeShader();
    void Pluck(float strength = 0.1, int location = -1);
    void CopySamplesToAudioBuffer();

    static const int WORK_GROUP_SIZE = 32;
    static const int NUM_MASSES = 32;
    static const int SAMPLES_BUFFER_SIZE = 4000;
    constexpr static const float BASE_HEIGHT = 20.0f;

    static GLuint posSSbo;
    static GLuint velSSbo;
    static GLuint accelSSbo;
    static GLuint samplesSSbo;
    static GLuint massSSbo;
    static GLuint paramSSbo;

    simParams simParameters{};
    static bool ready;
};
