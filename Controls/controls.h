#ifndef CONTROLLS_H
#define CONTROLLS_H

#include <QWidget>
#include "Scene/scene.h"
#include "ui_controls.h"

namespace Ui {
    class Controls;
}

class Controls : public QWidget {
Q_OBJECT

public:
    explicit Controls(QWidget *parent = nullptr);

    ~Controls();

    void setScene(std::shared_ptr<Scene> scenePtr);

    void updateSelected();

private slots:
    void on_uDensity_valueChanged(int arg1);

    void on_vDensity_valueChanged(int arg1);

    void on_majorRadius_valueChanged(double arg1);

    void on_minorRadius_valueChanged(double arg1);

    void on_posX_valueChanged(double arg1);

    void on_posY_valueChanged(double arg1);

    void on_posZ_valueChanged(double arg1);

    void on_rotX_valueChanged(double arg1);

    void on_rotY_valueChanged(double arg1);

    void on_rotZ_valueChanged(double arg1);

    void on_scaleX_valueChanged(double arg1);

    void on_scaleY_valueChanged(double arg1);

    void on_scaleZ_valueChanged(double arg1);

    void on_screenPosX_valueChanged(int arg1);

    void on_screenPosY_valueChanged(int arg1);

private:
    struct ObjectHandler {
        QPropertyNotifier position;
        QPropertyNotifier rotation;
        QPropertyNotifier scale;
    };
    QPropertyNotifier sceneHandler;
    Controls::ObjectHandler objectHandler;

    std::shared_ptr<Scene> scene;
    std::shared_ptr<Object> object;
    int dim;

    void setDensity() const;

    void setVectorX(void (Object::*target)(DirectX::XMFLOAT3),
                              DirectX::XMFLOAT3 (Object::*source)() const, float value) const;

    void setVectorY(void (Object::*target)(DirectX::XMFLOAT3),
                              DirectX::XMFLOAT3 (Object::*source)() const, float value) const;

    void setVectorZ(void (Object::*target)(DirectX::XMFLOAT3),
                              DirectX::XMFLOAT3 (Object::*source)() const, float value) const;

    void updatePosition();

    void updateRotation();

    void updateScale();

    template<size_t Dim>
    std::array<int, Dim> parameters() const;

    Ui::Controls *ui;

    // IN PROGRESS:
    // TODO: control screen space and scene space location of 3D cursor (if present)

    // TODO
    // TODO: add torus and 3D point to scene in place of 3D cursor
    // TODO: delete objects from scene
    // TODO: list objects on scene
    // TODO: names of objects on scene (in list and possible to change)
    // TODO: selection of object (from list and clicking on screen)
    // TODO: show center of all selected objects
    // TODO: modify _scale, _position and _rotation of selected objects (based on center of selected objects)
};

template<size_t Dim>
std::array<int, Dim> Controls::parameters() const {
    if constexpr (Dim == 1)
        return {ui->uDensity->value()};
    if constexpr (Dim == 2)
        return {ui->uDensity->value(), ui->vDensity->value()};
    return {};
}

#endif // CONTROLLS_H
