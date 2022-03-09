//
// Created by Molom on 2022-03-05.
//

#include "renderer.h"
#include "ui_renderer.h"

#include <utility>

Renderer::Renderer(QWidget *parent, std::shared_ptr<Scene> scenePtr)
        : QWidget(parent),
          ui(new Ui::Renderer),
          scene(std::move(scenePtr)),
          mouseButtonPressed(false),
          moveButtonPressed(false),
          lastMousePos() {
    ui->setupUi(this);
}

void Renderer::update(bool updateSize) {
    if (updateSize) {
        auto size = ui->label->size();
        scene->getCamera().resize(size);
    }

    if (!renderThread) {
        initialiseThread();
        return;
    }

    renderThread->update(updateSize);
}

void Renderer::update(int startingSegments) {
    if (!renderThread) {
        initialiseThread(startingSegments);
        return;
    }

    renderThread->setStartingSegments(startingSegments);
    renderThread->update();
}

void Renderer::setScene(std::shared_ptr<Scene> scenePtr) {
    scene = std::move(scenePtr);
}

void Renderer::initialiseThread(int startingSegments) {
    renderThread = std::make_unique<RenderThread>(ui->label, scene, startingSegments, this);

    connect(renderThread.get(), &RenderThread::finished, renderThread.get(), &QObject::deleteLater);
    renderThread->start();
}


Renderer::~Renderer() {
    delete ui;

    renderThread->finish();
    renderThread->wait();
}

#pragma region Event_Handlers

void Renderer::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);

    if (event->button() == Qt::MouseButton::LeftButton) {
        mouseButtonPressed = true;
        lastMousePos = event->position();
    }
}

void Renderer::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);

    if (event->button() == Qt::MouseButton::LeftButton) {
        mouseButtonPressed = false;
        lastMousePos = QPoint();
    }
}

void Renderer::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);

    if (!mouseButtonPressed) return;

    if (moveButtonPressed) {
        scene->getCamera().move(event->position() - lastMousePos);
        update();
    } else if (!lastMousePos.isNull()) {
        scene->getCamera().rotate(event->position() - lastMousePos);
        update();
    }

    lastMousePos = event->position();
}

void Renderer::wheelEvent(QWheelEvent *event) {
    QWidget::wheelEvent(event);

    scene->getCamera().changeZoom(event->angleDelta().y(), ui->label->size());
    update();
}

void Renderer::resizeEvent(QResizeEvent *event) {
    // leads to QLabel::setPixmap crash when fired too often
    QWidget::resizeEvent(event);

    update(true);
}

void Renderer::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    update(true);
}

void Renderer::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);

    if (event->key() == Qt::Key::Key_Shift) {
        moveButtonPressed = true;
    }
}

void Renderer::keyReleaseEvent(QKeyEvent *event) {
    QWidget::keyReleaseEvent(event);

    if (event->key() == Qt::Key::Key_Shift) {
        moveButtonPressed = false;
    }
}

#pragma endregion Event_Handlers