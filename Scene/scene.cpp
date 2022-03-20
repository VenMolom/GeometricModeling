//
// Created by Molom on 2022-02-27.
//

#include "scene.h"

using namespace DirectX;
using namespace std;

void Scene::draw(Renderer &renderer) const {
    if (composite) {
        composite->draw(renderer, _camera, SELECTED);
    }
    if (cursor) {
        cursor->draw(renderer, _camera, DEFAULT);
    }
    for (auto &object: objects) {
        auto a = _selected.value().lock().get();
        object->draw(renderer, _camera, a == object.get() ? SELECTED : DEFAULT);
    }

}

void Scene::addObject(shared_ptr<Object> &&object) {
    if (cursor) {
        object->setPosition(cursor->position());
        cursor.reset();
    }

    _selected = object;
    objects.push_back(std::move(object));
}


void Scene::removeObject(const std::shared_ptr<Object>& object) {
    objects.remove_if([&] (const shared_ptr<Object>& ob) { return ob.get() == object.get(); });
    composite.reset();
    _selected.setValue({});
}

void Scene::selectOrAddCursor(QPoint screenPosition) {
    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto screenSize = XMFLOAT2(_camera.viewport().width(), _camera.viewport().height());
    auto ray = Utils3D::getRayFromScreen(screenPos, screenSize, _camera.nearZ(), _camera.farZ(),
                                         _camera.projectionMatrix(), _camera.viewMatrix());

    if(auto object = findIntersectingObject(ray)) {
        setSelected(object);
    } else {
        addCursor(ray, screenPos);
    }
}

shared_ptr<Object> Scene::findIntersectingObject(Utils3D::XMFLOAT3RAY ray) {
    shared_ptr<Object> closest{nullptr};
    float closestDistance = INFINITY;

    for (auto &object : objects) {
        float distance{};
        if (object->boundingBox().Intersects(XMLoadFloat3(&ray.position), XMLoadFloat3(&ray.direction), distance) && distance < closestDistance){
            closest = object;
            closestDistance = distance;
        }
    }

    return closest;
}

void Scene::addCursor(Utils3D::XMFLOAT3RAY ray, XMINT2 screenPos) {
    auto plane = Utils3D::getPerpendicularPlaneThroughPoint(_camera.direction(), _camera.center());
    auto position = Utils3D::getRayCrossWithPlane(ray, plane);

    if (cursor) {
        cursor->setPosition(position);
        cursor->setScreenPosition(screenPos);
    } else {
        cursor = make_shared<Cursor>(position, screenPos, _camera);
        _selected = cursor;
        composite.reset();
    }
}

void Scene::setSelected(std::shared_ptr<Object> object) {
    if (object->type() == COMPOSITE) {
        composite = std::move(object);
        _selected = composite;
        cursor.reset();
    }else if (find_if(objects.begin(), objects.end(), [&] (const shared_ptr<Object>& ob) { return ob.get() == object.get(); }) != objects.end()) {
        _selected = object;
        composite.reset();
        cursor.reset();
    }
}