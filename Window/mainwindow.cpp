#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Objects/Torus/torus.h"
#include "Objects/Point/point.h"
#include "Objects/CompositeObject/compositeObject.h"
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
    shared_ptr<Object> point = make_shared<Point>(XMFLOAT3(0, 0, 0));
    addObjectToScene(std::move(point));
}

void MainWindow::on_addTorus_clicked() {
    shared_ptr<Object> torus = make_shared<Torus>(XMFLOAT3(0, 0, 0));
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

    shared_ptr<Object> selected;
    if (!(selected = scene->selected().lock())) {
        return;
    }

    if (scene->selected().lock()->type() == COMPOSITE) {
        // TODO: handle multiple from screen
    }

    if (selected->type() != CURSOR) {
        for (auto &item : items) {
            if (item->hasObject(selected)) {
                ui->objectsList->setCurrentItem(item.get());
            }
        }
    }
}

void MainWindow::on_objectsList_itemSelectionChanged() {
    auto selected = ui->objectsList->selectedItems();
    if (selected.empty()) {
        ui->deleteObject->setEnabled(false);
        return;
    }

    if (selected.size() == 1) {
        if (auto o = dynamic_cast<ObjectListItem *>(selected[0])) {
            o->select();
        }
    } else {
        std::list<shared_ptr<Object>> objects{};
        for (auto &select : selected) {
            objects.push_back(dynamic_cast<ObjectListItem*>(select)->object());
        }
        shared_ptr<Object> composite = make_shared<CompositeObject>(std::move(objects));

        selectedHandler = {};
        scene->setSelected(composite);
        selectedHandler = scene->bindableSelected().addNotifier([&] { updateSelection(); });
    }
    ui->deleteObject->setEnabled(true);
}

void MainWindow::on_deleteObject_clicked() {
    auto selected = ui->objectsList->selectedItems();
    items.remove_if([&] (const unique_ptr<ObjectListItem> &ob) {
       return selected.contains(ob.get());
    });
}
