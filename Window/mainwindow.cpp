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

    connect(scene.get(), &Scene::objectAdded, this, &MainWindow::onObjectAdded);

    selectedHandler = scene->bindableSelected().addNotifier([&] { updateSelection(); });

    setMouseTracking(true);
    ui->controlsWidget->setScene(scene);
    ui->renderWidget->setScene(scene);
}

MainWindow::~MainWindow() {
    ui->objectsList->clearSelection();
    delete ui;
}

void MainWindow::on_addPoint_clicked() {
    shared_ptr<Object> point = make_shared<Point>(XMFLOAT3(0, 0, 0));
    scene->addObject(std::move(point));
}

void MainWindow::on_addTorus_clicked() {
    shared_ptr<Object> torus = make_shared<Torus>(XMFLOAT3(0, 0, 0));
    scene->addObject(std::move(torus));
}

void MainWindow::onObjectAdded(const std::shared_ptr<Object>& object) {
    auto item = std::make_unique<ObjectListItem>(object, scene);
    ui->objectsList->addItem(item.get());
    ui->objectsList->clearSelection();
    ui->objectsList->setCurrentItem(item.get());
    items.push_back(std::move(item));
}

void MainWindow::updateSelection() {
    shared_ptr<Object> selected;
    if (!(selected = scene->selected().lock()) || selected->type() == CURSOR) {
        ui->objectsList->clearSelection();
        return;
    }

    QSignalBlocker blocker(ui->objectsList);
    if (selected->type() == COMPOSITE) {
        auto composite = dynamic_cast<CompositeObject *>(selected.get());
        for (auto &item: items) {
            if (composite->contains(item->object())) {
                ui->objectsList->setCurrentItem(item.get(), {QItemSelectionModel::SelectionFlag::Select});
            }
        }
        ui->deleteObject->setEnabled(true);
        ui->centerObject->setEnabled(true);
        return;
    }

    for (auto &item: items) {
        if (item->hasObject(selected)) {
            ui->objectsList->setCurrentItem(item.get(), {QItemSelectionModel::SelectionFlag::ClearAndSelect});
            ui->deleteObject->setEnabled(true);
            ui->centerObject->setEnabled(true);
            return;
        }
    }
}

void MainWindow::on_objectsList_itemSelectionChanged() {
    auto selected = ui->objectsList->selectedItems();
    if (selected.empty()) {
        ui->deleteObject->setEnabled(false);
        ui->centerObject->setEnabled(false);
        scene->setSelected({});
        return;
    }

    if (selected.size() == 1) {
        if (auto o = dynamic_cast<ObjectListItem *>(selected[0])) {
            o->select();
        }
    } else {
        std::list<shared_ptr<Object>> objects{};
        for (auto &select: selected) {
            objects.push_back(dynamic_cast<ObjectListItem *>(select)->object());
        }

        selectedHandler = {};
        scene->addComposite(std::move(objects));
        selectedHandler = scene->bindableSelected().addNotifier([&] { updateSelection(); });
    }
    ui->deleteObject->setEnabled(true);
    ui->centerObject->setEnabled(true);
}

void MainWindow::on_deleteObject_clicked() {
    auto selected = ui->objectsList->selectedItems();
    if (selected.empty()) return;

    scene->removeSelected();
    items.remove_if([&](const unique_ptr<ObjectListItem> &ob) {
        return selected.contains(ob.get());
    });
}

void MainWindow::on_centerObject_clicked() {
    auto selected = ui->objectsList->selectedItems();
    if (selected.empty()) return;

    scene->centerSelected();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);

    if (event->key() == Qt::Key::Key_Delete) {
        on_deleteObject_clicked();
    }
}
