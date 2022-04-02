#ifndef CONTROLLS_H
#define CONTROLLS_H

#include <QPushButton>
#include "Scene/scene.h"

namespace Ui {
    class Controls;
}

class Controls : public QWidget {
    Q_OBJECT

public:
    explicit Controls(QWidget *parent = nullptr);

    ~Controls();

    void setScene(std::shared_ptr<Scene> scenePtr);

private:
    QPropertyNotifier sceneHandler;

    std::shared_ptr<Scene> scene;
    std::vector<std::unique_ptr<QWidget>> modules;

    void updateSelected();

    Ui::Controls *ui;
};

#endif // CONTROLLS_H
