#pragma once
#include "GameObject.h"

class Environment {
   public:
    
	static Environment& getInstance() {
        static Environment instance;
        return instance;
    }
    void UpdateAll();
    void SetGravityCenterPosition(const glm::vec3& position);
	struct Wall : public GameObject{
		glm::vec3 _position;
		glm::vec3 _normal;
		float _length;
		Wall(glm::vec3 pos, glm::vec3 n, float l, Model* model):GameObject(model), _position(pos), _normal(n), _length(l){
			SetTextureIndex(UNTEXTURED);
		    SetColor(glm::vec3(0, 0, 0));
			SetPosition(_position);
			glm::normalize(_normal);
			// TODO: rotation doesn't work...
			EulerRotate(45,.0f,.0f);

			SetScale(-l*_normal.y, -l* _normal.x, l);
		}
		Wall() = default;

	};

	std::vector<Wall>& getWalls(){return _walls;}
   private:
	Environment();
    void CreateEnvironment();

    std::vector<GameObject> _gameObjects;
	std::vector<Wall> _walls;// TODO: try to unify these two
    int _gravityCenterIndex;
    Model* _cubeModel;
    Model* _sphereModel;
};
