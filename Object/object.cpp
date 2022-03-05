//
// Created by Molom on 2022-02-27.
//

#include "object.h"

Object::Object() : position(0, 0, 0, 1), specular(4), color(QColorConstants::Red) {
    model = QMatrix4x4();
    model.translate(position.toVector3DAffine());
}

QColor
Object::calculateColor(const QVector3D &worldPosition, const QVector3D &lightPosition, const QVector3D &normal) const {
    auto viewDir = (lightPosition - worldPosition).normalized();
    auto lightness = powf(std::max(QVector3D::dotProduct(normal.normalized(), viewDir), 0.0f), specular);
    return QColor(color.red() * lightness, color.green() * lightness, color.blue() * lightness);
}

QVector2D Object::convertFromScreen(float x, float y, const QSize &size) {
    return {2 * x / (size.width() - 1.0f) - 1, -(2 * y / (size.height() - 1.0f) - 1)};
}