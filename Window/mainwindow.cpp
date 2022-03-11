#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Objects/Torus/torus.h"
#include <iostream>

using namespace std;
using namespace DirectX;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    scene = std::make_shared<Scene>();

    unique_ptr<Object> torus = make_unique<Torus>(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 0, 0),
                                                  0.5f, 1, array<int, 2>{4, 4});
    scene->addObject(std::move(torus));

    setMouseTracking(true);
    ui->controlsWidget->setScene(scene);
    ui->renderWidget->setScene(scene);
}

MainWindow::~MainWindow() {
    delete ui;
}
