#ifndef MOVEMODULE_H
#define MOVEMODULE_H

#include <QWidget>
#include "Objects/object.h"

namespace Ui {
class MoveModule;
}

class MoveModule : public QWidget {
    Q_OBJECT

public:
    explicit MoveModule(std::shared_ptr<Object> object, QWidget *parent = nullptr);
    ~MoveModule();

private slots:
    void on_posX_valueChanged(double arg1);

    void on_posY_valueChanged(double arg1);

    void on_posZ_valueChanged(double arg1);

private:
    std::shared_ptr<Object> object;
    QPropertyNotifier positionHandler;

    Ui::MoveModule *ui;

    void updatePosition();
};

#endif // MOVEMODULE_H
