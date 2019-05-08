#include "SDL.h"
#include "StringSim.h"

StringSim::StringSim(StringParameters stringParams, int numVertices, SoundManager* soundManager)
    : _stringParameters(stringParams), _soundManager(soundManager), _numVertices(numVertices) {
    posX = new double[numVertices];
    posY = new double[numVertices];
    velX = new double[numVertices];
    velY = new double[numVertices];
    accX = new double[numVertices];
    accY = new double[numVertices];
    InitVertices();
}

void StringSim::Update(double dt, int numTimesToUpdate, bool audio) {
    if (_soundManager->lastSimulationSampleIndex - _soundManager->lastSampleIndex > 2 * 2048 ||
        _soundManager->lastSimulationSampleIndex + numTimesToUpdate >= SoundManager::soundBuffSize)
        return;  // Simulate ahead to keep a small buffer of sound
    for (int z = 0; z < numTimesToUpdate; z++) {
        for (int q = 0; q < 10; q++) {
            for (int i = 0; i < _numVertices; i++) {  // Zero out acceleration
                accX[i] = 0;
                accY[i] = 0;
            }

            for (int i = 0; i < _numVertices - 1; i++) {  // Compute new acceleration F=ma + Hook's Law
                double xlen = posX[i + 1] - posX[i];
                double ylen = posY[i + 1] - posY[i];
                double leng = sqrt(xlen * xlen + ylen * ylen);
                double force = (_stringParameters.k / restLen) * (leng - restLen);
                double forceX = xlen / leng;
                double forceY = ylen / leng;
                double aX = forceX * force;
                double aY = forceY * force;
                // if i == 0: print(ittr,force,leng,forceX)
                aX += _stringParameters.kV * (velX[i + 1] - velX[i]);
                aY += _stringParameters.kV * (velY[i + 1] - velY[i]);
                accX[i] += aX / 2;
                accY[i] += aY / 2;
                accX[i + 1] += -aX / 2;
                accY[i + 1] += -aY / 2;
            }

            // printf("X: %f %f %f\n",posY[_numVertices/2],accY[_numVertices/2],velY[_numVertices/2]);

            for (int i = 1; i < _numVertices - 1; i++) {  // Eulerian integration
                velX[i] += accX[i] * dt;
                velY[i] += accY[i] * dt;
                posX[i] += velX[i] * dt;
                posY[i] += velY[i] * dt;
            }
        }

        if (audio) {  // Allow us to turn off the sound (for debugging)
            int micPos = _numVertices / 2;
            int sideDist = _numVertices / 3;
            SDL_LockAudio();
            _soundManager->soundBuffs[0][_soundManager->lastSimulationSampleIndex++] =
                .5 * velY[micPos] + .25 * velY[micPos - sideDist] + .25 * velY[micPos + sideDist];
            SDL_UnlockAudio();
        }
        // printf("X: %f\n",velY[_numVertices/2]); //Print velocity for debugging
    }

    // Update visuals to match simulation
    /*for (int i = 0, v = 0; i < _numVertices * 5; i += 5, v++) {
        vertices[i] = posX[v];
        vertices[i + 1] = posY[v];
        vertices[i + 2] = 1.0f;
        vertices[i + 3] = 0;
        vertices[i + 4] = 0;
    }*/
}

void StringSim::Pluck(float strength, int location) {
    double d = strength * restLen;  // scale pluck with string size
    if (location == -1) location = _numVertices / 2;
    posY[location] += d;
    posY[location - 1] += d / 2;
    posY[location + 1] += d / 2;
    posY[location - 2] += d / 2;
    posY[location + 2] += d / 2;
}

void StringSim::InitVertices() {
    for (int i = 0; i < _numVertices; i++) {
        // posX[i] = (length*(i/(numV+0.f) - .5 + .5/(numV+0.f)));
        posX[i] = (_stringParameters.length * (i / (_numVertices + 0.f))) - _stringParameters.length / 2;
        // printf("%f\n",posX[i]);  //Node positions
        posY[i] = .5;
        velX[i] = 0;
        velY[i] = 0;
    }

    // Make sure the string is under tension
    restLen = _stringParameters.tension * ((posX[_numVertices - 1] - posX[0]) / (_numVertices - 1));
}
