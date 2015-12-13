#include "ground.h"
#include "scene.h"
#include "object_frag_shader.h"
#include "object_vert_shader.h"
#include "generator.h"

#include <GLFW/glfw3.h>

Ground::Ground() {
  rotation.x = -PI/2.0f;
  scale.x *= 10.0f;
  scale.y *= 10.0f;
  timeToDetonation = 100.0f;

  // Initialize static resources if needed
  if (!shader) shader = ShaderPtr(new Shader{object_vert_shader, object_frag_shader});
  if (!texture) texture = TexturePtr(new Texture{"ground/ground.rgb", 1024, 1024});
  if (!mesh) mesh = MeshPtr(new Mesh{shader, "ground/quad.obj"});
}


Ground::~Ground() {
}

bool Ground::Update(Scene &scene, float dt) {
  if(!minesGenerated){
    for(int i = 0; i < 3; i++){
      mines[i].position.x = Rand(position.x - scale.x, position.x + scale.x);
      mines[i].position.z = Rand(position.z - scale.y, position.z + scale.y);
      mines[i].position.y = position.y;
    }
    minesGenerated = true;
  }

  if(selfDestruct){
    timeToDetonation -= dt;
    if(timeToDetonation < 0.0f){
      for( auto obj : scene.objects ){
        // Giving Generator signal to generate another tile.
        if (obj.get() == this)
          continue;

        auto generator = std::dynamic_pointer_cast<Generator>(obj);
        if (!generator) continue;

        generator->numberOfTiles --;
      }
      return false;
    }
  }

  GenerateModelMatrix();
  for(int i = 0; i < 3; i++){
    mines[i].Update(scene, dt);
  }
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

  shader->SetVector(glm::vec3(scene.camera->player->position.x, scene.camera->player->position.y + 5.0f, scene.camera->player->position.z), "lightPosition");
  shader->SetVector(scene.camera->position, "viewPosition");

  mesh->Render();

  for(int i = 0; i < 3; i++){
    mines[i].Render(scene);
  }
}

// shared resources
MeshPtr Ground::mesh;
ShaderPtr Ground::shader;
TexturePtr Ground::texture;