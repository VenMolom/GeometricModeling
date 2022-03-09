//
// Created by Molom on 2022-02-27.
//

#include "camera.h"

Camera::Camera() : position(0, 0, 1),
                   center(0, 0, 0),
                   viewPlane(-1, 1, 2, 2),
                   front(0, 0, -1),
                   up(0, 1, 0),
                   worldUp(0, 1, 0),
                   right(1, 0, 0),
                   viewDepth(100),
                   distance(5),
                   zoom(0.01),
                   yaw(M_PI_2),
                   pitch(0) {
    perspective = QMatrix4x4();
    perspective.ortho(viewPlane.left(), viewPlane.right(), viewPlane.top() - viewPlane.height(), viewPlane.top(), 0,
                      viewDepth);

    position *= distance;
    view = QMatrix4x4();
    view.lookAt(position, center, up);
}

QMatrix4x4 Camera::viewMatrix() const {
    return perspective * view;
}

void Camera::resize(QSizeF newSize) {
    auto size = newSize * zoom;
    viewPlane = QRectF(-size.width() / 2, size.height() / 2, size.width(), size.height());

    perspective = QMatrix4x4();
    perspective.ortho(viewPlane.left(), viewPlane.right(), viewPlane.top() - viewPlane.height(), viewPlane.top(), 0,
                      viewDepth);
}

void Camera::changeZoom(float delta, QSizeF viewportSize) {
    // TODO: change distance (near/far)

    if (delta <= 0) {
        zoom *= STEP;
    } else {
        zoom /= STEP;
    }
    resize(viewportSize);
}

void Camera::rotate(QPointF angle) {
    auto rotate = angle * SENSITIVITY;

    yaw += rotate.x();
    pitch = std::clamp<float>(pitch += rotate.y(), -LIMIT, LIMIT);


    position.setX(cos(yaw) * cos(pitch));
    position.setY(sin(pitch));
    position.setZ(sin(yaw) * cos(pitch));
    position.normalize();
    position *= distance;

    right = QVector3D::crossProduct(front, worldUp).normalized();
    up = QVector3D::crossProduct(right, front).normalized();

    view = QMatrix4x4();
    view.lookAt(position, center, up);
}

void Camera::move(QPointF direction) {
    center += SPEED * direction.x() * right;
    center += SPEED * direction.y() * up;
    position += SPEED * direction.y() * up;

    view = QMatrix4x4();
    view.lookAt(position, center, up);
}
