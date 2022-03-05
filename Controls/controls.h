#ifndef CONTROLLS_H
#define CONTROLLS_H

#include <QWidget>
#include "Scene/scene.h"

namespace Ui {
    class Controls;
}

class Controls : public QWidget {
    Q_OBJECT
    std::shared_ptr<Scene> scene;
    int segments;

public:
    explicit Controls(QWidget *parent = nullptr);

    ~Controls();

    void setScene(std::shared_ptr<Scene> scenePtr);

private slots:
    void on_radiusX_valueChanged(double arg1);

    void on_radiusY_valueChanged(double arg1);

    void on_radiusZ_valueChanged(double arg1);

    void on_renderingSegments_valueChanged(int arg1);

    void on_specular_valueChanged(int arg1);

signals:
    void updated(int segments);

private:
    Ui::Controls *ui;
};

#endif // CONTROLLS_H
