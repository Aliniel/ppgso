#include "generator.h"
#include "ground.h"

bool Generator::Update(Scene &scene, float dt) {
  // Accumulate time

  // Generator will be in charge of keeping 5 tiles in the game.
  while(numberOfTiles != maxTiles){
    auto ground = GroundPtr(new Ground());
    ground->position.x = Rand(lastTilePosition.x - 2.5f * tileScale, lastTilePosition.x + 2.5f * tileScale);
    ground->position.y = Rand(lastTilePosition.y - 5, lastTilePosition.y + 5);
    ground->position.z = Rand(lastTilePosition.z + 2.0f * tileScale, lastTilePosition.z + 2.5f * tileScale);
    ground->timeToDetonation = 100.0f;
    scene.objects.push_back(ground);

    lastTilePosition = ground->position;
    numberOfTiles ++;
  }

  // Add object to scene when time reaches certain level
//  if (time > .3) {
//    auto obj = AsteroidPtr(new Asteroid());
//    obj->position = this->position;
//    obj->position.x += Rand(-20, 20);
//    scene.objects.push_back(obj);
//    time = 0;
//  }

  return true;
}

void Generator::Render(Scene &scene) {
  // Generator will not be rendered
}

Generator::~Generator() {
}

Generator::Generator() {
  numberOfTiles = 1;
  lastTilePosition = glm::vec3(0, 0, 0);
}