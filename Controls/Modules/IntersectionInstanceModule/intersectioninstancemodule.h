#ifndef INTERSECTIONINSTANCEMODULE_H
#define INTERSECTIONINSTANCEMODULE_H

#include <QWidget>
#include "Objects/Intersection/intersectionInstance.h"

namespace Ui {
    class IntersectionInstanceModule;
}

class IntersectionInstanceModule : public QWidget {
Q_OBJECT

public:
    IntersectionInstanceModule(std::shared_ptr<IntersectionInstance> instance, QWidget *parent = nullptr);

    ~IntersectionInstanceModule();

signals:
    void removeIntersectionInstance();

private slots:

    void on_activeCheckBox_stateChanged(int arg1);

    void on_firstCheckBox_stateChanged(int arg1);

    void on_removeButton_clicked();

private:
    std::shared_ptr<IntersectionInstance> instance;

    Ui::IntersectionInstanceModule *ui;
};

#endif // INTERSECTIONINSTANCEMODULE_H
