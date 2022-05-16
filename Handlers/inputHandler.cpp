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
                if (mode == ScreenTransform::NONE && scene->selected().lock()->type() & CURSOR) {
                    screenSelectActive = true;
                    screenSelectStart = event->position();
                    break;
                }

                transformHandler = make_unique<ScreenTransform>(
                        scene->selected().lock(), event->position().toPoint(), scene->camera(),
                        mode == ScreenTransform::NONE ? ScreenTransform::MOVE : mode, axis);

                if (mode != ScreenTransform::NONE) {
                    transformHandler->transform(event->position().toPoint());
                }
            } else {
                transformHandler = {};
            }

            break;
    }

    lastMousePos = event->position();
}

void InputHandler::mouseReleaseEvent(QMouseEvent *event) {
    if (!(event->buttons() & MAIN_BUTTON)) {
        transformHandler = {};
        if (screenSelectActive && (event->position() - screenSelectStart).manhattanLength() > SCREEN_SELECT_MIN_MOVE) {
            scene->selectFromScreen(screenSelectStart, event->position());
            screenSelectActive = false;
        }
    }
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
        if (screenSelectActive) {
            // TODO: update selector end in renderer
        }

        if (transformHandler && event->buttons() == MAIN_BUTTON) {
            transformHandler->transform(event->position().toPoint());
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
            axis = ScreenTransform::FREE;
            break;
        case MOVE_KEY:
            if (transformHandler) break;
            mode = ScreenTransform::MOVE;
            break;
        case ROTATE_KEY:
            if (transformHandler) break;
            mode = ScreenTransform::ROTATE;
            break;
        case SCALE_KEY:
            if (transformHandler) break;
            mode = ScreenTransform::SCALE;
            break;
        case FREE_AXIS_KEY:
            if (transformHandler) break;
            axis = ScreenTransform::FREE;
            break;
        case X_AXIS_KEY:
            axis = ScreenTransform::X;
            break;
        case Y_AXIS_KEY:
            axis = ScreenTransform::Y;
            break;
        case Z_AXIS_KEY:
            axis = ScreenTransform::Z;
            break;
    }

    if (transformHandler) {
        transformHandler->setAxis(axis);
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
