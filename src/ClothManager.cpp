#define GLM_FORCE_RADIANS

#include <SDL_stdinc.h>

#include <algorithm>
#include <ctime>
#include <glm/gtc/type_ptr.hpp>

#include "ClothManager.h"
#include "Constants.h"
#include "Environment.h"
#include "ShaderManager.h"
#include "SoundManager.h"
#include "Timer.h"
#include "Utils.h"
#include "glad.h"
#include "RayTracer.h"

const GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

GLuint ClothManager::posSSbo;
GLuint ClothManager::velSSbo;
GLuint ClothManager::accelSSbo;
GLuint ClothManager::samplesSSbo;
GLuint ClothManager::massSSbo;
GLuint ClothManager::paramSSbo;
bool ClothManager::ready = false;

ClothManager::ClothManager() {
    srand(time(NULL));
    simParameters = simParams{0.008,  // dt
                              50000,  // ks
                              0.0,    // kd
                              0.99,   // restLength
                              600,    // updates
                              NUM_MASSES / 2,
                              NUM_MASSES / 3};
    InitGL();
}

void ClothManager::InitGL() {
    // Prepare the positions buffer //
    glGenBuffers(1, &posSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_MASSES * sizeof(position), nullptr, GL_DYNAMIC_DRAW);

    printf("Initializing mass positions...\n");
    position *positions = (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(position), bufMask);
    for (int i = 0; i < NUM_MASSES; i++) {
        positions[i] = {0.0f, float(i), 1.0f, BASE_HEIGHT};
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the mass parameters buffer //
    glGenBuffers(1, &massSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, massSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_MASSES * sizeof(massParams), nullptr, GL_STATIC_DRAW);

    printf("Initializing springs...\n");
    massParams *massParameters = (massParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(massParams), bufMask);
    for (int i = 0; i < NUM_MASSES; i++) {
        massParameters[i].isFixed = (i == 0 || i == NUM_MASSES - 1);
        // massParameters[i].isFixed = false;
        massParameters[i].mass = 20;

        // Initialize connections
        unsigned int left = BAD_INDEX, right = BAD_INDEX;
        if (i > 0) {
            left = i - 1;
        }

        if (i < NUM_MASSES - 1) {
            right = i + 1;
        }

        massParameters[i].connections = {left, right};
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the samples buffer (just zero it out) //
    glGenBuffers(1, &samplesSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, samplesSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, SAMPLES_BUFFER_SIZE * sizeof(GLfloat), nullptr, GL_DYNAMIC_READ);

    GLfloat *samples = (GLfloat *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, SAMPLES_BUFFER_SIZE * sizeof(GLfloat), bufMask);
    memset(samples, 0, SAMPLES_BUFFER_SIZE * sizeof(GLfloat));

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the velocities buffer
    glGenBuffers(1, &velSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_MASSES * sizeof(velocity), nullptr, GL_STATIC_DRAW);

    printf("Initializing mass velocities\n");
    velocity *vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(velocity), bufMask);
    memset(vels, 0, NUM_MASSES * sizeof(velocity));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the new velocities buffer
    glGenBuffers(1, &accelSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, accelSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_MASSES * sizeof(velocity), nullptr, GL_STATIC_DRAW);

    vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(velocity), bufMask);
    memset(vels, 0, NUM_MASSES * sizeof(velocity));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Misc data //
    glGenBuffers(1, &paramSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(simParams), nullptr, GL_STATIC_DRAW);

    simParams *params = (simParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(simParams), bufMask);
    *params = simParameters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    printf("Done initializing buffers\n");
}

void ClothManager::UpdateComputeParameters() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramSSbo);
    simParams *params = (simParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(simParams), bufMask);
    *params = simParameters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ClothManager::ExecuteComputeShader() {
    if (ready) CopySamplesToAudioBuffer();
    UpdateComputeParameters();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, samplesSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, paramSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, accelSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, massSSbo);

    glUseProgram(ShaderManager::ClothComputeShader);

    glDispatchCompute(1, 1, 1);                      // Run the cloth sim compute shader
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  // Wait for all to finish

    int numSSbos = 7;
    for (int i = 0; i < numSSbos; i++) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i + 1, 0);
    }

    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, samplesSSbo);
    GLfloat *samples = (GLfloat *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, SAMPLES_BUFFER_SIZE * sizeof(GLfloat), GL_MAP_READ_BIT);
    for (int i = 0; i < 25; i++) {
        printf("SamplesVal %i: %f\n", i, samples[i]);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);*/
    ready = true;
}

void ClothManager::Pluck(float strength, int location) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    position *positions =
        (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(position), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

    double d = strength * simParameters.restLength;  // scale pluck with string size
    if (location == -1) location = NUM_MASSES / 2;
    positions[location].z += d;
    positions[location - 1].z += d / 2;
    positions[location + 1].z += d / 2;
    positions[location - 2].z += d / 2;
    positions[location + 2].z += d / 2;
    /*for (int i = 0; i < NUM_MASSES; i++) {
        positions[i].z += 1;
    }*/

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ClothManager::CopySamplesToAudioBuffer() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, samplesSSbo);
    GLfloat *samples = (GLfloat *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, SAMPLES_BUFFER_SIZE * sizeof(GLfloat), GL_MAP_READ_BIT);
    unsigned int index = SoundManager::copyToSoundBuffer(samples, simParameters.numSamplesToGenerate);
    RayTracer::getInstance().shootThemAll(index);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ClothManager::RenderParticles(float dt, Environment *environment) {
    glBindBuffer(GL_ARRAY_BUFFER, ShaderManager::ClothShader.VBO);

    auto color = glm::vec3(101 / 255.0, 67 / 255.0, 33 / 255.0);
    glUniform3fv(ShaderManager::ClothShader.Attributes.color, 1, glm::value_ptr(color));  // Update the color, if necessary

    glLineWidth(5);
    glDrawArrays(GL_LINE_STRIP, 0, NUM_MASSES);
    glBindVertexArray(ShaderManager::EnvironmentShader.VAO);
}
