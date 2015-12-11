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
  float groundY;
  float distance, dx, dz;

  // Keyboard controls
//  if(scene.keyboard[GLFW_KEY_LEFT]) {
////    currentRotation = rotationSpeed;
////    position.x -= movementSpeed * dt;
////    rotation.y += rotationSpeed * dt;
//  } else if(scene.keyboard[GLFW_KEY_RIGHT]) {
////    currentRotation = -rotationSpeed;
////    position.x += movementSpeed * dt;
////    rotation.y -= rotationSpeed * dt;
//  }
//  else{
//    currentRotation = 0;
//  }

  if(scene.keyboard[GLFW_KEY_W]) {
    currentMove = movementSpeed;
//    position.z -= movementSpeed * dt;
    rotation.x += 1.5f * rotationSpeed * dt;
  } else if(scene.keyboard[GLFW_KEY_S]) {
    currentMove = -movementSpeed;
//    position.z += movementSpeed * dt;
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

  if(scene.keyboard[GLFW_KEY_SPACE] && !jumped) {
    jumping = true;
    jumped = true;
  }

  // Falling down.
  if(!jumping) {
    for (auto obj : scene.objects) {
      // Ignore self in scene
      if (obj.get() == this)
        continue;

      // We only need to collide with asteroids, ignore other objects
      auto ground = std::dynamic_pointer_cast<Ground>(obj);
      if (!ground) continue;

      // Find the platform I'm over or under.
      if (abs(position.x - ground->position.x) < ground->scale.x &&
          abs(position.z - ground->position.z) < ground->scale.y) {
        groundY = ground->position.y;

        // If I stand on the tile, it will start to self destruct.
        if(position.y == groundY + scale.y){
          if(!ground->selfDestruct) {
            ground->timeToDetonation = 2.0f;
            ground->selfDestruct = true;
          }
        }
        break;
      }
      else {
        groundY = -50.0f;
      }
    }

    if (position.y - scale.y != groundY) {
      if(position.y - scale.y < groundY) {
        position.y -= jumpSpeed * dt;
      }
      else{
        position.y -= jumpSpeed * dt;
        if(position.y - scale.y < groundY) {
          position.y = groundY + scale.y;
          if (jumped) {
            jumped = false;
            maxHeight = position.y - scale.y + jumpHeigth;
          }
        }
      }
    }
  }
    // Jumping - going up.
  else{
    if(position.y < maxHeight + scale.y)
      position.y += jumpSpeed * dt;
    if(position.y > maxHeight + scale.y)
      position.y = maxHeight + scale.y;
    if(position.y == maxHeight + scale.y)
      jumping = false;
  }


  modelMatrix =
          glm::translate(glm::mat4(1.0f), position)
//          * glm::orientate4(rotation)
//          * glm::rotate(glm::mat4(1.0f), rotation.x * PI/180, glm::vec3(1.0f, 0, 0))
          * glm::rotate(glm::mat4(1.0f), rotation.y * PI/180.0f, glm::vec3(0, 1.0f, 0))
            * glm::rotate(glm::mat4(1.0f), rotation.x * PI/180.0f, glm::vec3(1.0f, 0, 0))
          * glm::scale(glm::mat4(1.0f), scale);

//  GenerateModelMatrix();
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