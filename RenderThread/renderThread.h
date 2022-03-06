//
// Created by Molom on 2022-03-06.
//

#ifndef MG1_RENDERTHREAD_H
#define MG1_RENDERTHREAD_H

#define SLEEP 50
#define BACKGROUND_COLOR 4278848010 // 10,10,10

#include <QThread>
#include <QLabel>
#include "Scene/scene.h"

class RenderThread : public QThread {
    Q_OBJECT
    int startingSegments;
    bool restart;
    bool working;
    std::shared_ptr<Scene> scene;
    QLabel *target;

public:

    explicit RenderThread(QLabel *target, std::shared_ptr<Scene> scene, int startingSegments, QObject* parent = nullptr);

    void finish();

    void setStartingSegments(int startingSegments);

    void update();

protected:
    void run() override;
};


#endif //MG1_RENDERTHREAD_H
