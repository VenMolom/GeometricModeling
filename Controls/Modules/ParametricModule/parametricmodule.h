#ifndef PARAMETRICMODULE_H
#define PARAMETRICMODULE_H

#include <QWidget>
#include "Objects/Parametric/parametricObject.h"

namespace Ui {
    class ParametricModule;
}

class ParametricModule : public QWidget {
Q_OBJECT

public:
    ParametricModule(std::shared_ptr<ParametricObject<2>> object, QWidget *parent = nullptr);

    ~ParametricModule();

private slots:

    void on_uDensity_valueChanged(int arg1);

    void on_vDensity_valueChanged(int arg1);

private:
    std::shared_ptr<ParametricObject<2>> object;

    Ui::ParametricModule *ui;
};


#endif // PARAMETRICMODULE_H
