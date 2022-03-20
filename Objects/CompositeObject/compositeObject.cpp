//
// Created by Molom on 2022-03-19.
//

#include "compositeObject.h"

#include <utility>
#include <DirectXMath.h>

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
    startingPosition = center;
    setPosition(center);
}

bool CompositeObject::contains(shared_ptr<Object> object) {
    return find_if(objects.begin(), objects.end(),
                   [&](const shared_ptr<Object> &ob) { return ob.get() == object.get(); }) != objects.end();
}

std::list<std::shared_ptr<Object>> &&CompositeObject::release() {
    XMVECTOR vpos, vrot, vscal;
    XMFLOAT3 pos{} ,scal{};
    XMFLOAT4 rot{};
    for (auto &object : objects) {
        XMMatrixDecompose(&vscal, &vrot, &vpos, object->modelMatrix() * XMLoadFloat4x4(&modifyMatrix));
        XMStoreFloat3(&pos, vpos);
        XMStoreFloat3(&scal, vscal);
        XMStoreFloat4(&rot, vrot);
        object->setPosition(pos);
        object->setScale(scal);
        object->setRotation(quaternionToEuler(rot));
    }

    return std::move(objects);
}

void
CompositeObject::draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection,
                      DrawType drawType) const {
    auto modifiedView = XMLoadFloat4x4(&modifyMatrix) * view;
    for (auto &object: objects) {
        object->draw(renderer, modifiedView, projection, NO_CURSOR);
    }
    renderer.drawCursor(XMLoadFloat4x4(&noScaleMatrix) * view * projection);
}

Type CompositeObject::type() const {
    return COMPOSITE;
}

DirectX::BoundingOrientedBox CompositeObject::boundingBox() const {
    return {};
}

void CompositeObject::setPosition(DirectX::XMFLOAT3 position) {
    Object::setPosition(position);
    updateMatrix();
}

void CompositeObject::setRotation(DirectX::XMFLOAT3 rotation) {
    Object::setRotation(rotation);
    updateMatrix();
}

void CompositeObject::setScale(DirectX::XMFLOAT3 scale) {
    Object::setScale(scale);
    updateMatrix();
}

void CompositeObject::updateMatrix() {
    auto translationBack = XMMatrixTranslationFromVector(XMLoadFloat3(&_position.value()));
    auto translationToCenter = XMMatrixTranslationFromVector(XMVectorScale(XMLoadFloat3(&startingPosition), -1));
    XMStoreFloat4x4(&modifyMatrix, translationToCenter * XMLoadFloat4x4(&scaleMatrix)
                                   * XMLoadFloat4x4(&rotationMatrix) * translationBack);
}

DirectX::XMFLOAT3 CompositeObject::quaternionToEuler(DirectX::XMFLOAT4 quaternion) {
    XMFLOAT3 angles{};
    // roll (x-axis rotation)
    double sinr_cosp = 2 * (quaternion.w * quaternion.x + quaternion.y * quaternion.z);
    double cosr_cosp = 1 - 2 * (quaternion.x * quaternion.x + quaternion.y * quaternion.y);
    angles.x = std::atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    double sinp = 2 * (quaternion.w * quaternion.y - quaternion.z * quaternion.x);
    if (std::abs(sinp) >= 1)
        angles.y = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        angles.y = std::asin(sinp);

    // yaw (z-axis rotation)
    double siny_cosp = 2 * (quaternion.w * quaternion.z + quaternion.x * quaternion.y);
    double cosy_cosp = 1 - 2 * (quaternion.y * quaternion.y + quaternion.z * quaternion.z);
    angles.z = std::atan2(siny_cosp, cosy_cosp);

    return angles;
}
