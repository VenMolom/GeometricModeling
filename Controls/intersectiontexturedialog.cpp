#include "intersectiontexturedialog.h"
#include "ui_intersectiontexturedialog.h"

IntersectionTextureDialog::IntersectionTextureDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::IntersectionTextureDialog) {
    ui->setupUi(this);

    // TODO: process cpuTex and save it to array then create QImage/QPixmap from that
}

IntersectionTextureDialog::~IntersectionTextureDialog() {
    delete ui;
}
