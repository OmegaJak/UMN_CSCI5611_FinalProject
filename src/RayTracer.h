#pragma once
#include <glm/vec3.hpp>
class RayTracer {
   public:
    enum {
        RayNumber = 1024,
		SoundSpeed = 340,
    };
	static const float ListenerRadius;
	static const float SoundLastingTime;
	static const float MaxListenDis;
	class Ray {
        glm::vec3 _start;
        glm::vec3 _dir;
		int _index;
	  public:
        Ray(glm::vec3 start, glm::vec3 dir, int i) : _start(start), _dir(dir),_index(i) {}
        void trace(const float travelDis = 0, const int step = 1);
        float hitListenr();
        bool hitWalls(float listenerDis, float, const int Step);
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
