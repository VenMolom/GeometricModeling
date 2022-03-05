//
// Created by Molom on 2022-02-27.
//

#include "scene.h"

Scene::Scene() : ellipsoid(), camera() {
}

Scene::Scene(Camera &&camera) noexcept: ellipsoid(), camera(std::move(camera)) {
}

void Scene::draw(QImage &target) const {
    ellipsoid.draw(target, camera.viewMatrix(), camera.getPosition());
}
