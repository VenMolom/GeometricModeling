#ifndef PARAMETRICMODULE_H
#define PARAMETRICMODULE_H

#include <QWidget>
#include "Objects/Parametric/parametricObject.h"
#include "ui_parametricmodule.h"

namespace Ui {
class ParametricModule;
}

class ParametricModule : public QWidget {
    Q_OBJECT

public:
    explicit ParametricModule(std::shared_ptr<ParametricObject<2>> object, QWidget *parent = nullptr);
    ~ParametricModule();

private slots:
    void on_uDensity_valueChanged(int arg1);

    void on_vDensity_valueChanged(int arg1);

private:
    std::shared_ptr<ParametricObject<2>> object;

    Ui::ParametricModule *ui;

    void setDensity() const;

    template<size_t Dim>
    std::array<int, Dim> parameters() const;
};


template<size_t Dim>
std::array<int, Dim> ParametricModule::parameters() const {
    if constexpr (Dim == 1)
        return {ui->uDensity->value()};
    if constexpr (Dim == 2)
        return {ui->uDensity->value(), ui->vDensity->value()};
    return {};
}

#endif // PARAMETRICMODULE_H
