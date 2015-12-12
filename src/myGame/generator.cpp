#include "generator.h"
#include "ground.h"

bool Generator::Update(Scene &scene, float dt) {
  // Accumulate time

  // Generator will be in charge of keeping 5 tiles in the game.
  while(numberOfTiles != maxTiles){
    auto ground = GroundPtr(new Ground());
    ground->position.x = Rand(lastTilePosition.x - 3.0f * tileScale, lastTilePosition.x + 3.0f * tileScale);
    ground->position.y = Rand(lastTilePosition.y - 15.0f, lastTilePosition.y + 5.0f);
    ground->position.z = Rand(lastTilePosition.z + 3.0f * tileScale, lastTilePosition.z + 3.0f * tileScale);
    ground->timeToDetonation = 100.0f;
    scene.objects.push_back(ground);

    lastTilePosition = ground->position;
    numberOfTiles ++;
  }

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