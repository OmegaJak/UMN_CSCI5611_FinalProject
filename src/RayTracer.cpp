#include "RayTracer.h"
#include <SoundManager.h>
#include <cmath>
#include <iostream>
#include "Camera.h"
#include "minmax.h"
#include "Environment.h"
extern float timePassed;
glm::vec3 RayTracer::listenerPoint;
void RayTracer::shootThemAll(int index) {
    // TODO: get current source positon from cloth manager
    const glm::vec3 sourcPoint = glm::vec3(0, 0, 0);

    listenerPoint = Camera::getInstance().GetPosition();
    listenerPoint.z = 0;  // de Dimesionalize

    const float AngleStep = 2 * M_PI / RayNumber;
    float angle = 0;
    for (int i = 0; i < RayNumber; ++i) {
        Ray ray(glm::vec3(sourcPoint.x, sourcPoint.y, 0), glm::vec3(cos(angle), sin(angle), 0), index);
        ray.trace();
        angle += AngleStep;
    }
}

float RayTracer::Ray::hitListenr() {
    float projecLength = glm::dot(_dir, (listenerPoint - _start));
    if (projecLength <= 0 || projecLength > MaxListenDis) {
        // prune useless calculation
    } else if (glm::distance(_start + projecLength * _dir, listenerPoint) <= ListenerRadius)
        return projecLength;

    return MaxListenDis;
}

float random(){
	return (2*rand()-(float)RAND_MAX)/(float)RAND_MAX;
}
bool RayTracer::Ray::hitWalls(float listenerDis, float travelDis) {
	//return false;
	auto& walls = Environment::getInstance().getWalls();
	float minLength = listenerDis;
	glm::vec3 hitNorm;

	for(const auto& w:walls){
		float dis = glm::dot(-w._normal, (w._position-_start));
		float cosTheta = glm::dot(_dir,-w._normal);
		if(cosTheta == 0) continue;
		float segLine = dis/cosTheta;
		
		if(segLine >= 0 && segLine < minLength) {
			glm::vec3 hitSurfacePointToSurfacePosition = _start + segLine * _dir - w._position;
			if(abs(hitSurfacePointToSurfacePosition.x) >= w._length
				|| abs(hitSurfacePointToSurfacePosition.y) >= w._length)
				continue;
			minLength = std::min(minLength, dis/cosTheta);
			hitNorm = w._normal;
		}
	}
	
	if(minLength < listenerDis) {
		_start = _start + (minLength-0.001f)*_dir;
		_dir = -glm::dot(hitNorm, _dir) * hitNorm;
		//_dir=_dir/glm::length(_dir);
		const float ReflectScope = 4;
		float r = ReflectScope*random();
		_dir = glm::normalize(_dir);
		//std::cout<<_dir.x <<"y"<<_dir.y<<std::endl;
		//_dir.x += r*_dir.y;
		//_dir.y += r*_dir.x;
		//_dir = glm::normalize(_dir);
		
		
		trace(minLength + travelDis);
		return true;
	}
	return false;
}

void RayTracer::Ray::trace(const float TravelDis) {
	if(TravelDis > MaxListenDis) return;
    float lDis = hitListenr();
    if (!hitWalls(lDis, TravelDis) 
		&& (TravelDis + lDis < MaxListenDis) ){
		SoundManager::addBuffer(timePassed + (TravelDis+lDis)/SoundSpeed, _index);
    }
}