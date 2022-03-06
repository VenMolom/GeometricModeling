#include "controls.h"
#include "ui_controls.h"

Controls::Controls(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Controls) {
    ui->setupUi(this);
    segments = ui->renderingSegments->value();
}

Controls::~Controls()
{
    delete ui;
}

void Controls::setScene(std::shared_ptr<Scene> scenePtr) {
    scene = std::move(scenePtr);
}

void Controls::on_radiusX_valueChanged(double arg1)
{
    scene->getEllipsoid().setRadiusX(arg1);
    emit updated(segments);
}


void Controls::on_radiusY_valueChanged(double arg1)
{
    scene->getEllipsoid().setRadiusY(arg1);
    emit updated(segments);
}


void Controls::on_radiusZ_valueChanged(double arg1)
{
    scene->getEllipsoid().setRadiusZ(arg1);
    emit updated(segments);
}

void Controls::on_renderingSegments_valueChanged(int arg1)
{
    segments = arg1;
    emit updated(segments);
}


void Controls::on_specular_valueChanged(int arg1)
{
    scene->getEllipsoid().setSpecular(arg1);
    emit updated(segments);
}

QVector3D Controls::getRadius() {
    return QVector3D(ui->radiusX->value(),ui->radiusY->value(),ui->radiusZ->value());
}
