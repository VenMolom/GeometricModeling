#include "torusmodule.h"
#include "ui_torusmodule.h"

TorusModule::TorusModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TorusModule)
{
    ui->setupUi(this);
}

TorusModule::~TorusModule()
{
    delete ui;
}
