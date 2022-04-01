#ifndef BREZIERCURVEMODULE_H
#define BREZIERCURVEMODULE_H

#include <QWidget>

namespace Ui {
class BrezierCurveModule;
}

class BrezierCurveModule : public QWidget
{
    Q_OBJECT

public:
    explicit BrezierCurveModule(QWidget *parent = nullptr);
    ~BrezierCurveModule();

private:
    Ui::BrezierCurveModule *ui;
};

#endif // BREZIERCURVEMODULE_H
