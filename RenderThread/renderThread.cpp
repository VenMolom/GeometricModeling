//
// Created by Molom on 2022-03-06.
//

#include "renderThread.h"

#include <utility>

RenderThread::RenderThread(QLabel *target, std::shared_ptr<Scene> scene, int startingSegments, QObject *parent)
        : QThread(parent),
          target(target),
          scene(std::move(scene)),
          startingSegments(startingSegments),
          restart(false),
          working(true) {

}

void RenderThread::finish() {
    working = false;
}

void RenderThread::setStartingSegments(int startingSegments) {
    this->startingSegments = startingSegments;
}

void RenderThread::update() {
    restart = true;
}

void RenderThread::run() {
    auto currentSegments = 1 << startingSegments;

    QImage image;
    while (working) {

        if (restart) {
            currentSegments = 1 << startingSegments;
            restart = false;
            msleep(SLEEP);
            continue;
        }

        auto targetSize = target->size();

        if (currentSegments >= targetSize.width() && currentSegments >= targetSize.height()) {
            msleep(SLEEP);
            continue;
        }

        image = QImage(std::min(currentSegments, targetSize.width()),
                       std::min(currentSegments, targetSize.height()), QImage::Format_ARGB32);

        image.fill(BACKGROUND_COLOR);
        scene->draw(image);
        target->setPixmap(QPixmap::fromImage(image).scaled(targetSize.width(), targetSize.height()));

        if (currentSegments < targetSize.width() || currentSegments < targetSize.height()) {
            currentSegments <<= 1;
        }

        msleep(SLEEP);
    }
}
