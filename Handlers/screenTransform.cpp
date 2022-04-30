//
// Created by Molom on 28/04/2022.
//

#include "screenTransform.h"

#include <utility>

using namespace std;
using namespace DirectX;
using namespace Utils3D;

ScreenTransform::ScreenTransform(shared_ptr<Object> object, QPoint screenPosition,
                                 shared_ptr<Camera> camera, Transform mode, Axis axis)
        : object(std::move(object)),
          camera(std::move(camera)),
          mode(mode),
          axis(axis),
          startingScreenPosition(screenPosition) {
    movable = this->object->type() & MOVABLE;
    transformable = this->object->type() & TRANSFORMABLE;

    startingScenePosition = getPositionFromScreen(screenPosition);

    if (transformable) {
        startRotation = this->object->rotation();
        startScale = this->object->scale();
    }
}

void ScreenTransform::transform(QPoint screenPosition) {
    switch (mode) {
        case MOVE:
            if (!movable) return;
            move(screenPosition);
            break;
        case ROTATE:
            if (!transformable) return;
            rotate(screenPosition);
            break;
        case SCALE:
            if (!transformable) return;
            scale(screenPosition);
            break;
    }

}

void ScreenTransform::move(QPoint screenPosition) {
    auto position = getPositionFromScreen(screenPosition);
    object->setPosition(position);
}

void ScreenTransform::rotate(QPoint screenPosition) {
// TODO: ? implement
}

void ScreenTransform::scale(QPoint screenPosition) {
    auto position = getPositionFromScreen(screenPosition);
    auto rotation = object->rotation();
    auto diff = XMVectorScale(
            XMVector3InverseRotate(
                    XMVectorSubtract(XMLoadFloat3(&position), XMLoadFloat3(&startingScenePosition)),
                    XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation))),
            SCALE_MODIFIER);
    XMFLOAT3 newScale{};
    XMStoreFloat3(&newScale, XMVectorAdd(XMLoadFloat3(&startScale), diff));


    object->setScale(newScale);
}

XMFLOAT3 ScreenTransform::getPositionFromScreen(QPoint screenPosition) {
    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto ray = getRayFromScreen(screenPos, camera);
    auto plane = getPerpendicularPlaneThroughPoint(camera->direction(), object->position());
    return getRayCrossWithPlane(ray, plane);
}
