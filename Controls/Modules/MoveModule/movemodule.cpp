#include "movemodule.h"
#include "ui_movemodule.h"

MoveModule::MoveModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MoveModule)
{
    ui->setupUi(this);
}

MoveModule::~MoveModule()
{
    delete ui;
}
