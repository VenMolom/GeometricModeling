#ifndef TRANSFORMMODULE_H
#define TRANSFORMMODULE_H

#include <QWidget>
#include "Objects/Object/object.h"

namespace Ui {
class TransformModule;
}

class TransformModule : public QWidget {
    Q_OBJECT

public:
    explicit TransformModule(std::shared_ptr<Object> object, QWidget *parent = nullptr);
    ~TransformModule();

private slots:
    void on_rotX_valueChanged(double arg1);

    void on_rotY_valueChanged(double arg1);

    void on_rotZ_valueChanged(double arg1);

    void on_scaleX_valueChanged(double arg1);

    void on_scaleY_valueChanged(double arg1);

    void on_scaleZ_valueChanged(double arg1);

private:
    std::shared_ptr<Object> object;
    QPropertyNotifier rotationHandler;
    QPropertyNotifier scaleHandler;

    Ui::TransformModule *ui;

    void updateRotation();

    void updateScale();
};

#endif // TRANSFORMMODULE_H
