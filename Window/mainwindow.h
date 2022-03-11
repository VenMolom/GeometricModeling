#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Scene/scene.h"
#include "Renderer/renderer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Renderer;

class MainWindow : public QMainWindow {
Q_OBJECT
    Ui::MainWindow *ui;
    std::shared_ptr<Scene> scene;

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;
};

#endif // MAINWINDOW_H
