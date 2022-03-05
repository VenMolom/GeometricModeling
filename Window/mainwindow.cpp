#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Ellipsoid/ellipsoid.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow),
          scene(std::make_shared<Scene>()) {
    ui->setupUi(this);
    setMouseTracking(true);
    ui->controlsWidget->setScene(scene);
    ui->renderWidget->setScene(scene);

    connect(ui->controlsWidget, &Controls::updated, this, &MainWindow::controlsUpdated);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::controlsUpdated(int segments) {
    // TODO: edit renderer segments
    ui->renderWidget->update();
}
