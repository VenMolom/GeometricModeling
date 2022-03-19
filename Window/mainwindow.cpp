#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Objects/Torus/torus.h"
#include "Objects/Point/point.h"
#include <iostream>

using namespace std;
using namespace DirectX;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    scene = std::make_shared<Scene>();

    setMouseTracking(true);
    ui->controlsWidget->setScene(scene);
    ui->renderWidget->setScene(scene);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_addPoint_clicked() {
    shared_ptr<Object> point = make_shared<Point>(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 0, 1));
    ui->objectsList->insertItem(0, point->name());
    scene->addObject(std::move(point));
}

void MainWindow::on_addTorus_clicked() {
    shared_ptr<Object> torus = make_shared<Torus>(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 0, 1));
    ui->objectsList->insertItem(0, torus->name());
    scene->addObject(std::move(torus));
}

void MainWindow::on_objectsList_itemSelectionChanged() {
    auto a = ui->objectsList->selectedItems();
    auto b = a.size();
}

