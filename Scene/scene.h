//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_SCENE_H
#define MG1_SCENE_H

#include <list>
#include "Objects/Object/object.h"
#include "Camera/camera.h"

class Scene {
public:
    explicit Scene() = default;

    void addObject(std::unique_ptr<Object> &&object);

    void draw(Renderer &renderer) const;

    Camera &getCamera() { return camera; }

private:
    std::vector<std::unique_ptr<Object>> objects;
    Camera camera;
};


#endif //MG1_SCENE_H
