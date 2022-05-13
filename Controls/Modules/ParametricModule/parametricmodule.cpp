#include "parametricmodule.h"
#include "ui_parametricmodule.h"

using namespace std;

ParametricModule::ParametricModule(std::shared_ptr<ParametricObject<2>> object, QWidget *parent) :
        QWidget(parent),
        object(std::move(object)),
        ui(new Ui::ParametricModule)
{
    ui->setupUi(this);
    ui->uDensity->setValue(this->object->density()[0]);
    ui->vDensity->setValue(this->object->density()[1]);
}

ParametricModule::~ParametricModule()
{
    delete ui;
}

void ParametricModule::on_uDensity_valueChanged(int arg1) {
    auto density = object->density();
    density[0] = arg1;
    object->setDensity(density);
}

void ParametricModule::on_vDensity_valueChanged(int arg1) {
    auto density = object->density();
    density[1] = arg1;
    object->setDensity(density);
}