//
// Created by Molom on 2022-03-17.
//

#ifndef MG1_INPUTHANDLER_H
#define MG1_INPUTHANDLER_H

#include <QWheelEvent>
#include "Scene/scene.h"

#define PAN_BUTTON Qt::MouseButton::MiddleButton
#define MAIN_BUTTON Qt::MouseButton::LeftButton
#define ROTATE_BUTTON Qt::MouseButton::RightButton
#define MULTI_SELECT_KEY Qt::Key::Key_Shift

class InputHandler {
public:
    InputHandler();

    void mousePressEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);

    void keyPressEvent(QKeyEvent *event);

    void keyReleaseEvent(QKeyEvent *event);

    void setScene(std::shared_ptr<Scene> scenePtr);

private:
    std::shared_ptr<Scene> scene;

    QPointF lastMousePos{};
    bool panButtonPressed{false};
    bool rotateButtonPressed{false};
    bool multiSelectButtonPressed{false};
};


#endif //MG1_INPUTHANDLER_H
