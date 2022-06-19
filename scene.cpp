//
// Created by Molom on 2022-02-27.
//

#include "scene.h"

using namespace DirectX;
using namespace Utils3D;
using namespace std;

Scene::Scene()
        : grid(500), virtualPointsHolders(), _objects(), _selected(), factory() {
    _camera = make_shared<Camera>();
    _camera->rotate({0, 200.0f});
}

void Scene::draw(Renderer &renderer) {
    grid.draw(renderer, DEFAULT);
    if (composite) {
        composite->draw(renderer, SELECTED);
    }
    if (cursor) {
        cursor->draw(renderer, DEFAULT);
    }
    for (auto &object: _objects) {
        auto selected = _selected.value().lock().get();
        object->draw(renderer, selected == object.get() ? SELECTED : DEFAULT);
    }

    if (creator && !_selected.value().expired()) {
        _selected.value().lock()->draw(renderer, SELECTED);
    }
}

#pragma region Add

void Scene::addObject(shared_ptr<Object> &&object, bool overrideCursor) {
    if (!overrideCursor && cursor) {
        object->setPosition(cursor->position());
        cursor.reset();
    }
    boolean select = true;

    // add point to selected curve
    shared_ptr<Object> sel;
    if ((sel = _selected.value().lock()) && sel->type() & CURVE && object->type() & POINT3D) {
        auto *c = dynamic_cast<Curve *>(sel.get());
        shared_ptr<Point> p = static_pointer_cast<Point>(object);
        c->addPoint(p);
        select = false;
    }

    if (select) {
        setSelected(object);
    }

    if (object->type() & VIRTUALPOINTSHOLDER) {
        virtualPointsHolders.push_back(dynamic_pointer_cast<VirtualPointsHolder>(object));
    }

    _objects.push_back(std::move(object));
    emit objectAdded(_objects.back(), select);
}

void Scene::addPoint(QPoint screenPosition) {
    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto position = getPositionOnPlane(screenPos, _camera->direction(), _camera->center());
    addObject(factory.createPoint(position), true);
}

void Scene::addCursor(XMFLOAT3RAY ray, XMINT2 screenPos) {
    auto plane = getPerpendicularPlaneThroughPoint(_camera->direction(), _camera->center());
    auto position = getRayCrossWithPlane(ray, plane);

    removeComposite();
    if (cursor) {
        cursor->setPosition(position);
        cursor->setScreenPosition(screenPos);
        setSelected(cursor);
    } else {
        cursor = make_shared<Cursor>(position, screenPos, _camera);
        setSelected(cursor);
    }
}

shared_ptr<CompositeObject> Scene::addComposite(list<shared_ptr<Object>> &&objects) {
    _objects.remove_if([&objects](const shared_ptr<Object> &ob) {
        return find_if(objects.begin(), objects.end(),
                       [&ob](const shared_ptr<Object> &obb) { return ob->equals(obb); }) != objects.end();
    });

    auto comp = make_shared<CompositeObject>(std::move(objects));

    if (comp->empty()) {
        setSelected({});
        return {};
    }

    setSelected(comp);
    return comp;
}

void Scene::addCreator(shared_ptr<Object> &&creator) {
    if (cursor) {
        creator->setPosition(cursor->position());
        cursor.reset();
    }

    setSelected(creator);
    this->creator = creator;
}

void Scene::createFromCreator() {
    if (creator) {
        addObject(dynamic_pointer_cast<Creator>(creator)->create(factory.id()), true);
    }
}

#pragma endregion

#pragma region Actions

void Scene::collapseSelected() {
    if (!composite) return;

    auto comp = static_pointer_cast<CompositeObject>(composite);

    if (!comp->collapsable()) return;

    auto newPoint = comp->collapse();

    virtualPointsHolders.remove_if([](weak_ptr<VirtualPointsHolder> &holder) { return holder.expired(); });
    for (auto &holder: virtualPointsHolders) {
        auto points = holder.lock()->virtualPoints();
        for (auto &point: points) {
            if (comp->contains(point)) {
                holder.lock()->replacePoint(point, newPoint);
            }
        }
    }

    composite.reset();
    setSelected(newPoint);
}

void Scene::fillIn() {
    if (!composite) return;

    auto comp = static_pointer_cast<CompositeObject>(composite);

    if (!comp->fillable()) return;

    addObject(comp->fillIn(factory.id()));
}

void Scene::intersect(IntersectHandler &handler) {
    array<shared_ptr<ParametricObject<2>>, 2> surfaces{};

    if (composite) {
        auto comp = dynamic_cast<CompositeObject *>(composite.get());
        if (!comp->intersectable()) return;

        auto released = comp->release();

        int index = 0;
        for (auto& surf : released) {
            surfaces[index++] = static_pointer_cast<ParametricObject<2>>(surf);
        }

        _objects.splice(_objects.end(), std::move(released));
        composite.reset();
    } else {
        auto selected = _selected.value().lock();
        if (!selected || !(selected->type() & PARAMETRIC)) return;

        auto surf = static_pointer_cast<ParametricObject<2>>(selected);
        surfaces[0] = surfaces[1] = surf;
    }

    handler.setSurfaces(surfaces);

    shared_ptr<Object> intersection;
    if (cursor) {
        intersection = handler.calculateIntersection(cursor->position());
    } else {
        intersection = handler.calculateIntersection();
    }

    if (intersection) addObject(std::move(intersection), true);
}

#pragma endregion

#pragma region Selected

void Scene::centerSelected() {
    if (auto selected = _selected.value().lock()) {
        _camera->moveTo(selected->position());
    }
}

void Scene::selectFromScreen(QPointF start, QPointF end) {
    QPointF minPoint{min(start.x(), end.x()), min(start.y(), end.y())};
    QPointF maxPoint{max(start.x(), end.x()), max(start.y(), end.y())};

    list<shared_ptr<Object>> selected{};

    for(auto &object : _objects) {
        if (!(object->type() & SCREENSELECTABLE)) continue;

        auto screenPos = project(object->position());
        if (screenPos.x >= minPoint.x() && screenPos.x <= maxPoint.x()
            && screenPos.y >= minPoint.y() && screenPos.y <= maxPoint.y()) {
            selected.push_back(object);
        }
    }

    virtualPointsHolders.remove_if([](weak_ptr<VirtualPointsHolder> &holder) { return holder.expired(); });
    for (auto &holder: virtualPointsHolders) {
        auto points = holder.lock()->virtualPoints();
        for (auto &point: points) {
            auto screenPos = project(point->position());
            if (screenPos.x >= minPoint.x() && screenPos.x <= maxPoint.x()
                && screenPos.y >= minPoint.y() && screenPos.y <= maxPoint.y()) {
                selected.push_back(point);
            }
        }
    }

    if (selected.empty()) {
        setSelected({});
        return;
    }

    if (selected.size() == 1) {
        setSelected(selected.front());
        return;
    }

    addComposite(std::move(selected));
}

void Scene::selectOrAddCursor(QPoint screenPosition, bool multiple) {
    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto ray = getRayFromScreenPosition(screenPos);

    if (auto object = findIntersectingObject(ray)) {
        shared_ptr<Object> sel;

        if (multiple && (sel = _selected.value().lock()) && object->type() & COMPOSABLE) {
            if (sel->type() & CURVE && object->type() & POINT3D) {
                auto *c = dynamic_cast<Curve *>(sel.get());
                shared_ptr<Point> p = static_pointer_cast<Point>(object);
                c->addPoint(p);
            } else if (sel->type() & COMPOSABLE && sel.get() != object.get()) {
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
            setSelected(object);
        }
    } else {
        addCursor(ray, screenPos);
    }
}

void Scene::setSelected(std::shared_ptr<Object> object) {
    if (!object && cursor) {
        return;
    }

    if (creator) {
        creator.reset();
    }

    if (!object) {
        removeComposite();
        _selected.setValue({});
        return;
    }

    if (object->type() & COMPOSITE) {
        composite = std::move(object);
        _selected = composite;
    } else if ((composite && composite->equals(object))
               || object->type() & VIRTUAL
               || find_if(_objects.begin(), _objects.end(),
                          [&object](const shared_ptr<Object> &ob) { return object->equals(ob); }) != _objects.end()) {
        _selected = object;
        removeComposite();
    }
}

#pragma endregion

#pragma region Helpers

Utils3D::XMFLOAT3RAY Scene::getRayFromScreenPosition(XMINT2 screenPosition) const {
    return getRayFromScreen(screenPosition, _camera);
}

shared_ptr<Object> Scene::findIntersectingObject(XMFLOAT3RAY ray) {
    shared_ptr<Object> closest{nullptr};
    float closestDistance = INFINITY;

    for (auto &object: _objects) {
        float distance{};
        if (object->intersects(ray.position, ray.direction, _camera->viewMatrix(),
                               _camera->nearZ(), _camera->farZ(), distance)
            && distance >= 0 && distance < closestDistance) {
            closest = object;
            closestDistance = distance;
        }
    }

    virtualPointsHolders.remove_if([](weak_ptr<VirtualPointsHolder> &holder) { return holder.expired(); });
    for (auto &holder: virtualPointsHolders) {
        auto points = holder.lock()->virtualPoints();
        for (auto &point: points) {
            float distance{};
            if (point->intersects(ray.position, ray.direction, _camera->cameraMatrix(),
                                  _camera->nearZ(), _camera->farZ(), distance) &&
                distance < closestDistance) {
                closest = point;
                closestDistance = distance;
            }
        }
    }

    return closest;
}

XMFLOAT3 Scene::getPositionOnPlane(XMINT2 screenPosition, XMFLOAT3 normal, XMFLOAT3 point) const {
    auto ray = getRayFromScreenPosition(screenPosition);
    auto plane = getPerpendicularPlaneThroughPoint(normal, point);
    return getRayCrossWithPlane(ray, plane);
}

XMFLOAT2 Scene::project(XMFLOAT3 position) {
    XMFLOAT2 screenPos;
    XMStoreFloat2(&screenPos, XMVector3Project(XMLoadFloat3(&position), 0, 0,
                                               _camera->viewport().width(), _camera->viewport().height(), 0.0f, 1.0f,
                                               _camera->projectionMatrix(), _camera->viewMatrix(), XMMatrixIdentity()));
    return screenPos;
}

#pragma endregion

#pragma region Remove

void Scene::removeComposite() {
    if (!composite) return;

    auto comp = dynamic_cast<CompositeObject *>(composite.get());
    auto released = comp->release();

    released.remove_if([](const shared_ptr<Object> &ob) {
        return ob->type() & VIRTUAL;
    });

    _objects.splice(_objects.end(), std::move(released));
    composite.reset();
}

void Scene::removeSelected() {
    if (auto selected = _selected.value().lock()) {
        _objects.remove_if([&selected](const shared_ptr<Object> &ob) { return selected->equals(ob); });
        composite.reset();
        setSelected(nullptr);
    }
}

#pragma endregion

#pragma region Serialize

void Scene::load(MG1::Scene &scene) {
    _selected.setValue({});
    _objects.clear();
    cursor.reset();
    composite.reset();
    creator.reset();
    virtualPointsHolders.clear();

    factory.nextId = 0;

    for(auto& patch : scene.surfacesC0) {
        factory.nextId = max(factory.nextId, patch.GetId() + 1);
        addObject(make_shared<BicubicC0>(patch, scene.points, bindableSelected()), true);
    }
    for(auto& patch : scene.surfacesC2) {
        factory.nextId = max(factory.nextId, patch.GetId() + 1);
        addObject(make_shared<BicubicC2>(patch, scene.points, bindableSelected()), true);
    }
    for(auto& point : scene.points) {
        factory.nextId = max(factory.nextId, point.GetId() + 1);
        addObject(make_shared<Point>(point), true);
    }
    for(auto& torus : scene.tori) {
        factory.nextId = max(factory.nextId, torus.GetId() + 1);
        addObject(make_shared<Torus>(torus), true);
    }
    for(auto& bezier : scene.bezierC0) {
        factory.nextId = max(factory.nextId, bezier.GetId() + 1);
        addObject(make_shared<BrezierC0>(bezier, _objects), true);
    }
    for(auto& bezier : scene.bezierC2) {
        factory.nextId = max(factory.nextId, bezier.GetId() + 1);
        addObject(make_shared<BrezierC2>(bezier, _objects, bindableSelected()), true);
    }
    for(auto& interpolated : scene.interpolatedC2) {
        factory.nextId = max(factory.nextId, interpolated.GetId() + 1);
        addObject(make_shared<InterpolationCurveC2>(interpolated, _objects), true);
    }
}

void Scene::serialize(MG1::Scene &scene) {
    scene.Clear();
    for(auto & object : _objects) {
        if (object->type() == POINT3D) {
            auto p = static_pointer_cast<Point>(object);
            scene.points.push_back(p->serialize());
        }
        if (object->type() == TORUS) {
            auto t = static_pointer_cast<Torus>(object);
            scene.tori.push_back(t->serialize());
        }
        if (object->type() == BREZIERC0) {
            auto b = static_pointer_cast<BrezierC0>(object);
            scene.bezierC0.push_back(b->serialize());
        }
        if (object->type() == BREZIERC2) {
            auto b = static_pointer_cast<BrezierC2>(object);
            scene.bezierC2.push_back(b->serialize());
        }
        if (object->type() == INTERPOLATIONC2) {
            auto i = static_pointer_cast<InterpolationCurveC2>(object);
            scene.interpolatedC2.push_back(i->serialize());
        }
    }

    for(auto & object : _objects) {
        if (object->type() == PATCHC0) {
            auto b = static_pointer_cast<BicubicC0>(object);
            scene.surfacesC0.push_back(b->serialize(scene.points));
        }
        if (object->type() == PATCHC2) {
            auto b = static_pointer_cast<BicubicC2>(object);
            scene.surfacesC2.push_back(b->serialize(scene.points));
        }
    }
}

#pragma endregion