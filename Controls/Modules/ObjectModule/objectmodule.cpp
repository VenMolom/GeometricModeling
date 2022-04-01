#include "objectmodule.h"
#include "ui_objectmodule.h"

ObjectModule::ObjectModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ObjectModule)
{
    ui->setupUi(this);
}

ObjectModule::~ObjectModule()
{
    delete ui;
}
