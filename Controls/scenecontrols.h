#ifndef SCENECONTROLS_H
#define SCENECONTROLS_H

#include <QWidget>
#include <QPushButton>
#include "scene.h"
#include "DirectX/stereoscopicRenderer.h"

namespace Ui {
    class SceneControls;
}

class SceneControls : public QWidget {
    Q_OBJECT

public:
    explicit SceneControls(QWidget *parent = nullptr);

    ~SceneControls();

    void setScene(std::shared_ptr<Scene> scenePtr);

    void setRenderer(std::unique_ptr<StereoscopicRenderer> rendererPtr);

private slots:
    void on_stereoscopicCheckBox_stateChanged(int arg1);

    void on_eyesDistance_valueChanged(double arg1);

    void on_focusDistance_valueChanged(double arg1);

    void on_leftEyeColor_clicked();

    void on_rightEyeColor_clicked();

private:
    std::shared_ptr<Scene> scene;
    std::unique_ptr<StereoscopicRenderer> renderer;

    QColor leftColor{0, 255, 255}, rightColor{255, 0, 0};

    Ui::SceneControls *ui;

    void setColor(QPushButton *item, const QColor &color);
};

#endif // SCENECONTROLS_H
