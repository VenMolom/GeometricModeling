#include "parametricmodule.h"
#include "ui_parametricmodule.h"

ParametricModule::ParametricModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParametricModule)
{
    ui->setupUi(this);
}

ParametricModule::~ParametricModule()
{
    delete ui;
}
