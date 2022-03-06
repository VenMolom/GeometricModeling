//
// Created by Molom on 2022-02-27.
//

#include "scene.h"

Scene::Scene(Ellipsoid &&ellipsoid) : ellipsoid(ellipsoid), camera() {

}

void Scene::draw(QImage &target) const {
    ellipsoid.draw(target, camera.viewMatrix(), camera.getPosition());
}
