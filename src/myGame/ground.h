#ifndef PPGSO_GROUND_H
#define PPGSO_GROUND_H

#include <texture.h>
#include <shader.h>
#include <mesh.h>

#include "object.h"
#include "landMine.h"

// Simple object representing the player
// Reads keyboard status and manipulates its own position
// On Update checks collisions with Asteroid objects in the scene
class Ground : public Object {
public:
  Ground();
  ~Ground();

  bool Update(Scene &scene, float dt) override;
  void Render(Scene &scene) override;

  float timeToDetonation;
  bool selfDestruct = false;
  Landmine mines[3];

private:
  bool minesGenerated = false;

  // Static resources (Shared between instances)
  static MeshPtr mesh;
  static ShaderPtr shader;
  static TexturePtr texture;
};
typedef std::shared_ptr< Ground > GroundPtr;

#endif //PPGSO_GROUND_H
