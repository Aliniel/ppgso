#ifndef PPGSO_CAMERA_H
#define PPGSO_CAMERA_H

#include <memory>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec3.hpp>
#include "player.h"

#define PI 3.14159265358979323846f

// Simple camera object that keeps track of viewMatrix and projectionMatrix
// the projectionMatrix is by default constructed as perspective projection
// the viewMatrix is generated from up, position and back vectors on Update
class Camera {
public:
  Camera(float fow = 45.0f, float ratio = 16.0f/9.0f, float near = 0.1f, float far = 10.0f);
  ~Camera();

  void Update();

  Player *player;

  float mouseSpeed = 50.0f;

  float pitch = 15.0f;
  float jaw = 0;
  float distance = 20.0f;

  glm::vec3 up;
  glm::vec3 position;
  glm::vec3 forward;
  glm::vec3 right;

  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
};
typedef std::shared_ptr< Camera > CameraPtr;

#endif //PPGSO_CAMERA_H
