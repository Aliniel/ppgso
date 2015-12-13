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

  bool bombed = false;

    // Should I die?
    bool rip = false;
private:
    glm::mat4 orientation = glm::orientate4(position);

    // Movement
    float movementSpeed = 15.0f;
    float currentMove = 0;

    // Rotation
    float rotationSpeed = 250.0f;
    float currentRotation = 0;

    // Angle around Y axis.
    float angle = 0;

    // Jumping
    float gravity = -50.f;
    float jumpPower = 30.0f;
    float jumpSpeed = 0;
    bool inAir = false;

  // Static resources (Shared between instances)
  static MeshPtr mesh;
  static ShaderPtr shader;
  static TexturePtr texture;
};
typedef std::shared_ptr< Player > PlayerPtr;

#endif //PPGSO_PLAYER_H
