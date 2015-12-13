//
// Created by Branislav on 13/12/2015.
//

#ifndef PPGSO_LAVA_H
#define PPGSO_LAVA_H

#include "scene.h"

class Lava : public Object {
public:
    Lava();
    ~Lava();

    bool Update(Scene &scene, float dt) override;
    void Render(Scene &scene) override;

    Player *player;

private:

    // Static resources (Shared between instances)
    static MeshPtr mesh;
    static ShaderPtr shader;
    static TexturePtr texture;
};
typedef std::shared_ptr< Lava > LavaPtr;

#endif //PPGSO_LAVA_H
