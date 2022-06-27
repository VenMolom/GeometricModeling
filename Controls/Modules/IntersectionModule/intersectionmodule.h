#ifndef INTERSECTIONMODULE_H
#define INTERSECTIONMODULE_H

#include <QWidget>
#include "Objects/Intersection/intersection.h"
#include "scene.h"

namespace Ui {
    class IntersectionModule;
}

class IntersectionModule : public QWidget {
Q_OBJECT

public:
    explicit IntersectionModule(std::shared_ptr<Intersection> intersection, std::shared_ptr<Scene> scene,
                                QWidget *parent = nullptr);

    ~IntersectionModule();

private slots:

    void on_parameterSpaceButton_clicked();

    void on_convertToCurveButton_clicked();

private:
    std::shared_ptr<Intersection> intersection;
    std::shared_ptr<Scene> scene;
    std::unique_ptr<QDialog> dialog;

    Ui::IntersectionModule *ui;
};

#endif // INTERSECTIONMODULE_H
