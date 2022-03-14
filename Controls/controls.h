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

    int majorRadius() const { return ui->majorRadius->value(); }

    int minorRadius() const { return ui->minorRadius->value(); }

    template <size_t Dim>
    std::array<int, Dim> parameters() const;

private slots:
    void on_uDensity_valueChanged(int arg1);

    void on_vDensity_valueChanged(int arg1);

    void on_majorRadius_valueChanged(double arg1);

    void on_minorRadius_valueChanged(double arg1);

private:
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Object> object;
    int dim;

    void setDensity() const;

    Ui::Controls *ui;
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
