//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_SCENE_H
#define MG1_SCENE_H


#include <QImage>
#include <list>
#include "Object/object.h"
#include "Camera/camera.h"
#include "Ellipsoid/ellipsoid.h"

class Scene {
    Ellipsoid ellipsoid;
    Camera camera;

public:
    explicit Scene(Ellipsoid &&ellipsoid);

    void draw(QImage &target) const;

    [[nodiscard]] Camera &getCamera() { return camera; }

    Ellipsoid &getEllipsoid() { return ellipsoid; }
};


#endif //MG1_SCENE_H
