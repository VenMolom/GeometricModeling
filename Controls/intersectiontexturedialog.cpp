#include "intersectiontexturedialog.h"
#include "ui_intersectiontexturedialog.h"

IntersectionTextureDialog::IntersectionTextureDialog(std::shared_ptr<Intersection> intersection, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::IntersectionTextureDialog) {
    ui->setupUi(this);

    ui->firstTexture->setPixmap(intersection->firstInstance()->pixmap());
    ui->secondTexture->setPixmap(intersection->secondInstance()->pixmap());

    setWindowTitle("Intersection Parameter Space");
}

IntersectionTextureDialog::~IntersectionTextureDialog() {
    delete ui;
}
