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
    selectedHandler = scene->bindableSelected().addNotifier([&] { updateSelection(); });

    setMouseTracking(true);
    ui->controlsWidget->setScene(scene);
    ui->renderWidget->setScene(scene);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_addPoint_clicked() {
    shared_ptr<Object> point = make_shared<Point>(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 0, 1));
    addObjectToScene(std::move(point));
}

void MainWindow::on_addTorus_clicked() {
    shared_ptr<Object> torus = make_shared<Torus>(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 0, 1));
    addObjectToScene(std::move(torus));
}

void MainWindow::addObjectToScene(std::shared_ptr<Object> &&object) {
    auto item = std::make_unique<ObjectListItem>(object, scene);
    ui->objectsList->addItem(item.get());
    ui->objectsList->clearSelection();
    ui->objectsList->setCurrentItem(item.get());
    items.push_back(std::move(item));
    scene->addObject(std::move(object));
}

void MainWindow::updateSelection() {
    ui->objectsList->clearSelection();

    shared_ptr<Object> ob;
    if ((ob = scene->selected().lock()) && ob->type() != CURSOR) {
        for (auto &item : items) {
            // TODO: handle composite object
            if (item->hasObject(ob)) {
                ui->objectsList->setCurrentItem(item.get());
            }
        }
    }
}

void MainWindow::on_objectsList_itemSelectionChanged() {
    auto selected = ui->objectsList->selectedItems();
    if (selected.empty()) {
        ui->deleteObject->setEnabled(false);
    } else {
        ui->deleteObject->setEnabled(true);
    }
    if (selected.size() == 1) {
        if (auto o = dynamic_cast<ObjectListItem *>(selected[0])) {
            o->select();
        }
    }

    // TODO: composite object
}

void MainWindow::on_deleteObject_clicked() {
    auto selected = ui->objectsList->selectedItems();
    items.remove_if([&] (const unique_ptr<ObjectListItem> &ob) {
       return selected.contains(ob.get());
    });
}
