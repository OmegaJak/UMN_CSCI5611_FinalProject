#pragma once
#include <glm/vec3.hpp>
class RayTracer {
	enum {
		RayNumber = 1024,
		ListenerRadius = 10,
		// use one of them to stop raytrace
		MaxReflection = 5,
		MaxListenDis = 420,
	};
    RayTracer() {}
    class Ray {
        glm::vec3 _start;
        glm::vec3 _dir;

       public:
        Ray(glm::vec3 start, glm::vec3 dir) : _start(start), _dir(dir){}
        void trace(int step = MaxReflection);
        float hitListenr();
		float hitWal(float listenerDis, int step);
	};
    static glm::vec3 listenerPoint;

   public:
    void shootThemAll();
    static RayTracer& getInstance() {
        static RayTracer instance;
        return instance;
    }
};
