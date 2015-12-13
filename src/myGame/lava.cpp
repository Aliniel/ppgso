#include "scene.h"
#include "object_frag_shader.h"
#include "object_vert_shader.h"
#include "lava.h"

#include <GLFW/glfw3.h>

Lava::Lava() {
  rotation.x = -PI/2.0f;
  scale.x *= 150.0f;
  scale.y *= 150.0f;
  position.y = -40.0f;

  // Initialize static resources if needed
  if (!shader) shader = ShaderPtr(new Shader{object_vert_shader, object_frag_shader});
  if (!texture) texture = TexturePtr(new Texture{"lava/lava.rgb", 1024, 1024});
  if (!mesh) mesh = MeshPtr(new Mesh{shader, "ground/quad.obj"});
}

Lava::~Lava() {
}

bool Lava::Update(Scene &scene, float dt) {
  position.x = player->position.x;
  position.z = player->position.z;

  GenerateModelMatrix();
  return true;
}

void Lava::Render(Scene &scene) {
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
MeshPtr Lava::mesh;
ShaderPtr Lava::shader;
TexturePtr Lava::texture;