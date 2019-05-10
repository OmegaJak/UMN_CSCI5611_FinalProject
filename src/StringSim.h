#pragma once
#include <vector>


struct StringParameters {
    double k, kV, tension, length;
};

class StringSim {
   public:
    StringSim(StringParameters stringParams, int numVertices);
    void Update(double dt, int numTimesToUpdate = 1, bool audio = false);

    void Pluck(float strength = 0.1, int location = -1);

   private:
    void InitVertices();

    StringParameters _stringParameters;
    int _numVertices;
    double restLen;
    double *posX, *posY, *velX, *velY, *accX, *accY;
};
