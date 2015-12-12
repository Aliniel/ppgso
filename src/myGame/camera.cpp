#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "player.h"

Camera::Camera(float fow, float ratio, float near, float far) {
  float fowInRad = (PI/180.0f) * fow;

  projectionMatrix = glm::perspective(fowInRad, ratio, near, far);

  up = glm::vec3(0,1.0f,0);
  position = glm::vec3(0,15,15);
  forward = glm::vec3(0,0,1.0f);
}

Camera::~Camera() {
}

void Camera::Update() {
  // Calculating horizontal and vertical distances from player:
  float horizontalDistance = distance * (float)cos(pitch * PI/180.0f);
  float verticalDistance = distance * (float)sin(pitch * PI/180.0f);

  // Calculating the position of the camera:
  position.x = player->position.x - horizontalDistance * (float)sin(player->rotation.y * PI/180.0f);
  position.y = player->position.y + verticalDistance;
  position.z = player->position.z - horizontalDistance * (float)cos(player->rotation.y * PI/180.0f);

  if(pitch < 0){
    pitch = 0;
  }
  else if(pitch > 80.0f){
    pitch = 80.0f;
  }
  if(dead){
    viewMatrix = glm::lookAt(lookAtExplosion - glm::vec3(10.0f, 10.0f, 10.0f), lookAtExplosion, up);
  }
  else {
    viewMatrix = glm::lookAt(position, player->position, up);
  }
}