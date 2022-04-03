#include "movemodule.h"
#include "ui_movemodule.h"

using namespace std;

MoveModule::MoveModule(shared_ptr<Object> object, QWidget *parent) :
    QWidget(parent),
    object(std::move(object)),
    ui(new Ui::MoveModule)
{
    ui->setupUi(this);
    positionHandler = this->object->bindablePosition().addNotifier([this] { updatePosition(); });
    updatePosition();
}

MoveModule::~MoveModule()
{
    delete ui;
}

void MoveModule::updatePosition() {
    QSignalBlocker xBlocker(ui->posX);
    QSignalBlocker yBlocker(ui->posY);
    QSignalBlocker zBlocker(ui->posZ);
    ui->posX->setValue(object->position().x);
    ui->posY->setValue(object->position().y);
    ui->posZ->setValue(object->position().z);
}

void MoveModule::on_posX_valueChanged(double arg1) {
    auto pos = object->position();
    pos.x = arg1;
    object->setPosition(pos);
}

void MoveModule::on_posY_valueChanged(double arg1) {
    auto pos = object->position();
    pos.y = arg1;
    object->setPosition(pos);
}

void MoveModule::on_posZ_valueChanged(double arg1) {
    auto pos = object->position();
    pos.z = arg1;
    object->setPosition(pos);
}