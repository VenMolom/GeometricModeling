#include "patchcreatormodule.h"
#include "ui_patchcreatormodule.h"

using namespace std;

PatchCreatorModule::PatchCreatorModule(shared_ptr<PatchCreator> creator, shared_ptr<Scene> scene,
                                       QWidget *parent) :
        QWidget(parent),
        creator(std::move(creator)),
        scene(std::move(scene)),
        ui(new Ui::PatchCreatorModule) {
    ui->setupUi(this);
}

PatchCreatorModule::~PatchCreatorModule() {
    delete ui;
}

void PatchCreatorModule::on_cylinderCheckBox_stateChanged(int arg1) {
    creator->setCylinder(Qt::Checked == (Qt::CheckState) arg1);
}


void PatchCreatorModule::on_uSize_valueChanged(double arg1) {
    creator->setSize({static_cast<float>(ui->uSize->value()), static_cast<float>(ui->vSize->value())});
}


void PatchCreatorModule::on_vSize_valueChanged(double arg1) {
    creator->setSize({static_cast<float>(ui->uSize->value()), static_cast<float>(ui->vSize->value())});
}


void PatchCreatorModule::on_vSegments_valueChanged(int arg1) {
    creator->setSegments({ui->uSegments->value(), ui->vSegments->value()});
}


void PatchCreatorModule::on_uSegments_valueChanged(int arg1) {
    creator->setSegments({ui->uSegments->value(), ui->vSegments->value()});
}


void PatchCreatorModule::on_createButton_clicked() {
    scene->createFromCreator();
}

