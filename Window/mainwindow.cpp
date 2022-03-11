#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>

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
