#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Controls/intersectdialog.h"
#include "Utils/pathsCreator.h"
#include <QFileDialog>
#include <QMessageBox>

using namespace std;
using namespace DirectX;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    scene = std::make_shared<Scene>();

    connect(scene.get(), &Scene::objectAdded, this, &MainWindow::onObjectAdded);

    selectedHandler = scene->bindableSelected().addNotifier([this] { updateSelection(); });

    setMouseTracking(true);
    ui->selectedControls->setScene(scene);
    ui->sceneControls->setScene(scene);
    ui->sceneControls->setRenderer(unique_ptr<DxRenderer>(ui->renderWidget));
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
    scene->addObject(std::move(
            scene->objectFactory().createBrezierC2(std::move(getSelectedPoints()), scene->bindableSelected())));
}

void MainWindow::on_addInterpolationC2_clicked() {
    scene->addObject(std::move(scene->objectFactory().createInterpolationCurveC2(std::move(getSelectedPoints()))));
}

void MainWindow::on_addPatchC0_clicked() {
    scene->addCreator(std::move(scene->objectFactory().createBicubicC0(XMFLOAT3(0, 0, 0), scene->bindableSelected())));
}

void MainWindow::on_addPatchC2_clicked() {
    scene->addCreator(std::move(scene->objectFactory().createBicubicC2(XMFLOAT3(0, 0, 0), scene->bindableSelected())));
}

void MainWindow::on_addCNCRouter_clicked() {
    scene->addObject(std::move(scene->objectFactory().createCNCRouter(XMFLOAT3(0, 0, 0))));
}


void MainWindow::onObjectAdded(const std::shared_ptr<Object> &object, bool select) {
    auto item = make_unique<ObjectListItem>(object, scene);
    ui->objectsList->addItem(item.get());

    if (select) {
        ui->objectsList->clearSelection();
        ui->objectsList->setCurrentItem(item.get());
    }
    items.push_back(std::move(item));
}

void MainWindow::updateSelection() {
    ui->actionCollapse_Points->setEnabled(false);
    ui->actionFill_In->setEnabled(false);
    ui->actionIntersect->setEnabled(false);
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

    if (selected->type() & PARAMETRIC) {
        ui->actionIntersect->setEnabled(true);
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
        ui->actionCollapse_Points->setEnabled(composite->collapsable());
        ui->actionFill_In->setEnabled(composite->fillable());
        ui->actionIntersect->setEnabled(composite->intersectable());
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
    ui->actionCollapse_Points->setEnabled(false);
    ui->actionFill_In->setEnabled(false);
    ui->actionIntersect->setEnabled(false);
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

            if (o->type() & PARAMETRIC) {
                ui->actionIntersect->setEnabled(true);
            }
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
        auto comp = scene->addComposite(std::move(objects));
        if (comp) {
            ui->actionCollapse_Points->setEnabled(comp->collapsable());
            ui->actionFill_In->setEnabled(comp->fillable());
            ui->actionIntersect->setEnabled(comp->intersectable());
        }
        selectedHandler = scene->bindableSelected().addNotifier([this] { updateSelection(); });
    }
    ui->deleteObject->setEnabled(true);
    ui->centerObject->setEnabled(true);
}

void MainWindow::on_actionCollapse_Points_triggered() {
    scene->collapseSelected();
}

void MainWindow::on_actionFill_In_triggered() {
    scene->fillIn();
}

void MainWindow::on_actionIntersect_triggered() {
    IntersectHandler handler{scene->hasCursor(), scene->objectFactory()};
    IntersectDialog dialog(this, handler);

    if (!dialog.exec()) return;

    scene->intersect(handler, *ui->renderWidget);
}

void MainWindow::on_deleteObject_clicked() {
    auto selected = ui->objectsList->selectedItems();
    if (selected.empty()) return;

    auto sel = scene->selected().lock();
    scene->removeSelected();
    if (sel) {
        items.remove_if([&sel](const unique_ptr<ObjectListItem> &ob) {
            return sel->equals(ob->object());
        });
    }
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

void MainWindow::on_actionSave_triggered() {
    QFileDialog dialog(this, "Save scene file");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(QDir::Files);
    dialog.setDirectory("./Saves");
    dialog.setNameFilter("Scene (*.json)");
    dialog.selectFile("scene.json");

    if (!dialog.exec()) return;

    auto fileName = dialog.selectedFiles()[0];

    try {
        scene->serialize(MG1::Scene::Get());
        serializer.SaveScene(fileName.toStdString());
    } catch (...) {
        QMessageBox::warning(this, "Save error", "Failed to save scene");
    }
}


void MainWindow::on_actionLoad_triggered() {
    QFileDialog dialog(this, "Select scene file");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setFilter(QDir::Files);
    dialog.setDirectory("./Saves");
    dialog.setNameFilter("Scene (*.json)");

    if (!dialog.exec()) return;

    auto fileName = dialog.selectedFiles()[0];

    try {
        auto loaded = serializer.LoadScene(fileName.toStdString());

        ui->objectsList->clearSelection();
        items.clear();
        scene->load(loaded);
    } catch (...) {
        QMessageBox::warning(this, "Load error", "Failed to load scene");
    }
}

void MainWindow::on_actionGenerate_paths_triggered() {
    QFileDialog dialog(this, "Select paths base directory");
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setFilter(QDir::Dirs);
    dialog.setDirectory("./Paths");

    if (!dialog.exec()) return;

    auto path = filesystem::path(dialog.selectedFiles()[0].toStdString());
    PathsCreator::create(path, scene->objects(), *ui->renderWidget);
}

