#include "intersectdialog.h"
#include "ui_intersectdialog.h"

IntersectDialog::IntersectDialog(QWidget *parent, IntersectHandler &handler) :
        QDialog(parent),
        handler(handler),
        ui(new Ui::IntersectDialog) {
    ui->setupUi(this);

    if (!handler.canUseCursor()) {
        ui->useCursor->setEnabled(false);
    }

    setWindowTitle("Intersect");
}

IntersectDialog::~IntersectDialog() {
    delete ui;
}


void IntersectDialog::on_buttonBox_accepted() {
    handler.setMaxPoints(ui->maxPoints->value());
    handler.setIterations(ui->iterations->value());
    handler.setStep(static_cast<float>(ui->step->value()));
    handler.useCursor(ui->useCursor->isChecked());
}

