//
// Created by Molom on 2022-02-27.
//

#include "scene.h"

using namespace DirectX;
using namespace std;

void Scene::draw(Renderer &renderer) const {
    if (composite) {
        composite->draw(renderer, _camera.viewMatrix(), _camera.projectionMatrix(), SELECTED);
    }
    if (cursor) {
        cursor->draw(renderer, _camera.viewMatrix(), _camera.projectionMatrix(), DEFAULT);
    }
    for (auto &object: _objects) {
        auto a = _selected.value().lock().get();
        object->draw(renderer, _camera.viewMatrix(), _camera.projectionMatrix(), a == object.get() ? SELECTED : DEFAULT);
    }

}

void Scene::addObject(shared_ptr<Object> &&object) {
    if (cursor) {
        object->setPosition(cursor->position());
        cursor.reset();
    }

    _selected = object;
    _objects.push_back(std::move(object));
}

void Scene::addComposite(list<shared_ptr<Object>> &&objects) {
    _objects.remove_if([&](const shared_ptr<Object> &ob) {
        return find_if(objects.begin(), objects.end(),
                [&](const shared_ptr<Object> &obb) { return ob.get() == obb.get(); }) != objects.end();
    });

    auto comp = make_shared<CompositeObject>(std::move(objects));
    setSelected(comp);
}

void Scene::removeSelected() {
    if (auto selected = _selected.value().lock()) {
        _objects.remove_if([&](const shared_ptr<Object> &ob) { return ob.get() == selected.get(); });
        composite.reset();
        _selected.setValue({});
    }
}

void Scene::selectOrAddCursor(QPoint screenPosition, bool multiple) {
    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto screenSize = XMFLOAT2(_camera.viewport().width(), _camera.viewport().height());
    auto ray = Utils3D::getRayFromScreen(screenPos, screenSize, _camera.nearZ(), _camera.farZ(),
                                         _camera.projectionMatrix(), _camera.viewMatrix());

    if (auto object = findIntersectingObject(ray)) {
        shared_ptr<Object> sel;
        if (multiple && (sel = _selected.value().lock()) && sel.get() != object.get()) {
            if (composite) {
                auto comp = dynamic_cast<CompositeObject *>(composite.get());

                if (comp->contains(object)) return;

                auto obs = comp->release();
                obs.push_back(object);
                addComposite(std::move(obs));
            } else {
                list<shared_ptr<Object>> obs = {sel, object};
                addComposite(std::move(obs));
            }
        } else {
            setSelected(object);
        }
    } else {
        addCursor(ray, screenPos);
    }
}

shared_ptr<Object> Scene::findIntersectingObject(Utils3D::XMFLOAT3RAY ray) {
    shared_ptr<Object> closest{nullptr};
    float closestDistance = INFINITY;

    for (auto &object: _objects) {
        float distance{};
        if (object->boundingBox().Intersects(XMLoadFloat3(&ray.position), XMLoadFloat3(&ray.direction), distance) &&
            distance < closestDistance) {
            closest = object;
            closestDistance = distance;
        }
    }

    return closest;
}

void Scene::addCursor(Utils3D::XMFLOAT3RAY ray, XMINT2 screenPos) {
    auto plane = Utils3D::getPerpendicularPlaneThroughPoint(_camera.direction(), _camera.center());
    auto position = Utils3D::getRayCrossWithPlane(ray, plane);

    removeComposite();
    if (cursor) {
        cursor->setPosition(position);
        cursor->setScreenPosition(screenPos);
    } else {
        cursor = make_shared<Cursor>(position, screenPos, _camera);
        _selected = cursor;
    }
}

void Scene::setSelected(std::shared_ptr<Object> object) {
    if(!object) {
        removeComposite();
        _selected.setValue({});
        return;
    }

    if (object->type() == COMPOSITE) {
        composite = std::move(object);
        _selected = composite;
        cursor.reset();
    } else if (find_if(_objects.begin(), _objects.end(),
                       [&](const shared_ptr<Object> &ob) { return ob.get() == object.get(); }) != _objects.end()) {
        _selected = object;
        removeComposite();
        cursor.reset();
    }
}

void Scene::removeComposite() {
    if (!composite) return;

    auto comp = dynamic_cast<CompositeObject *>(composite.get());
    _objects.splice(_objects.end(), comp->release());
    composite.reset();
}
