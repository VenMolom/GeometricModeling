//
// Created by Molom on 2022-03-05.
//

#ifndef MG1_RENDERER_H
#define MG1_RENDERER_H

#include <QWidget>
#include <QWheelEvent>
#include <QThread>
#include "Scene/scene.h"
#include "RenderThread/renderThread.h"

namespace Ui {
    class Renderer;
}

class Renderer : public QWidget {
    Q_OBJECT
    Ui::Renderer *ui;
    std::shared_ptr<Scene> scene;

    std::unique_ptr<RenderThread> renderThread;

    QPointF lastMousePos;
    bool mouseButtonPressed;
    bool moveButtonPressed;

    void initialiseThread(int startingSegments = 2);

public:
    explicit Renderer(QWidget *parent, std::shared_ptr<Scene> scenePtr = nullptr);

    ~Renderer();

    void setScene(std::shared_ptr<Scene> scenePtr);

    void update(bool updateSize = false);

    void update(int startingSegments);

protected:
    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void showEvent(QShowEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;
};


#endif //MG1_RENDERER_H
