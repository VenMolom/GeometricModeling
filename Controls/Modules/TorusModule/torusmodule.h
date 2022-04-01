#ifndef TORUSMODULE_H
#define TORUSMODULE_H

#include <QWidget>

namespace Ui {
class TorusModule;
}

class TorusModule : public QWidget
{
    Q_OBJECT

public:
    explicit TorusModule(QWidget *parent = nullptr);
    ~TorusModule();

private:
    Ui::TorusModule *ui;
};

#endif // TORUSMODULE_H
