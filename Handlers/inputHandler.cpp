//
// Created by Molom on 2022-03-17.
//

#include "inputHandler.h"

InputHandler::InputHandler() {

}

void InputHandler::mousePressEvent(QMouseEvent *event) {
    switch (event->button()) {
        case MAIN_BUTTON:
            if (subactionKeyPressed) {
                scene->addPoint(event->position().toPoint());
            } else if (mode == ScreenTransform::NONE) {
                scene->selectOrAddCursor(event->position().toPoint(), actionKeyPressed);
            }
            break;
    }

    lastMousePos = event->position();
}

void InputHandler::mouseReleaseEvent(QMouseEvent *event) {
    switch (event->button()) {
    }
}

void InputHandler::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & ROTATE_BUTTON) {
        scene->camera()->rotate(event->position() - lastMousePos);
    }
    if (event->buttons() & PAN_BUTTON) {
        scene->camera()->move(event->position() - lastMousePos);
    }
    if (event->buttons() & MAIN_BUTTON) {
        if (transformHandler) {
            transformHandler->transform(event->position().toPoint(), event->position() - lastMousePos);
        }
    }

    lastMousePos = event->position();
}

void InputHandler::wheelEvent(QWheelEvent *event) {
    if (event->buttons() & PAN_BUTTON) return;

    scene->camera()->changeZoom(static_cast<float>(event->angleDelta().y()));
}

void InputHandler::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case ACTION_KEY:
            actionKeyPressed = true;
            break;
        case SUBACTION_KEY:
            subactionKeyPressed = true;
            break;
        case SELECT_KEY:
            mode = ScreenTransform::NONE;
        case MOVE_KEY:
            mode = ScreenTransform::MOVE;
        case ROTATE_KEY:
            mode = ScreenTransform::ROTATE;
        case SCALE_KEY:
            mode = ScreenTransform::SCALE;
    }
}

void InputHandler::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
        case ACTION_KEY:
            actionKeyPressed = false;
            break;
        case SUBACTION_KEY:
            subactionKeyPressed = false;
            break;
    }
}

void InputHandler::setScene(std::shared_ptr<Scene> scenePtr) {
    scene = std::move(scenePtr);
}

void InputHandler::focusLost() {
    actionKeyPressed = false;
    subactionKeyPressed = false;
}
