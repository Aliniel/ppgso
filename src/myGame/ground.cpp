#include "ground.h"
#include "scene.h"
//#include "asteroid.h"
//#include "projectile.h"
//#include "explosion.h"

#include "object_frag_shader.h"
#include "object_vert_shader.h"

#include <GLFW/glfw3.h>

Ground::Ground() {
  rotation.x = -PI/2.0f;
  scale.x *= 10.0f;
  scale.y *= 10.0f;
  scale.z *= 2.0f;

  // Initialize static resources if needed
  if (!shader) shader = ShaderPtr(new Shader{object_vert_shader, object_frag_shader});
  if (!texture) texture = TexturePtr(new Texture{"ground/ground.rgb", 1024, 1024});
//  if (!mesh) mesh = MeshPtr(new Mesh{shader, "fy_snow/fy_snow/fy_snow.obj"});
  if (!mesh) mesh = MeshPtr(new Mesh{shader, "ground/quad.obj"});
}


Ground::~Ground() {
}

bool Ground::Update(Scene &scene, float dt) {

  if(scene.keyboard[GLFW_KEY_U]) {
    position.y += 10.0f * dt;
  }
  if(scene.keyboard[GLFW_KEY_J]) {
    position.y -= 10.0f * dt;
  }

  GenerateModelMatrix();
  return true;
}

void Ground::Render(Scene &scene) {
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
MeshPtr Ground::mesh;
ShaderPtr Ground::shader;
TexturePtr Ground::texture;