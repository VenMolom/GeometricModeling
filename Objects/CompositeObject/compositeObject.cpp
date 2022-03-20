//
// Created by Molom on 2022-03-19.
//

#include "compositeObject.h"

#include <utility>

using namespace std;
using namespace DirectX;

CompositeObject::CompositeObject(list<shared_ptr<Object>> &&objects)
        : Object("Composite", {0, 0, 0}),
          objects(objects) {
    calculateCenter();
}

CompositeObject::~CompositeObject() {
    release();
}

void CompositeObject::calculateCenter() {
    XMFLOAT3 center{};
    XMVECTOR c = XMLoadFloat3(&center);
    for (auto &object: objects) {
        auto oc = object->position();
        c = XMVectorAdd(c, XMLoadFloat3(&oc));
    }
    XMStoreFloat3(&center, XMVectorScale(c, 1.0f / static_cast<float>(objects.size())));
    Object::setPosition(center);
}

bool CompositeObject::contains(shared_ptr<Object> object) {
    return find_if(objects.begin(), objects.end(),
                   [&](const shared_ptr<Object> &ob) { return ob.get() == object.get(); }) != objects.end();
}

std::list<std::shared_ptr<Object>> &&CompositeObject::release() {
    // TODO: apply changes to objects
    return std::move(objects);
}

void CompositeObject::draw(Renderer &renderer, const Camera &camera, DrawType drawType) const {
    for (auto &object: objects) {
        object->draw(renderer, camera, NO_CURSOR);
    }
    // TODO: don't multiply by scale matrix
    renderer.drawCursor(modelMatrix() * camera.cameraMatrix());
}

Type CompositeObject::type() const {
    return COMPOSITE;
}

DirectX::BoundingOrientedBox CompositeObject::boundingBox() const {
    return {};
}
