#include "transformmodule.h"
#include "ui_transformmodule.h"

using namespace std;
using namespace DirectX;

TransformModule::TransformModule(shared_ptr<Object> object, QWidget *parent) :
    QWidget(parent),
    object(std::move(object)),
    ui(new Ui::TransformModule)
{
    ui->setupUi(this);
    rotationHandler = this->object->bindableRotation().addNotifier([this] { updateRotation(); });
    scaleHandler = this->object->bindableScale().addNotifier([this] { updateScale(); });
    updateRotation();
    updateScale();
}

TransformModule::~TransformModule()
{
    delete ui;
}

void TransformModule::updateRotation() {
    QSignalBlocker xBlocker(ui->rotX);
    QSignalBlocker yBlocker(ui->rotY);
    QSignalBlocker zBlocker(ui->rotZ);
    ui->rotX->setValue(XMConvertToDegrees(object->rotation().x));
    ui->rotY->setValue(XMConvertToDegrees(object->rotation().y));
    ui->rotZ->setValue(XMConvertToDegrees(object->rotation().z));
}

void TransformModule::updateScale() {
    QSignalBlocker xBlocker(ui->scaleX);
    QSignalBlocker yBlocker(ui->scaleY);
    QSignalBlocker zBlocker(ui->scaleZ);
    ui->scaleX->setValue(object->scale().x);
    ui->scaleY->setValue(object->scale().y);
    ui->scaleZ->setValue(object->scale().z);
}

void TransformModule::on_rotX_valueChanged(double arg1) {
    auto pos = object->rotation();
    pos.x = XMConvertToRadians(static_cast<float>(arg1));
    object->setRotation(pos);
}

void TransformModule::on_rotY_valueChanged(double arg1) {
    auto pos = object->rotation();
    pos.y = XMConvertToRadians(static_cast<float>(arg1));
    object->setRotation(pos);
}

void TransformModule::on_rotZ_valueChanged(double arg1) {
    auto pos = object->rotation();
    pos.z = XMConvertToRadians(static_cast<float>(arg1));
    object->setRotation(pos);
}

void TransformModule::on_scaleX_valueChanged(double arg1) {
    auto pos = object->scale();
    pos.x = arg1;
    object->setScale(pos);
}

void TransformModule::on_scaleY_valueChanged(double arg1) {
    auto pos = object->scale();
    pos.y = arg1;
    object->setScale(pos);
}

void TransformModule::on_scaleZ_valueChanged(double arg1) {
    auto pos = object->scale();
    pos.z = arg1;
    object->setScale(pos);
}