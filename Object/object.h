//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_OBJECT_H
#define MG1_OBJECT_H


#include <QVector4D>
#include <QMatrix4x4>
#include <QImage>

class Object {
protected:
    QVector4D position;

    QColor color;

    int specular;

    QMatrix4x4 model;

public:
    Object();

    Object(Object &object) = default;

    Object(Object &&object) = default;

    virtual void draw(QImage &target, const QMatrix4x4 &camera, const QVector3D &lightPosition) const = 0;

    virtual QVector4D getNormal(QVector4D &worldPosition) const = 0;

    static QVector2D convertFromScreen(float x, float y, const QSize &size);

    [[nodiscard]] QMatrix4x4 modelMatrix() const { return model; }

    [[nodiscard]] QColor
    calculateColor(const QVector3D &worldPosition, const QVector3D &lightPosition, const QVector3D &normal) const;

    void setSpecular(int specular) { this->specular = specular; }
};


#endif //MG1_OBJECT_H
