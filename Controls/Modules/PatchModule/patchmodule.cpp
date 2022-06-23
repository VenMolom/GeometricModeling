#include "patchmodule.h"
#include "ui_patchmodule.h"

using namespace std;

PatchModule::PatchModule(shared_ptr<Patch> patch, QWidget *parent) :
        QWidget(parent),
        patch(std::move(patch)),
        ui(new Ui::PatchModule) {
    ui->setupUi(this);
    ui->polygonalCheckBox->setCheckState(this->patch->polygonal() ? Qt::Checked : Qt::Unchecked);
    ui->pointsCheckBox->setCheckState(this->patch->showPoints() ? Qt::Checked : Qt::Unchecked);
}

PatchModule::~PatchModule() {
    delete ui;
}

void PatchModule::on_polygonalCheckBox_stateChanged(int arg1) {
    patch->setPolygonal(Qt::Checked == (Qt::CheckState) arg1);
}


void PatchModule::on_pointsCheckBox_stateChanged(int arg1) {
    patch->setShowPoints(Qt::Checked == (Qt::CheckState) arg1);
}