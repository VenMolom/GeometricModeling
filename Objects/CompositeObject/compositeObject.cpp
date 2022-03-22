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
    XMVECTOR vpos{}, vrot{}, vscal{};
    XMFLOAT3 pos{} ,scal{}, eulerRot{};
    XMFLOAT4 rot{};
    for (auto &object : objects) {
        auto childMatrix = object->modelMatrix() * childTransformMatrix(object);
        auto res = XMMatrixDecompose(&vscal, &vrot, &vpos, childMatrix);
        XMStoreFloat3(&pos, vpos);
        XMStoreFloat3(&scal, vscal);
        XMStoreFloat4(&rot, vrot);
        if (!res) {
            childMatrix.r[0] = XMVectorScale(childMatrix.r[0], 1/scal.x);
            childMatrix.r[1] = XMVectorScale(childMatrix.r[1], 1/scal.y);
            childMatrix.r[2] = XMVectorScale(childMatrix.r[2], 1/scal.z);
            eulerRot = rotationMatrixToEuler(childMatrix);
        } else {
            eulerRot = rotationMatrixToEuler(XMMatrixRotationQuaternion(vrot));
        }
        object->setPosition(pos);
        object->setScale(scal);
        object->setRotation(eulerRot);
    }

    return std::move(objects);
}

void
CompositeObject::draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection,
                      DrawType drawType) const {
    //auto modifiedView = XMLoadFloat4x4(&modifyMatrix) * view;
    for (auto &object: objects) {
        auto modifiedView = childTransformMatrix(object) * view;
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

}

DirectX::XMMATRIX CompositeObject::childTransformMatrix(const std::shared_ptr<Object>& child) const {
    auto translationBack = XMMatrixTranslationFromVector(XMLoadFloat3(&_position.value()));
    auto translationToCenter = XMMatrixTranslationFromVector(XMVectorNegate(XMLoadFloat3(&startingPosition)));
    auto childRot = child->rotation();
    auto scaling = XMLoadFloat4x4(&scaleMatrix) * XMMatrixInverse(nullptr, XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&childRot)));
    return translationToCenter * scaling * XMLoadFloat4x4(&rotationMatrix) * translationBack;

}

DirectX::XMFLOAT3 CompositeObject::rotationMatrixToEuler(XMMATRIX rotationMatrix) const {
//    XMFLOAT4X4 XMFLOAT4X4_Values;
//    XMStoreFloat4x4(&XMFLOAT4X4_Values, XMMatrixTranspose(rotationMatrix));
//
//    XMFLOAT3 euler{};
//    euler.x = (float)asin(-XMFLOAT4X4_Values._23);
//    euler.y = (float) atan2(XMFLOAT4X4_Values._13, XMFLOAT4X4_Values._33);
//    euler.z = (float) atan2(XMFLOAT4X4_Values._21, XMFLOAT4X4_Values._22);
//
//    return euler;

//    XMFLOAT3 angles{};
//    // roll (x-axis rotation)
//    double sinr_cosp = 2 * (quaternion.w * quaternion.x + quaternion.y * quaternion.z);
//    double cosr_cosp = 1 - 2 * (quaternion.x * quaternion.x + quaternion.y * quaternion.y);
//    angles.x = std::atan2(sinr_cosp, cosr_cosp);
//
//    // pitch (y-axis rotation)
//    double sinp = 2 * (quaternion.w * quaternion.y - quaternion.z * quaternion.x);
//    if (std::abs(sinp) >= 1)
//        angles.y = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
//    else
//        angles.y = std::asin(sinp);
//
//    // yaw (z-axis rotation)
//    double siny_cosp = 2 * (quaternion.w * quaternion.z + quaternion.x * quaternion.y);
//    double cosy_cosp = 1 - 2 * (quaternion.y * quaternion.y + quaternion.z * quaternion.z);
//    angles.z = std::atan2(siny_cosp, cosy_cosp);
//
//    return angles;

    float c2 = sqrt(rotationMatrix.r[0].m128_f32[0] * rotationMatrix.r[0].m128_f32[0]
            + rotationMatrix.r[0].m128_f32[1] * rotationMatrix.r[0].m128_f32[1]);

    XMFLOAT3 euler{};
    euler.x = atan2(rotationMatrix.r[1].m128_f32[2], rotationMatrix.r[2].m128_f32[2]);
    euler.y = atan2(-rotationMatrix.r[0].m128_f32[2], c2);

    float s1 = sin(euler.x);
    float c1 = cos(euler.x);

    euler.z = atan2(s1 * rotationMatrix.r[2].m128_f32[0] - c1 * rotationMatrix.r[1].m128_f32[0],
                    c1 * rotationMatrix.r[1].m128_f32[1] - s1 * rotationMatrix.r[2].m128_f32[1]);

    return euler;
}
