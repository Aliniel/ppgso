#include "player.h"
#include "scene.h"

#include "object_frag_shader.h"
#include "object_vert_shader.h"
#include "ground.h"

#include <GLFW/glfw3.h>

Player::Player() {
  // Scale the default model
//  rotation.x = -PI/2;
  scale *= 3.0f;

  // Initialize static resources if needed
  if (!shader) shader = ShaderPtr(new Shader{object_vert_shader, object_frag_shader});
  if (!texture) texture = TexturePtr(new Texture{"player/sphere.rgb", 1024, 512});
  if (!mesh) mesh = MeshPtr(new Mesh{shader, "player/sphere.obj"});
}

Player::~Player() {
}

void Player::rotate(float xpos){
  if(xpos > 0){
    currentRotation = rotationSpeed;
  }
  else if (xpos < 0){
    currentRotation = -rotationSpeed;
  }
  else{
    currentRotation = 0;
  }
}

bool Player::Update(Scene &scene, float dt) {
//  position.x ++;
//  position.z ++;
  float groundX, groundY, groundZ;
  float groundScale;
  float distance, dx, dz;

  for (auto obj : scene.objects) {
    // Ignore self in scene
    if (obj.get() == this)
      continue;

    // Collision detection
    auto ground = std::dynamic_pointer_cast<Ground>(obj);
    auto landmine = std::dynamic_pointer_cast<Landmine>(obj);

    if (ground){
      // Find the platform I'm over or under.
      if (abs(position.x - ground->position.x) < ground->scale.x &&
          abs(position.z - ground->position.z) < ground->scale.y) {
        if (position.y < ground->position.y) {
          rip = true;
        }
        groundX = ground->position.x;
        groundY = ground->position.y;
        groundZ = ground->position.z;
        groundScale = ground->scale.x;

        // If I stand on the tile, it will start to self destruct.
        if (position.y - 0.5f * scale.y == groundY) {
          if (!ground->selfDestruct) {
            ground->timeToDetonation = 2.0f;
//            ground->selfDestruct = true;
          }
        }

        // Check if I stepped on any landmines.
        for(int i = 0; i < 3; i ++){
          if(glm::distance(position, ground->mines[i].position) < 0.5f * scale.y + 0.5 * ground->mines[i].scale.y){
            jumpSpeed = 3 * jumpPower;
          }
        }
        break;
      }
      else {
        groundY = -50.0f;
      }
    }
  }

  jumpSpeed += gravity * dt;
  position.y += jumpSpeed * dt;

  if(position.y - 0.5f * scale.y < groundY && !rip) {
    jumpSpeed = 0;
    position.y = groundY + 0.5f * scale.y;
    inAir = false;
  }

  if(scene.keyboard[GLFW_KEY_W]) {
    currentMove = movementSpeed;
    rotation.x += 1.5f * rotationSpeed * dt;
  } else if(scene.keyboard[GLFW_KEY_S]) {
    currentMove = -movementSpeed;
    rotation.x -= 1.5f * rotationSpeed * dt;
  }
  else{
    currentMove = 0;
  }

  rotation.y += currentRotation * dt;
  distance = currentMove * dt;
  dx = distance * (float)sin(rotation.y * PI/180.0f);
  dz = distance * (float)cos(rotation.y * PI/180.0f);
  position += glm::vec3(dx, 0, dz);

  if(scene.keyboard[GLFW_KEY_SPACE]) {
    if(!inAir) {
      jumpSpeed = jumpPower;
      inAir = true;
    }
  }

  modelMatrix =
          glm::translate(glm::mat4(1.0f), position)
          * glm::rotate(glm::mat4(1.0f), rotation.y * PI/180.0f, glm::vec3(0, 1.0f, 0))
            * glm::rotate(glm::mat4(1.0f), rotation.x * PI/180.0f, glm::vec3(1.0f, 0, 0))
          * glm::scale(glm::mat4(1.0f), scale);

  return true;
}

void Player::Render(Scene &scene) {
  shader->Use();

  // use camera
  shader->SetMatrix(scene.camera->projectionMatrix, "ProjectionMatrix");
  shader->SetMatrix(scene.camera->viewMatrix, "ViewMatrix");

  // render mesh
  shader->SetMatrix(modelMatrix, "ModelMatrix");
  shader->SetTexture(texture, "Texture");
  mesh->Render();
}

// shared resources
MeshPtr Player::mesh;
ShaderPtr Player::shader;
TexturePtr Player::texture;