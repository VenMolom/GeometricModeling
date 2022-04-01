#include "breziercurvemodule.h"
#include "ui_breziercurvemodule.h"

BrezierCurveModule::BrezierCurveModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BrezierCurveModule)
{
    ui->setupUi(this);
}

BrezierCurveModule::~BrezierCurveModule()
{
    delete ui;
}
