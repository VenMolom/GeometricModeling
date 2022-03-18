#include "controls.h"
#include "ui_controls.h"
#include "Objects/Torus/torus.h"
#include "Objects/Cursor/cursor.h"

using namespace std;
using namespace DirectX;

Controls::Controls(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Controls) {
    ui->setupUi(this);

    ui->torusGroupBox->hide();
    ui->parametersGroupBox->hide();
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
        ui->objectGroupBox->show();
        ui->rotationFrame->show();
        ui->scaleFrame->show();
        ui->screenPosFrame->hide();


        objectHandler = {
                object->bindablePosition().addNotifier([&] { updatePosition(); }),
                object->bindableRotation().addNotifier([&] { updateRotation(); }),
                object->bindableScale().addNotifier([&] { updateScale(); })
        };
        updatePosition();
        updateRotation();
        updateScale();

        switch (object->type()) {
            case CURSOR:
                auto *c = dynamic_cast<Cursor *>(object.get());
                ui->rotationFrame->hide();
                ui->scaleFrame->hide();
                ui->screenPosFrame->show();

                ui->screenPosX->setValue(c->screenPosition().x);
                ui->screenPosY->setValue(c->screenPosition().y);

            case TORUS:
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
        }
    } else {
        ui->objectGroupBox->hide();
        ui->parametersGroupBox->hide();
        ui->torusGroupBox->hide();
    }
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
    ui->posX->setValue(object->position().x);
    ui->posY->setValue(object->position().y);
    ui->posZ->setValue(object->position().z);
}

void Controls::updateRotation() {
    ui->rotX->setValue(XMConvertToDegrees(object->rotation().x));
    ui->rotY->setValue(XMConvertToDegrees(object->rotation().y));
    ui->rotZ->setValue(XMConvertToDegrees(object->rotation().z));
}

void Controls::updateScale() {
    ui->scaleX->setValue(object->scale().x);
    ui->scaleY->setValue(object->scale().y);
    ui->scaleZ->setValue(object->scale().z);
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

#pragma endregion Slots
