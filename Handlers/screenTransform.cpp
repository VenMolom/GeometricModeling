//
// Created by Molom on 28/04/2022.
//

#include "screenTransform.h"

#include <utility>

using namespace std;
using namespace DirectX;
using namespace Utils3D;

ScreenTransform::ScreenTransform(shared_ptr<Object> object, shared_ptr<Camera> camera, Transform mode, Axis axis)
        : object(std::move(object)),
          camera(std::move(camera)),
          mode(mode),
          axis(axis) {
    movable = this->object->type() & MOVABLE;
    transformable = this->object->type() & TRANSFORMABLE;

    if (transformable) {
        startRotation = this->object->rotation();
        startScale = this->object->scale();
    }
}

void ScreenTransform::transform(QPoint screenPosition, QPointF delta) {
    switch (mode) {
        case MOVE:
            move(screenPosition);
            break;
        case ROTATE:
            rotate(delta);
            break;
        case SCALE:
            scale(delta);
            break;
    }

}

void ScreenTransform::move(QPoint screenPosition) {
    if (!movable) return;

    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto ray = getRayFromScreen(screenPos, camera);
    auto plane = getPerpendicularPlaneThroughPoint(camera->direction(), object->position());
    auto position = getRayCrossWithPlane(ray, plane);

    object->setPosition(position);
}

void ScreenTransform::rotate(QPointF delta) {

}

void ScreenTransform::scale(QPointF delta) {

}