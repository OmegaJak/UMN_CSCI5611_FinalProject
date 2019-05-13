#include "ClothManager.h"
#include "Environment.h"
#include "Utils.h"

Environment::Environment() {
    _cubeModel = new Model("models/cube.txt");
    _sphereModel = new Model("models/sphere.txt");

    CreateEnvironment();
}

void Environment::UpdateAll() {
    for (auto gameObject : _gameObjects) {
        gameObject.Update();
    }
    for (auto gameObject : _walls) {
        gameObject.Update();
    }
}

void Environment::SetGravityCenterPosition(const glm::vec3& position) {
    _gameObjects[_gravityCenterIndex].SetPosition(position);
}

void Environment::CreateEnvironment() {
    // gameObject = GameObject(_cubeModel);  // ground
    // gameObject.SetTextureIndex(UNTEXTURED);
    // gameObject.SetColor(glm::vec3(0, 77 / 255.0, 26 / 255.0));
    // gameObject.SetScale(20, 20, 1);
    // gameObject.SetPosition(glm::vec3(0, 0, -0.55));
    // gameObject.material_.specFactor_ = 0.2;
    //_gameObjects.push_back(gameObject);

    /*
gameObject = GameObject(_cubeModel);  // reference person
gameObject.SetTextureIndex(TEX1);
gameObject.SetScale(1, 0, -3);
gameObject.SetPosition(glm::vec3(-25, 13, 1.5));
_gameObjects.push_back(gameObject);
    */
    const float WallScale = 42;
    // end walls near strings
    _walls.push_back(Wall(glm::vec3(-2 * WallScale, 0, 0), glm::vec3(1, 0, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(-2 * WallScale, 2 * WallScale, 0), glm::vec3(1, 0, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(-2 * WallScale, -2 * WallScale, 0), glm::vec3(1, 0, 0), 2 * WallScale, _cubeModel));

    // long side walls
    _walls.push_back(Wall(glm::vec3(-WallScale, 3 * WallScale, 0), glm::vec3(0, -1, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(-WallScale, -3 * WallScale, 0), glm::vec3(0, -1, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(WallScale, 3 * WallScale, 0), glm::vec3(0, -1, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(WallScale, -3 * WallScale, 0), glm::vec3(0, -1, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(3 * WallScale, 3 * WallScale, 0), glm::vec3(0, -1, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(3 * WallScale, -3 * WallScale, 0), glm::vec3(0, -1, 0), 2 * WallScale, _cubeModel));

    // doorway walls
    _walls.push_back(Wall(glm::vec3(4 * WallScale, 2 * WallScale, 0), glm::vec3(1, 0, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(4 * WallScale, -2 * WallScale, 0), glm::vec3(1, 0, 0), 2 * WallScale, _cubeModel));

    // side room side walls
    _walls.push_back(Wall(glm::vec3(5 * WallScale, 3 * WallScale, 0), glm::vec3(0, -1, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(5 * WallScale, -3 * WallScale, 0), glm::vec3(0, -1, 0), 2 * WallScale, _cubeModel));

    // side room end walls
    _walls.push_back(Wall(glm::vec3(6 * WallScale, 0, 0), glm::vec3(1, 0, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(6 * WallScale, 2 * WallScale, 0), glm::vec3(1, 0, 0), 2 * WallScale, _cubeModel));
    _walls.push_back(Wall(glm::vec3(6 * WallScale, -2 * WallScale, 0), glm::vec3(1, 0, 0), 2 * WallScale, _cubeModel));

    /*
    _walls.push_back(Wall(glm::vec3(-WallScale,WallScale,0), glm::vec3(1,0,0), 2*WallScale,_cubeModel));
    _walls.push_back(Wall(glm::vec3(-WallScale,-WallScale,0), glm::vec3(1,0,0), 2*WallScale,_cubeModel));
    _walls.push_back(Wall(glm::vec3(-WallScale,-3*WallScale,0), glm::vec3(1,0,0), 2*WallScale,_cubeModel));
    _walls.push_back(Wall(glm::vec3(-WallScale,3*WallScale,0), glm::vec3(1,0,0), 2*WallScale,_cubeModel));
    //_walls.push_back(Wall(glm::vec3(WallScale,0,0), glm::vec3(-1,0,0), 2*WallScale,_cubeModel));
    _walls.push_back(Wall(glm::vec3(0,4*WallScale,0), glm::vec3(0,-1,0), 2*WallScale,_cubeModel));
    _walls.push_back(Wall(glm::vec3(0,-4*WallScale,0), glm::vec3(0,-1,0), 2*WallScale,_cubeModel));
    _walls.push_back(Wall(glm::vec3(2*WallScale,4*WallScale,0), glm::vec3(0,-1,0), 2*WallScale,_cubeModel));
    _walls.push_back(Wall(glm::vec3(2*WallScale,-4*WallScale,0), glm::vec3(0,-1,0), 2*WallScale,_cubeModel));

    _walls.push_back(Wall(glm::vec3(3*WallScale,3*WallScale,0), glm::vec3(1,0,0), 2*WallScale,_cubeModel));
    _walls.push_back(Wall(glm::vec3(3*WallScale,-3*WallScale,0), glm::vec3(1,0,0), 2*WallScale,_cubeModel));
    _walls.push_back(Wall(glm::vec3(3*WallScale,-WallScale,0), glm::vec3(1,0,0), 2*WallScale,_cubeModel));
    */
    //_walls.push_back(Wall(glm::vec3(0,-WallScale,0), glm::vec3(0,1,0), 2*WallScale,_cubeModel));
    //_walls.push_back(Wall(glm::vec3(2*WallScale,WallScale,0), glm::vec3(0,1,0), 2*WallScale,_cubeModel));
    GameObject gameObject;
    // floor
    gameObject = GameObject(_cubeModel);
    gameObject.SetTextureIndex(UNTEXTURED);
    gameObject.SetColor(glm::vec3(1, 1, 1));
    gameObject.SetScale(12 * WallScale, 6 * WallScale, 1);
    gameObject.SetPosition(glm::vec3(0, 0, -WallScale));
    _gameObjects.push_back(gameObject);

    // ceiling
    gameObject = GameObject(_cubeModel);
    gameObject.SetTextureIndex(UNTEXTURED);
    gameObject.SetColor(glm::vec3(1, 1, 1));
    gameObject.SetScale(12 * WallScale, 6 * WallScale, 1);
    gameObject.SetPosition(glm::vec3(0, 0, WallScale));
    _gameObjects.push_back(gameObject);

    // string holders
    gameObject = GameObject(_cubeModel);
    gameObject.SetTextureIndex(UNTEXTURED);
    gameObject.SetColor(glm::vec3(0.5, 0.5, 0.5));
    gameObject.SetScale(8, 0.5, 0.5);
    gameObject.SetPosition(glm::vec3(3.5, 0, 1));
    _gameObjects.push_back(gameObject);

    gameObject = GameObject(_cubeModel);
    gameObject.SetTextureIndex(UNTEXTURED);
    gameObject.SetColor(glm::vec3(0.5, 0.5, 0.5));
    gameObject.SetScale(8, 0.5, 0.5);
    gameObject.SetPosition(glm::vec3(3.5, 31, 1));
    _gameObjects.push_back(gameObject);

    gameObject = GameObject(_sphereModel);
    gameObject.SetTextureIndex(UNTEXTURED);
    gameObject.SetColor(glm::vec3(0, 0, 0));
    gameObject.SetPosition(glm::vec3(1000, 1000, 1000));
    gameObject.SetScale(1, 1, 1);
    _gameObjects.push_back(gameObject);

    _gravityCenterIndex = _gameObjects.size() - 1;
}
