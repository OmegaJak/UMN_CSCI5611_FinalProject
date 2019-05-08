#include "RayTracer.h"
#include <SoundManager.h>
#include <cmath>
#include "Camera.h"
#include "minmax.h"
#include <iostream>

glm::vec3 RayTracer::listenerPoint;
void RayTracer::shootThemAll() {
    // TODO: get current source positon from cloth manager
    const glm::vec3 sourcPoint = glm::vec3(0, 0, 0);

    listenerPoint = Camera::getInstance().GetPosition();
    listenerPoint.z = 0; // de Dimesionalize

    const float AngleStep = 2 * M_PI / RayNumber;
    float angle = 0;
    for (int i = 0; i < RayNumber; ++i) {
        Ray ray(glm::vec3(sourcPoint.x, sourcPoint.y, 0), glm::vec3(cos(angle), sin(angle), 0));
        ray.trace();
        angle += AngleStep;
    }
}

float RayTracer::Ray::hitListenr() {
    float projecLength = glm::dot(_dir, (listenerPoint - _start));
	if(projecLength <= 0|| projecLength > MaxListenDis) {
		//prune useless calculation
    } else if (glm::distance(_start + projecLength * _dir, listenerPoint) <= ListenerRadius) 
		return projecLength;
    
    return MaxListenDis;
}

float RayTracer::Ray::hitWal(float listenerDis, int step) {
    return MaxListenDis;
	// if Hit wall recursive raytrace here
}

void RayTracer::Ray::trace(int step) {
    float lDis = hitListenr();
    float wDis = hitWal(lDis,step);
    
    if (lDis < wDis) {
        // add current to buffer
        SoundManager::addBuffer();
    }
}
