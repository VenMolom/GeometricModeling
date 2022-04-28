//
// Created by Molom on 2022-03-17.
//

#include "inputHandler.h"

using namespace std;

void InputHandler::mousePressEvent(QMouseEvent *event) {
    clickPos = event->position();

    switch (event->button()) {
        case MAIN_BUTTON:
            if (subactionKeyPressed) {
                scene->addPoint(event->position().toPoint());
            } else if (mode == ScreenTransform::NONE) {
                scene->selectOrAddCursor(event->position().toPoint(), actionKeyPressed);
            }

            if (!scene->selected().expired()) {
                transformHandler = make_unique<ScreenTransform>(
                        scene->selected().lock(), scene->camera(),
                        mode == ScreenTransform::NONE ? ScreenTransform::MOVE : mode, lockAxis);

                if (mode != ScreenTransform::NONE) {
                    transformHandler->transform(event->position().toPoint(), event->position() - clickPos);
                }
            } else {
                transformHandler = {};
            }

            break;
    }

    lastMousePos = event->position();
}

void InputHandler::mouseReleaseEvent(QMouseEvent *event) {
    transformHandler = {};

//    switch (event->button()) {
//    }
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
            transformHandler->transform(event->position().toPoint(), event->position() - clickPos);
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
            break;
        case MOVE_KEY:
            mode = ScreenTransform::MOVE;
            break;
        case ROTATE_KEY:
            mode = ScreenTransform::ROTATE;
            break;
        case SCALE_KEY:
            mode = ScreenTransform::SCALE;
            break;
    }
//    if (transformHandler) {
//        transformHandler->changeMode(mode);
//        transformHandler->changeLockAxis(lockAxis);
//    }
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

QString InputHandler::currentMode() {
    switch (mode) {
        case ScreenTransform::NONE:
            return "SELECT";
        case ScreenTransform::MOVE:
            return "MOVE";
        case ScreenTransform::ROTATE:
            return "ROTATE";
        case ScreenTransform::SCALE:
            return "SCALE";
    }
}
