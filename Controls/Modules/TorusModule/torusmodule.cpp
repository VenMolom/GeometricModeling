#include "torusmodule.h"
#include "ui_torusmodule.h"

using namespace std;

TorusModule::TorusModule(shared_ptr<Torus> torus, QWidget *parent) :
    QWidget(parent),
    torus(std::move(torus)),
    ui(new Ui::TorusModule)
{
    ui->setupUi(this);
    ui->majorRadius->setValue(this->torus->majorRadius());
    ui->minorRadius->setValue(this->torus->minorRadius());
}

TorusModule::~TorusModule()
{
    delete ui;
}

void TorusModule::on_majorRadius_valueChanged(double arg1) {
    torus->setMajorRadius(static_cast<float>(arg1));
}


void TorusModule::on_minorRadius_valueChanged(double arg1) {
    torus->setMinorRadius(static_cast<float>(arg1));
}
