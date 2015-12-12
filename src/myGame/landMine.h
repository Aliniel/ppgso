#ifndef PPGSO_LANDMINE_H
#define PPGSO_LANDMINE_H

#include <texture.h>
#include <shader.h>
#include <mesh.h>

#include "object.h"

// Simple object representing the player
// Reads keyboard status and manipulates its own position
// On Update checks collisions with Asteroid objects in the scene
class Landmine : public Object {
public:
    Landmine();
    ~Landmine();

    bool Update(Scene &scene, float dt) override;
    void Render(Scene &scene) override;

    bool destroyed = false;
private:
    // Static resources (Shared between instances)
    static MeshPtr mesh;
    static ShaderPtr shader;
    static TexturePtr texture;
};
typedef std::shared_ptr< Landmine > LandminePtr;

#endif //PPGSO_LANDMINE_H
