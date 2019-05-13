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
    struct Wall : public GameObject {
        glm::vec3 _position;
        glm::vec3 _normal;
        float _length;
        Wall(glm::vec3 pos, glm::vec3 n, float l, Model* model) : GameObject(model), _position(pos), _normal(n), _length(l) {
            SetTextureIndex(TEX1);
            SetColor(glm::vec3(0, 1, 0));
            SetPosition(_position);
            glm::normalize(_normal);
            // TODO: rotation doesn't work...
            // EulerRotate(45,.0f,.0f);

            SetScale(-l * _normal.y + 0.1f, -l * _normal.x + 0.1f, l);
        }
        Wall() = default;
    };

    const std::vector<Wall>& getWalls() {
        return _walls;
    }

   private:
    Environment();
    void CreateEnvironment();

    std::vector<GameObject> _stringHolders; // This is horrible and I'm sorry
    std::vector<GameObject> _gameObjects;
    std::vector<Wall> _walls;  // TODO: try to unify these two
    int _gravityCenterIndex;
    Model* _cubeModel;
    Model* _sphereModel;
};
