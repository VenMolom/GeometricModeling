#include "intersectionmodule.h"

#include <utility>
#include "ui_intersectionmodule.h"
#include "Controls/intersectiontexturedialog.h"

IntersectionModule::IntersectionModule(std::shared_ptr<Intersection> intersection, std::shared_ptr<Scene> scene,
                                       QWidget *parent) :
        QWidget(parent),
        intersection(std::move(intersection)),
        scene(std::move(scene)),
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
    scene->createInterpolationCurve(intersection->points(), intersection->closed());
}

