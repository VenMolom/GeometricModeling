#ifndef CONTROLS_H
#define CONTROLS_H

#include <QPushButton>
#include "scene.h"

namespace Ui {
    class Controls;
}

class Controls : public QWidget {
    Q_OBJECT

public:
    explicit Controls(QWidget *parent = nullptr);

    ~Controls();

    void setScene(std::shared_ptr<Scene> scenePtr);

public slots:
    void removeIntersection();

private:
    QPropertyNotifier sceneHandler;

    std::shared_ptr<Scene> scene;
    std::vector<std::unique_ptr<QWidget>> modules;
    std::weak_ptr<Object> object;

    void updateSelected();

    Ui::Controls *ui;
};

#endif // CONTROLS_H
