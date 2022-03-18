//
// Created by Molom on 2022-03-17.
//

#include "inputHandler.h"

InputHandler::InputHandler() {

}

void InputHandler::mousePressEvent(QMouseEvent *event) {
    switch (event->button()) {
        case PAN_BUTTON:
            panButtonPressed = true;
            break;
        case MAIN_BUTTON:
            scene->addCursor(event->position().toPoint());
            break;
        case ROTATE_BUTTON:
            rotateButtonPressed = true;
            break;
    }

    lastMousePos = event->position();
}

void InputHandler::mouseReleaseEvent(QMouseEvent *event) {
    switch (event->button()) {
        case PAN_BUTTON:
            panButtonPressed = false;
            break;
        case ROTATE_BUTTON:
            rotateButtonPressed = false;
            break;
    }
}

void InputHandler::mouseMoveEvent(QMouseEvent *event) {
    if (rotateButtonPressed) {
        scene->camera().rotate(event->position() - lastMousePos);
    }
    if (panButtonPressed) {
        scene->camera().move(event->position() - lastMousePos);
    }

    lastMousePos = event->position();
}

void InputHandler::wheelEvent(QWheelEvent *event) {
    scene->camera().changeZoom(static_cast<float>(event->angleDelta().y()));
}

void InputHandler::keyPressEvent(QKeyEvent *event) {
}

void InputHandler::keyReleaseEvent(QKeyEvent *event) {
}

void InputHandler::setScene(std::shared_ptr<Scene> scenePtr) {
    scene = std::move(scenePtr);
}
