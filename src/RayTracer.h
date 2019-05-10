#pragma once
#include <glm/vec3.hpp>
class RayTracer {
   public:
    enum {
        RayNumber = 1024,
        ListenerRadius = 10,
        // use one of them to stop raytrace
        MaxReflection = 5,
        MaxListenDis = 420,
		SoundSpeed = 34,
    };
	class Ray {
        glm::vec3 _start;
        glm::vec3 _dir;
		int _index;
	  public:
        Ray(glm::vec3 start, glm::vec3 dir, int i) : _start(start), _dir(dir),_index(i) {}
        void trace(const float travelDis = 0);
        float hitListenr();
        bool hitWalls(float listenerDis, float);
    };

   private:
    RayTracer() {}
    static glm::vec3 listenerPoint;

   public:
    void shootThemAll(int index);
    static RayTracer& getInstance() {
        static RayTracer instance;
        return instance;
    }
};
