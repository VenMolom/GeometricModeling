//
// Created by Molom on 2022-03-18.
//

#include "utils3D.h"

using namespace Utils3D;
using namespace DirectX;

XMFLOAT3RAY Utils3D::getRayFromScreen(XMINT2 screenPosition, const std::shared_ptr<Camera> &camera) {
    XMFLOAT3 position = camera->position();
    XMFLOAT3 right = camera->right();
    XMFLOAT3 up = camera->up();
    XMFLOAT3 direction = camera->direction();
    auto viewport = camera->viewport();

    float scaleX = screenPosition.x / (viewport.width() / 2.0f)- 1.0f;
    float scaleY = 1.0f - screenPosition.y / (viewport.height() / 2.0f);

    XMFLOAT3RAY result{};
    XMStoreFloat3(&result.position, XMLoadFloat3(&position));
    XMStoreFloat3(&result.direction, XMVector3Normalize(
            XMVectorAdd(
                    XMVectorAdd(XMVectorScale(XMLoadFloat3(&right), scaleX),
                                XMVectorScale(XMLoadFloat3(&up), scaleY)),
            XMVectorScale(XMLoadFloat3(&direction), -camera->z())
    )));
    return result;
}

XMFLOAT4 Utils3D::getPerpendicularPlaneThroughPoint(XMFLOAT3 direction, XMFLOAT3 point) {
    float d;
    XMStoreFloat(&d, XMVector3Dot(
            XMLoadFloat3(&direction), XMLoadFloat3(&point)));
    return {direction.x, direction.y, direction.z, -d};
}

XMFLOAT3 Utils3D::getRayCrossWithPlane(XMFLOAT3RAY ray, XMFLOAT4 plane) {
    XMFLOAT4 p(ray.position.x, ray.position.y, ray.position.z, 1),
            v(ray.direction.x, ray.direction.y, ray.direction.z, 0);
    float pA, vA;
    XMStoreFloat(&pA, XMVector4Dot(XMLoadFloat4(&p), XMLoadFloat4(&plane)));
    XMStoreFloat(&vA, XMVector4Dot(XMLoadFloat4(&v), XMLoadFloat4(&plane)));
    float t = -pA / vA;
    XMFLOAT3 result{};
    XMStoreFloat3(&result, XMVectorAdd(
            XMLoadFloat3(&ray.position),
            XMVectorScale(XMLoadFloat3(&ray.direction), t))
    );
    return result;
}

void
Utils3D::storeFloat3Lerp(DirectX::XMFLOAT3 &target, const DirectX::XMFLOAT3 &v1, const DirectX::XMFLOAT3 &v2, float t) {
    XMStoreFloat3(&target, XMVectorLerp(XMLoadFloat3(&v1), XMLoadFloat3(&v2), t));
}
