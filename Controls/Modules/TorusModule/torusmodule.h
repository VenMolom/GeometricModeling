#ifndef TORUSMODULE_H
#define TORUSMODULE_H

#include <QWidget>
#include "Objects/Parametric/torus.h"

namespace Ui {
class TorusModule;
}

class TorusModule : public QWidget {
    Q_OBJECT

public:
    explicit TorusModule(std::shared_ptr<Torus> torus, QWidget *parent = nullptr);
    ~TorusModule();

private slots:
    void on_majorRadius_valueChanged(double arg1);

    void on_minorRadius_valueChanged(double arg1);

private:
    std::shared_ptr<Torus> torus;

    Ui::TorusModule *ui;
};

#endif // TORUSMODULE_H
