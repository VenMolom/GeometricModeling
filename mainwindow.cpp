#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Objects/Point/point.h"
#include "Objects/Parametric/torus.h"
#include "Objects/Curve/brezierC0.h"
#include "Objects/Curve/brezierC2.h"
#include "Objects/Curve/interpolationCurveC2.h"
#include "Objects/CompositeObject/compositeObject.h"

using namespace std;
using namespace DirectX;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    scene = std::make_shared<Scene>();

    connect(scene.get(), &Scene::objectAdded, this, &MainWindow::onObjectAdded);

    selectedHandler = scene->bindableSelected().addNotifier([this] { updateSelection(); });

    setMouseTracking(true);
    ui->controlsWidget->setScene(scene);
    ui->renderWidget->setScene(scene);
    ui->renderWidget->setStatusBar(ui->statusbar);
}

MainWindow::~MainWindow() {
    ui->objectsList->clearSelection();
    delete ui;
}

void MainWindow::on_addPoint_clicked() {
    scene->addObject(std::move(scene->objectFactory().createPoint(XMFLOAT3(0, 0, 0))));
}

void MainWindow::on_addTorus_clicked() {
    scene->addObject(std::move(scene->objectFactory().createTorus(XMFLOAT3(0, 0, 0))));
}

void MainWindow::on_addBrezierC0_clicked() {
    scene->addObject(std::move(scene->objectFactory().createBrezierC0(std::move(getSelectedPoints()))));
}

void MainWindow::on_addBrezierC2_clicked() {
    scene->addObject(std::move(scene->objectFactory().createBrezierC2(std::move(getSelectedPoints()), scene->bindableSelected())));
}

void MainWindow::on_addInterpolationC2_clicked() {
    scene->addObject(std::move(scene->objectFactory().createInterpolationCurveC2(std::move(getSelectedPoints()))));
}

void MainWindow::onObjectAdded(const std::shared_ptr<Object> &object, bool select) {
    auto item = std::make_unique<ObjectListItem>(object, scene);
    ui->objectsList->addItem(item.get());

    if (select) {
        ui->objectsList->clearSelection();
        ui->objectsList->setCurrentItem(item.get());
    }
    items.push_back(std::move(item));
}

void MainWindow::updateSelection() {
    shared_ptr<Object> selected;
    if (!(selected = scene->selected().lock())) {
        ui->objectsList->clearSelection();
        ui->deleteObject->setEnabled(false);
        ui->centerObject->setEnabled(false);
        return;
    }

    QSignalBlocker blocker(ui->objectsList);
    if (selected->type() & VIRTUAL) {
        ui->objectsList->clearSelection();
        ui->deleteObject->setEnabled(false);
        ui->centerObject->setEnabled(false);
        return;
    }

    if (selected->type() & COMPOSITE) {
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

std::vector<std::weak_ptr<Point>> MainWindow::getSelectedPoints() {
    auto selected = ui->objectsList->selectedItems();

    std::vector<weak_ptr<Point>> objects{};
    for (auto &select: selected) {
        auto ob = dynamic_cast<ObjectListItem *>(select)->object();
        weak_ptr<Point> p = dynamic_pointer_cast<Point>(ob);
        if (p.lock()) {
            objects.push_back(p);
        }
    }

    return std::move(objects);
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
        shared_ptr<Object> sel;
        QListWidgetItem *newSelected;
        if ((sel = scene->selected().lock()) && sel->type() & CURVE) {
            auto *b = dynamic_cast<Curve *>(sel.get());
            for (auto &select: selected) {
                auto ob = dynamic_cast<ObjectListItem *>(select)->object();
                if (ob->type() & POINT3D) {
                    shared_ptr<Point> p = static_pointer_cast<Point>(ob);
                    b->addPoint(p);
                } else if (ob->type() & CURVE) {
                    newSelected = select;
                }
            }
            QSignalBlocker blocker(ui->objectsList);
            ui->objectsList->setCurrentItem(newSelected, {QItemSelectionModel::SelectionFlag::ClearAndSelect});
            return;
        }

        std::list<shared_ptr<Object>>
                objects{};
        for (auto &select: selected) {
            auto object = dynamic_cast<ObjectListItem *>(select)->object();
            if (!(object->type() & COMPOSABLE)) continue;
            objects.push_back(std::move(object));
        }

        selectedHandler = {};
        scene->addComposite(std::move(objects));
        selectedHandler = scene->bindableSelected().addNotifier([this] { updateSelection(); });
    }
    ui->deleteObject->setEnabled(true);
    ui->centerObject->setEnabled(true);
}

void MainWindow::on_deleteObject_clicked() {
    auto selected = ui->objectsList->selectedItems();
    if (selected.empty()) return;

    scene->removeSelected();
    items.remove_if([&selected](const unique_ptr<ObjectListItem> &ob) {
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

    ui->renderWidget->handleKeyEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    QWidget::keyReleaseEvent(event);

    ui->renderWidget->handleKeyEvent(event);
}