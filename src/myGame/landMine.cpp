#include "object_frag_shader.h"
#include "object_vert_shader.h"
#include "landMine.h"
#include "scene.h"

#include <GLFW/glfw3.h>

Landmine::Landmine() {

  // Initialize static resources if needed
  if (!shader) shader = ShaderPtr(new Shader{object_vert_shader, object_frag_shader});
  if (!texture) texture = TexturePtr(new Texture{"landmine/landmine.rgb", 1000, 800});
  if (!mesh) mesh = MeshPtr(new Mesh{shader, "player/sphere.obj"});
}


Landmine::~Landmine() {
}

bool Landmine::Update(Scene &scene, float dt) {
  GenerateModelMatrix();
  return true;
}

void Landmine::Render(Scene &scene) {
  if(destroyed){
    return;
  }
  shader->Use();

  // use camera
  shader->SetMatrix(scene.camera->projectionMatrix, "ProjectionMatrix");
  shader->SetMatrix(scene.camera->viewMatrix, "ViewMatrix");

  // render mesh
  shader->SetMatrix(modelMatrix, "ModelMatrix");
  shader->SetTexture(texture, "Texture");

  shader->SetVector(glm::vec3(scene.camera->player->position.x, scene.camera->player->position.y + 5.0f, scene.camera->player->position.z), "lightPosition");
  shader->SetVector(scene.camera->position, "viewPosition");

  mesh->Render();
}

// shared resources
MeshPtr Landmine::mesh;
ShaderPtr Landmine::shader;
TexturePtr Landmine::texture;