#define GLM_FORCE_RADIANS

#include <SDL_stdinc.h>

#include <algorithm>
#include <ctime>
#include <glm/gtc/type_ptr.hpp>

#include "ClothManager.h"
#include "Constants.h"
#include "Environment.h"
#include "RayTracer.h"
#include "ShaderManager.h"
#include "SoundManager.h"
#include "Timer.h"
#include "Utils.h"
#include "glad.h"

const GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

GLuint ClothManager::posSSbo;
GLuint ClothManager::velSSbo;
GLuint ClothManager::accelSSbo;
GLuint ClothManager::samplesSSbo;
GLuint ClothManager::massSSbo;
GLuint ClothManager::stringParamSSbo;
GLuint ClothManager::globalSimParamsSSbo;
bool ClothManager::ready = false;

ClothManager::ClothManager() {
    srand(time(NULL));
    GenerateStringParams();
    globalSimParameters = simParams{SAMPLES_PER_FRAME};
    InitGL();
}

void ClothManager::GenerateStringParams() {
    for (int i = 0; i < 8; i++) {
        stringParameters[i] = stringParams{dt, baseKs + i * deltaKs, kd, restLength, MASSES_PER_STRING / 2, MASSES_PER_STRING / 3};
    }
}

void ClothManager::InitGL() {
    // Prepare the positions buffer //
    glGenBuffers(1, &posSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TOTAL_NUM_MASSES * sizeof(position), nullptr, GL_DYNAMIC_DRAW);

    printf("Initializing mass positions...\n");
    position *positions = (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, TOTAL_NUM_MASSES * sizeof(position), bufMask);
    for (int stringIndex = 0; stringIndex < NUM_STRINGS; stringIndex++) {
        for (int i = 0; i < MASSES_PER_STRING; i++) {
            int index = stringIndex * MASSES_PER_STRING + i;
            positions[index] = {float(stringIndex), float(i), 1.0f,
                                BASE_HEIGHT};  // TODO: Why am I setting w here and is this messing things up (probably not)
        }
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the mass parameters buffer //
    glGenBuffers(1, &massSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, massSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TOTAL_NUM_MASSES * sizeof(massParams), nullptr, GL_STATIC_DRAW);

    printf("Initializing springs...\n");
    massParams *massParameters =
        (massParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, TOTAL_NUM_MASSES * sizeof(massParams), bufMask);
    for (int stringIndex = 0; stringIndex < NUM_STRINGS; stringIndex++) {
        for (int i = 0; i < MASSES_PER_STRING; i++) {
            int index = stringIndex * MASSES_PER_STRING + i;
            massParameters[index].isFixed = (i == 0 || i == MASSES_PER_STRING - 1);
            // massParameters[i].isFixed = false;
            massParameters[index].mass = 10;

            // Initialize connections
            unsigned int left = BAD_INDEX, right = BAD_INDEX;
            if (i > 0) {
                left = (stringIndex * MASSES_PER_STRING) + i - 1;
            }

            if (i < MASSES_PER_STRING - 1) {
                right = (stringIndex * MASSES_PER_STRING) + i + 1;
            }

            massParameters[index].connections = {left, right};
        }
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the samples buffer (just zero it out) //
    glGenBuffers(1, &samplesSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, samplesSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, GPU_SAMPLES_BUFFER_SIZE * sizeof(GLfloat), nullptr, GL_DYNAMIC_READ);

    GLfloat *samples = (GLfloat *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, GPU_SAMPLES_BUFFER_SIZE * sizeof(GLfloat), bufMask);
    memset(samples, 0, GPU_SAMPLES_BUFFER_SIZE * sizeof(GLfloat));

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the velocities buffer
    glGenBuffers(1, &velSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TOTAL_NUM_MASSES * sizeof(velocity), nullptr, GL_STATIC_DRAW);

    printf("Initializing mass velocities\n");
    velocity *vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, TOTAL_NUM_MASSES * sizeof(velocity), bufMask);
    memset(vels, 0, TOTAL_NUM_MASSES * sizeof(velocity));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the new velocities buffer
    glGenBuffers(1, &accelSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, accelSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TOTAL_NUM_MASSES * sizeof(velocity), nullptr, GL_STATIC_DRAW);

    vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, TOTAL_NUM_MASSES * sizeof(velocity), bufMask);
    memset(vels, 0, TOTAL_NUM_MASSES * sizeof(velocity));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Per-string parameters //
    glGenBuffers(1, &stringParamSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stringParamSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_STRINGS * sizeof(stringParams), nullptr, GL_STATIC_DRAW);

    stringParams *strParameters =
        (stringParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_STRINGS * sizeof(stringParams), bufMask);
    memcpy(strParameters, stringParameters, NUM_STRINGS * sizeof(stringParams));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Global simulation parameters //
    glGenBuffers(1, &globalSimParamsSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, globalSimParamsSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(simParams), nullptr, GL_STATIC_DRAW);

    simParams *simParameters = (simParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(simParams), bufMask);
    *simParameters = globalSimParameters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    printf("Done initializing buffers\n");
}

void ClothManager::UpdateComputeParameters() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stringParamSSbo);
    stringParams *params = (stringParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_STRINGS * sizeof(stringParams), bufMask);
    memcpy(params, stringParameters, NUM_STRINGS * sizeof(stringParams));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ClothManager::ExecuteComputeShader() {
    if (ready) {
        SoundManager::sumSoundsOntime();
        CopySamplesToAudioBuffer();
    }
    UpdateComputeParameters();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, samplesSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, stringParamSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, accelSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, massSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, globalSimParamsSSbo);

    glUseProgram(ShaderManager::ClothComputeShader);

    glDispatchCompute(NUM_STRINGS, 1, 1);            // Run the cloth sim compute shader
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

void ClothManager::Pluck(int stringIndex, float strength, int location) {
    assert(stringIndex < NUM_STRINGS);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    position *positions =
        (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, TOTAL_NUM_MASSES * sizeof(position), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

    double d = strength * stringParameters[stringIndex].restLength;  // scale pluck with string size
    if (location == -1) location = (stringIndex * MASSES_PER_STRING) + (MASSES_PER_STRING / 2);
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
    GLfloat *samples = (GLfloat *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, GPU_SAMPLES_BUFFER_SIZE * sizeof(GLfloat), GL_MAP_READ_BIT);
    memset(samplesBuffer, 0, SAMPLES_PER_FRAME * sizeof(float));

    // Add together all string's samples into just one buffer
    for (int stringIndex = 0; stringIndex < NUM_STRINGS; stringIndex++) {  // This may be slow
        int offset = stringIndex * SAMPLES_PER_FRAME;
        for (int i = 0; i < SAMPLES_PER_FRAME; i++) {
            samplesBuffer[i] += samples[offset + i];
        }
    }

    unsigned int index = SoundManager::storeSample(samplesBuffer);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    RayTracer::getInstance().shootThemAll(index);
}

void ClothManager::RenderParticles(float dt) {
    glBindBuffer(GL_ARRAY_BUFFER, ShaderManager::ClothShader.VBO);

    auto color = glm::vec3(101 / 255.0, 67 / 255.0, 33 / 255.0);
    glUniform3fv(ShaderManager::ClothShader.Attributes.color, 1, glm::value_ptr(color));  // Update the color, if necessary

    glLineWidth(5);
    glDrawArrays(GL_LINE_STRIP, 0, TOTAL_NUM_MASSES);
    glBindVertexArray(ShaderManager::EnvironmentShader.VAO);
}
