#include "intersectioninstancemodule.h"
#include "ui_intersectioninstancemodule.h"

using namespace std;

IntersectionInstanceModule::IntersectionInstanceModule(shared_ptr<IntersectionInstance> instance, QWidget *parent)
        : QWidget(parent),
          instance(std::move(instance)),
          ui(new Ui::IntersectionInstanceModule) {
    ui->setupUi(this);

    ui->firstCheckBox->setCheckState(this->instance->first() ? Qt::Checked : Qt::Unchecked);
    ui->activeCheckBox->setCheckState(this->instance->active() ? Qt::Checked : Qt::Unchecked);
}

IntersectionInstanceModule::~IntersectionInstanceModule() {
    delete ui;
}

void IntersectionInstanceModule::on_activeCheckBox_stateChanged(int arg1) {
    instance->setActive(Qt::Checked == (Qt::CheckState) arg1);
}


void IntersectionInstanceModule::on_firstCheckBox_stateChanged(int arg1) {
    instance->setFirst(Qt::Checked == (Qt::CheckState) arg1);
}


void IntersectionInstanceModule::on_removeButton_clicked() {
    emit removeIntersectionInstance();
}

