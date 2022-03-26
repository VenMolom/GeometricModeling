#include "controls.h"
#include "ui_controls.h"
#include "Objects/Torus/torus.h"
#include "Objects/Cursor/cursor.h"
#include "Objects/BrezierC0/brezierC0.h"

using namespace std;
using namespace DirectX;

Controls::Controls(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Controls) {
    ui->setupUi(this);

    ui->torusGroupBox->hide();
    ui->parametersGroupBox->hide();

    ui->pointsList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

Controls::~Controls() {
    delete ui;
}

void Controls::setScene(shared_ptr<Scene> scenePtr) {
    scene = std::move(scenePtr);
    sceneHandler = scene->bindableSelected().addNotifier([&] { updateSelected(); });
    updateSelected();
}

void Controls::updateSelected() {
    object = scene->selected().lock();
    if (object) {
        resetView();

        objectHandler = {
                object->bindablePosition().addNotifier([&] { updatePosition(); }),
                object->bindableRotation().addNotifier([&] { updateRotation(); }),
                object->bindableScale().addNotifier([&] { updateScale(); }),
                object->bindableName().addNotifier([&] { ui->nameEdit->setText(object->name()); })

        };
        updatePosition();
        updateRotation();
        updateScale();
        ui->nameEdit->setText(object->name());

        switch (object->type()) {
            case POINT3D:
                ui->rotationFrame->hide();
                ui->scaleFrame->hide();
                break;

            case COMPOSITE:
                ui->nameFrame->hide();
                break;

            case CURSOR: {
                auto *c = dynamic_cast<Cursor *>(object.get());
                ui->rotationFrame->hide();
                ui->scaleFrame->hide();
                ui->nameFrame->hide();
                ui->screenPosFrame->show();

                objectHandler.screen = c->bindableScreenPosition().addNotifier([&] { updateScreenPosition(); });
                updateScreenPosition();
                break;
            }
            case TORUS: {
                auto *t = dynamic_cast<Torus *>(object.get());
                dim = 2;
                ui->torusGroupBox->show();
                ui->parametersGroupBox->show();
                ui->uFrame->show();
                ui->vFrame->show();

                ui->majorRadius->setValue(t->majorRadius());
                ui->minorRadius->setValue(t->minorRadius());
                ui->uDensity->setValue(t->density()[0]);
                ui->vDensity->setValue(t->density()[1]);
                break;
            }
            case BREZIERC0: {
                auto *b = dynamic_cast<BrezierC0 *>(object.get());
                ui->positionFrame->hide();
                ui->rotationFrame->hide();
                ui->scaleFrame->hide();
                ui->curveGroupBox->show();

                pointsHandler = b->bindablePoints().addNotifier([&] { updateCurvePoints(); });
                updateCurvePoints();

                ui->polygonalCheckBox->setCheckState(b->drawPolygonal() ? Qt::Checked : Qt::Unchecked);
                break;
            }
        }
    } else {
        objectHandler = {};
        ui->objectGroupBox->hide();
        ui->parametersGroupBox->hide();
        ui->torusGroupBox->hide();
        ui->curveGroupBox->hide();
    }
}

void Controls::resetView() {
    ui->objectGroupBox->show();
    ui->positionFrame->show();
    ui->rotationFrame->show();
    ui->scaleFrame->show();
    ui->nameFrame->show();
    ui->screenPosFrame->hide();
    ui->torusGroupBox->hide();
    ui->curveGroupBox->hide();
    ui->parametersGroupBox->hide();
}

void Controls::setDensity() const {
    if (auto *p = dynamic_cast<ParametricObject<2> *>(object.get())) {
        p->setDensity(parameters<2>());
    }
}

void Controls::setVectorX(void (Object::*target)(DirectX::XMFLOAT3),
                          DirectX::XMFLOAT3 (Object::*source)() const, float value) const {
    auto v = (object.get()->*source)();
    v.x = value;
    (object.get()->*target)(v);
}

void Controls::setVectorY(void (Object::*target)(DirectX::XMFLOAT3),
                          DirectX::XMFLOAT3 (Object::*source)() const, float value) const {
    auto v = (object.get()->*source)();
    v.y = value;
    (object.get()->*target)(v);
}

void Controls::setVectorZ(void (Object::*target)(DirectX::XMFLOAT3),
                          DirectX::XMFLOAT3 (Object::*source)() const, float value) const {
    auto v = (object.get()->*source)();
    v.z = value;
    (object.get()->*target)(v);
}

void Controls::updatePosition() {
    QSignalBlocker xBlocker(ui->posX);
    QSignalBlocker yBlocker(ui->posY);
    QSignalBlocker zBlocker(ui->posZ);
    ui->posX->setValue(object->position().x);
    ui->posY->setValue(object->position().y);
    ui->posZ->setValue(object->position().z);
}

void Controls::updateRotation() {
    QSignalBlocker xBlocker(ui->rotX);
    QSignalBlocker yBlocker(ui->rotY);
    QSignalBlocker zBlocker(ui->rotZ);
    ui->rotX->setValue(XMConvertToDegrees(object->rotation().x));
    ui->rotY->setValue(XMConvertToDegrees(object->rotation().y));
    ui->rotZ->setValue(XMConvertToDegrees(object->rotation().z));
}

void Controls::updateScale() {
    QSignalBlocker xBlocker(ui->scaleX);
    QSignalBlocker yBlocker(ui->scaleY);
    QSignalBlocker zBlocker(ui->scaleZ);
    ui->scaleX->setValue(object->scale().x);
    ui->scaleY->setValue(object->scale().y);
    ui->scaleZ->setValue(object->scale().z);
}

void Controls::updateScreenPosition() {
    QSignalBlocker xBlocker(ui->screenPosX);
    QSignalBlocker yBlocker(ui->screenPosY);
    if (auto *c = dynamic_cast<Cursor *>(object.get())) {
        ui->screenPosX->setValue(c->screenPosition().x);
        ui->screenPosY->setValue(c->screenPosition().y);
    }
}

void Controls::updateCurvePoints() {
    points.clear();
    deletePointButtons.clear();
    ui->pointsList->clearContents();

    if (auto *c = dynamic_cast<BrezierC0 *>(object.get())) {
        ui->pointsList->setRowCount(c->points().size());
        for (auto &point: c->points()) {
            if (auto pp = point.lock()) {
                auto item = make_unique<QTableWidgetItem>(pp->name());
                auto button = make_unique<QPushButton>("Delete");
                int index = static_cast<int>(points.size());
                ui->pointsList->setItem(index, 0, item.get());
                ui->pointsList->setCellWidget(index, 1, button.get());

                connect(button.get(), &QPushButton::clicked, this, &Controls::onDeletePointButtonClicked);

                points.push_back(std::move(item));
                deletePointButtons.push_back(std::move(button));
            }
        }
        ui->pointsList->setRowCount(points.size());
    }
}

#pragma region Slots

void Controls::on_uDensity_valueChanged(int arg1) {
    setDensity();
}


void Controls::on_vDensity_valueChanged(int arg1) {
    setDensity();
}

void Controls::on_majorRadius_valueChanged(double arg1) {
    if (auto *t = dynamic_cast<Torus *>(object.get())) {
        t->setMajorRadius(static_cast<float>(arg1));
    }
}


void Controls::on_minorRadius_valueChanged(double arg1) {
    if (auto *t = dynamic_cast<Torus *>(object.get())) {
        t->setMinorRadius(static_cast<float>(arg1));
    }
}

void Controls::on_posX_valueChanged(double arg1) {
    setVectorX(&Object::setPosition, &Object::position, static_cast<float>(arg1));
}

void Controls::on_posY_valueChanged(double arg1) {
    setVectorY(&Object::setPosition, &Object::position, static_cast<float>(arg1));
}

void Controls::on_posZ_valueChanged(double arg1) {
    setVectorZ(&Object::setPosition, &Object::position, static_cast<float>(arg1));
}

void Controls::on_rotX_valueChanged(double arg1) {
    setVectorX(&Object::setRotation, &Object::rotation, XMConvertToRadians(static_cast<float>(arg1)));
}

void Controls::on_rotY_valueChanged(double arg1) {
    setVectorY(&Object::setRotation, &Object::rotation, XMConvertToRadians(static_cast<float>(arg1)));
}

void Controls::on_rotZ_valueChanged(double arg1) {
    setVectorZ(&Object::setRotation, &Object::rotation, XMConvertToRadians(static_cast<float>(arg1)));
}

void Controls::on_scaleX_valueChanged(double arg1) {
    setVectorX(&Object::setScale, &Object::scale, static_cast<float>(arg1));
}

void Controls::on_scaleY_valueChanged(double arg1) {
    setVectorY(&Object::setScale, &Object::scale, static_cast<float>(arg1));
}

void Controls::on_scaleZ_valueChanged(double arg1) {
    setVectorZ(&Object::setScale, &Object::scale, static_cast<float>(arg1));
}

void Controls::on_screenPosX_valueChanged(int arg1) {
    if (auto *c = dynamic_cast<Cursor *>(object.get())) {
        auto sp = c->screenPosition();
        sp.x = static_cast<int>(arg1);
        c->setScreenPosition(sp);
    }
}

void Controls::on_screenPosY_valueChanged(int arg1) {
    if (auto *c = dynamic_cast<Cursor *>(object.get())) {
        auto sp = c->screenPosition();
        sp.y = static_cast<int>(arg1);
        c->setScreenPosition(sp);
    }
}

void Controls::on_nameEdit_editingFinished() {
    object->setName(ui->nameEdit->text());
}

void Controls::on_polygonalCheckBox_stateChanged(int arg1) {
    if (auto *b = dynamic_cast<BrezierC0 *>(object.get())) {
        b->setDrawPolygonal(Qt::Checked == (Qt::CheckState) arg1);
    }
}

void Controls::on_pointsList_itemSelectionChanged() {
    auto selected = ui->pointsList->selectedItems();
    auto disable = selected.empty();
    ui->movePointUp->setDisabled(disable);
    ui->movePointDown->setDisabled(disable);
}

void Controls::on_movePointUp_clicked() {
    auto selected = ui->pointsList->selectedItems();
    if (selected.empty()) return;

    if (auto *b = dynamic_cast<BrezierC0 *>(object.get())) {
        b->movePoint(selected[0]->row(), Direction::UP);
    }
}

void Controls::on_movePointDown_clicked() {
    auto selected = ui->pointsList->selectedItems();
    if (selected.empty()) return;

    if (auto *b = dynamic_cast<BrezierC0 *>(object.get())) {
        b->movePoint(selected[0]->row(), Direction::DOWN);
    }
}

void Controls::onDeletePointButtonClicked() {
    if (auto *c = dynamic_cast<BrezierC0 *>(object.get())) {
        auto index = ui->pointsList->currentRow();
        c->removePoint(index);
    }
}

#pragma endregion Slots
