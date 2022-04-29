//
// Created by Molom on 2022-03-17.
//

#ifndef MG1_INPUTHANDLER_H
#define MG1_INPUTHANDLER_H

#include <QWheelEvent>
#include "scene.h"
#include "Handlers/screenTransform.h"

#define PAN_BUTTON Qt::MouseButton::MiddleButton
#define MAIN_BUTTON Qt::MouseButton::LeftButton
#define ROTATE_BUTTON Qt::MouseButton::RightButton

#define ACTION_KEY Qt::Key::Key_Shift
#define SUBACTION_KEY Qt::Key::Key_Control

#define SELECT_KEY Qt::Key::Key_Q
#define MOVE_KEY Qt::Key::Key_W
#define ROTATE_KEY Qt::Key::Key_E
#define SCALE_KEY Qt::Key::Key_R

#define FREE_AXIS_KEY Qt::Key::Key_V
#define X_AXIS_KEY Qt::Key::Key_X
#define Y_AXIS_KEY Qt::Key::Key_C
#define Z_AXIS_KEY Qt::Key::Key_Z

class InputHandler {
public:
    void mousePressEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);

    void keyPressEvent(QKeyEvent *event);

    void keyReleaseEvent(QKeyEvent *event);

    void setScene(std::shared_ptr<Scene> scenePtr);

    void focusLost();

    QString currentMode();

    ScreenTransform::Axis lockAxis() { return axis; }

private:
    std::shared_ptr<Scene> scene;

    QPointF lastMousePos{};
    QPointF clickPos{};
    bool actionKeyPressed{false};
    bool subactionKeyPressed{false};
    ScreenTransform::Transform mode{ScreenTransform::NONE};
    ScreenTransform::Axis axis{ScreenTransform::FREE};
    std::unique_ptr<ScreenTransform> transformHandler;

    // TODO: works as before for mode = NONE (select), click selects and can move while holding
    // TODO: other modes only work on currently selected object
};


#endif //MG1_INPUTHANDLER_H
