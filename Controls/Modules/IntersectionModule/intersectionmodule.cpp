#include "intersectionmodule.h"

#include <utility>
#include "ui_intersectionmodule.h"
#include "Controls/intersectiontexturedialog.h"

IntersectionModule::IntersectionModule(std::shared_ptr<Intersection> intersection, QWidget *parent) :
        QWidget(parent),
        intersection(std::move(intersection)),
        ui(new Ui::IntersectionModule) {
    ui->setupUi(this);
}

IntersectionModule::~IntersectionModule() {
    delete ui;
}

void IntersectionModule::on_parameterSpaceButton_clicked() {
    dialog = make_unique<IntersectionTextureDialog>(intersection);

    dialog->show();
}


void IntersectionModule::on_convertToCurveButton_clicked() {
    //TODO: convert to curve
}

