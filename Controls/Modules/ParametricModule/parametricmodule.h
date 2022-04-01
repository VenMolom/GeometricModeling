#ifndef PARAMETRICMODULE_H
#define PARAMETRICMODULE_H

#include <QWidget>

namespace Ui {
class ParametricModule;
}

class ParametricModule : public QWidget
{
    Q_OBJECT

public:
    explicit ParametricModule(QWidget *parent = nullptr);
    ~ParametricModule();

private:
    Ui::ParametricModule *ui;
};

#endif // PARAMETRICMODULE_H
