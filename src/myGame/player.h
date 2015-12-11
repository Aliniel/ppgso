#ifndef PPGSO_PLAYER_H
#define PPGSO_PLAYER_H

#include <texture.h>
#include <shader.h>
#include <mesh.h>
#include <glm/gtx/euler_angles.hpp>

#include "object.h"

// Simple object representing the player
// Reads keyboard status and manipulates its own position
// On Update checks collisions with Asteroid objects in the scene
class Player : public Object {
public:
  Player();
  ~Player();

  void rotate(float xpos);
  bool Update(Scene &scene, float dt) override;
  void Render(Scene &scene) override;

private:
//    glm::vec3 rotationAxis = glm::vec3(0,0,0);

    glm::mat4 orientation = glm::orientate4(position);

    // Movement
    char movementSpeed = 15.0f;
    char currentMove = 0;

    // Rotation
    float rotationSpeed = 250.0f;
    float currentRotation = 0;

    // Angle around Y axis.
    float angle = 0;

    // Jumping
    bool jumping = false;
    bool jumped = false;
    float jumpSpeed = 20.0f;
    float jumpHeigth = 8.0f;
    float maxHeight = position.y + jumpHeigth;

  // Static resources (Shared between instances)
  static MeshPtr mesh;
  static ShaderPtr shader;
  static TexturePtr texture;
};
typedef std::shared_ptr< Player > PlayerPtr;

#endif //PPGSO_PLAYER_H
