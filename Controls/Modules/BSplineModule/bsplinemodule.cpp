#include "bsplinemodule.h"
#include "ui_bsplinemodule.h"

using namespace std;

BSplineModule::BSplineModule(shared_ptr<BrezierC2> curve, QWidget *parent) :
        QWidget(parent),
        curve(std::move(curve)),
        ui(new Ui::BSplineModule) {
    ui->setupUi(this);
    ui->bothPolygonalsCheckBox->setCheckState(this->curve->bothPolygonals() ? Qt::Checked : Qt::Unchecked);
    ui->bernsteinBaseCheckBox->setCheckState(this->curve->bernsteinBase() ? Qt::Checked : Qt::Unchecked);
}

BSplineModule::~BSplineModule() {
    delete ui;
}

void BSplineModule::on_bernsteinBaseCheckBox_stateChanged(int arg1) {
    curve->setBernsteinBase(Qt::Checked == (Qt::CheckState) arg1);
}


void BSplineModule::on_bothPolygonalsCheckBox_stateChanged(int arg1) {
    curve->setBothPolygonals(Qt::Checked == (Qt::CheckState) arg1);
}

