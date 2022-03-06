#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Ellipsoid/ellipsoid.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    Ellipsoid ellipsoid(QVector3D(), ui->controlsWidget->getRadius(), QColor(244, 146, 41), ui->controlsWidget->getSpecular());
    scene = std::make_shared<Scene>(std::move(ellipsoid));

    setMouseTracking(true);
    ui->controlsWidget->setScene(scene);
    ui->renderWidget->setScene(scene);
    ui->renderWidget->update(ui->controlsWidget->getRenderingSegments());

    connect(ui->controlsWidget, &Controls::updated, this, &MainWindow::controlsUpdated);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::controlsUpdated(int segments) {
    ui->renderWidget->update(segments);
}
