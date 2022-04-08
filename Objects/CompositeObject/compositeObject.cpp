//
// Created by Molom on 2022-03-19.
//

#include "compositeObject.h"

#include <utility>
#include <Objects/Cursor/cursor.h>

using namespace std;
using namespace DirectX;

CompositeObject::CompositeObject(list<shared_ptr<Object>> &&objects)
        : Object("Composite", {0, 0, 0}),
          objects() {

    for (auto &object : objects) {
        if (!(object->type() & COMPOSABLE)) continue;

        XMFLOAT4X4 model{};
        XMStoreFloat4x4(&model, object->modelMatrix());
        startingMatrices.push_back(model);
        this->objects.push_back(object);
    }

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

bool CompositeObject::contains(const shared_ptr<Object> &object) const {
    return find_if(objects.begin(), objects.end(),
                   [&object](const shared_ptr<Object> &ob) { return object->equals(ob); }) != objects.end();
}

std::list<std::shared_ptr<Object>> &&CompositeObject::release() {
    return std::move(objects);
}

void
CompositeObject::draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection,
                      DrawType drawType) {
    for (auto &object: objects) {
        object->draw(renderer, view, projection, NO_CURSOR);
    }
    Cursor::drawCursor(renderer, XMLoadFloat4x4(&noScaleMatrix) * view * projection);
}

Type CompositeObject::type() const {
    return COMPOSITE;
}

DirectX::BoundingOrientedBox CompositeObject::boundingBox() const {
    return {{}, {}, {0, 0, 0, 1.f}};
}

void CompositeObject::setPosition(DirectX::XMFLOAT3 position) {
    Object::setPosition(position);
    updateChildren();
}

void CompositeObject::setRotation(DirectX::XMFLOAT3 rotation) {
    Object::setRotation(rotation);
    updateChildren();
}

void CompositeObject::setScale(DirectX::XMFLOAT3 scale) {
    Object::setScale(scale);
    updateChildren();
}

void CompositeObject::updateChildren() {
    auto rotation = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&_rotation.value()));
    auto scaling = XMMatrixScalingFromVector(XMLoadFloat3(&_scale.value()));

    auto translationBack = XMMatrixTranslationFromVector(XMLoadFloat3(&_position.value()));
    auto translationToCenter = XMMatrixTranslationFromVector(XMVectorNegate(XMLoadFloat3(&startingPosition)));
    auto modifyMatrix = translationToCenter * scaling * rotation * translationBack;

    XMVECTOR vpos{}, vrot{}, vscal{};
    XMFLOAT3 pos{} ,scal{}, eulerRot{};
    XMFLOAT4 rot{};
    int i = 0;
    for (auto &object : objects) {
        auto childMatrix = XMLoadFloat4x4(&startingMatrices[i++]) * modifyMatrix;
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
}

DirectX::XMFLOAT3 CompositeObject::rotationMatrixToEuler(XMMATRIX rotationMatrix) {
    XMFLOAT4X4 XMFLOAT4X4_Values{};
    XMStoreFloat4x4(&XMFLOAT4X4_Values, XMMatrixTranspose(rotationMatrix));

    XMFLOAT3 euler{};
    euler.x = (float)asin(-XMFLOAT4X4_Values._23);
    euler.y = (float) atan2(XMFLOAT4X4_Values._13, XMFLOAT4X4_Values._33);
    euler.z = (float) atan2(XMFLOAT4X4_Values._21, XMFLOAT4X4_Values._22);

    return euler;

//    float c2 = sqrt(rotationMatrix.r[0].m128_f32[0] * rotationMatrix.r[0].m128_f32[0]
//            + rotationMatrix.r[0].m128_f32[1] * rotationMatrix.r[0].m128_f32[1]);
//
//    XMFLOAT3 euler{};
//    euler.x = atan2(rotationMatrix.r[1].m128_f32[2], rotationMatrix.r[2].m128_f32[2]);
//    euler.y = atan2(-rotationMatrix.r[0].m128_f32[2], c2);
//
//    float s1 = sin(euler.x);
//    float c1 = cos(euler.x);
//
//    euler.z = atan2(s1 * rotationMatrix.r[2].m128_f32[0] - c1 * rotationMatrix.r[1].m128_f32[0],
//                    c1 * rotationMatrix.r[1].m128_f32[1] - s1 * rotationMatrix.r[2].m128_f32[1]);
//
//    return euler;
}

bool CompositeObject::equals(const shared_ptr<Object> &other) const {
    return contains(other);
}
