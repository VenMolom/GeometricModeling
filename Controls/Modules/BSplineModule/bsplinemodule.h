#ifndef BSPLINEMODULE_H
#define BSPLINEMODULE_H

#include <QWidget>
#include "Objects/Curve/brezierC2.h"

namespace Ui {
class BSplineModule;
}

class BSplineModule : public QWidget {
    Q_OBJECT

public:
    explicit BSplineModule(std::shared_ptr<BrezierC2> curve, QWidget *parent = nullptr);
    ~BSplineModule();

private slots:
    void on_bernsteinBaseCheckBox_stateChanged(int arg1);

    void on_bothPolygonalsCheckBox_stateChanged(int arg1);

private:
    std::shared_ptr<BrezierC2> curve;

    Ui::BSplineModule *ui;
};

#endif // BSPLINEMODULE_H
