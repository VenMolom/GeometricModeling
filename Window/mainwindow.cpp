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

    shared_ptr<Object> torus = make_shared<Torus>(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 0, 1),
                                                  ui->controlsWidget->minorRadius(),
                                                  ui->controlsWidget->majorRadius(),
                                                  ui->controlsWidget->parameters<2>());
    scene->addObject(std::move(torus));

    setMouseTracking(true);
    ui->controlsWidget->setScene(scene);
    ui->renderWidget->setScene(scene);
}

MainWindow::~MainWindow() {
    delete ui;
}
