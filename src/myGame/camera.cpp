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
//  pitch += dt * float(WIDTH/2 - scene.mouse.x);
//  jaw += dt * float(HEIGTH/2 - scene.mouse.y);

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
//  forward = glm::vec3(
//          cos(jaw) * sin(pitch),
//          sin(jaw),
//          cos(jaw) * cos(pitch)
//  );
//  right = glm::vec3(
//          sin(pitch - PI / 2.0f),
//          0,
//          cos(pitch - PI / 2)
//  );
//  up = glm::cross(right, forward);

//  position = glm::vec3(
////          player->position.x -
//  );

//  viewMatrix = glm::lookAt(position, position + forward, up);
  viewMatrix = glm::lookAt(position, player->position, up);
//  viewMatrix = glm::lookAt(position, glm::vec3(0,0,0), up);
}
