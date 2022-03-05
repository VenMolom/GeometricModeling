//
// Created by Molom on 2022-02-27.
//

#include "camera.h"

Camera::Camera() : position(0, 0, 5),
                   viewPlane(-1, 1, 2, 2),
                   front(0, 0, -1),
                   up(0, 1, 0),
                   worldUp(0, 1, 0),
                   right(1, 0, 0),
                   viewDepth(100),
                   zoom(0.01),
                   yaw(-M_PI_2),
                   pitch(0) {
    perspective = QMatrix4x4();
    perspective.ortho(viewPlane.left(), viewPlane.right(), viewPlane.top() - viewPlane.height(), viewPlane.top(), 0,
                      viewDepth);

    view = QMatrix4x4();
    view.lookAt(position, position + front, up);
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
    pitch = std::clamp<float>(pitch -= rotate.y(), -LIMIT, LIMIT);


    front.setX(cos(yaw) * cos(pitch));
    front.setY(sin(pitch));
    front.setZ(sin(yaw) * cos(pitch));
    front.normalize();

    right = QVector3D::crossProduct(front, worldUp).normalized();
    up = QVector3D::crossProduct(right, front).normalized();

    view = QMatrix4x4();
    view.lookAt(position, position + front, up);
}

void Camera::move(QPointF direction) {
    position += SPEED * direction.x() * right;
    position -= SPEED * direction.y() * front;

    view = QMatrix4x4();
    view.lookAt(position, position + front, up);
}
