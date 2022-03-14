#include "controls.h"
#include "ui_controls.h"
#include "Objects/Torus/torus.h"

using namespace std;

Controls::Controls(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Controls) {
    ui->setupUi(this);

    ui->torusGroupBox->hide();
    ui->parametersGroupBox->hide();
}

Controls::~Controls()
{
    delete ui;
}

void Controls::setScene(shared_ptr<Scene> scenePtr) {
    scene = std::move(scenePtr);

    object = scene->selectedObject().lock();
    if (object) {
        switch (object->type()) {
            case TORUS:
                dim = 2;
                ui->torusGroupBox->show();
                ui->parametersGroupBox->show();
                ui->uFrame->show();
                ui->vFrame->show();
        }
    }
}

void Controls::setDensity() const {
    if (auto* p = dynamic_cast<ParametricObject<2>*>(object.get())) {
        p->setDensity(parameters<2>());
    }
}

void Controls::on_uDensity_valueChanged(int arg1) {
    setDensity();
}


void Controls::on_vDensity_valueChanged(int arg1) {
    setDensity();
}

void Controls::on_majorRadius_valueChanged(double arg1) {
    if (auto* t = dynamic_cast<Torus*>(object.get())) {
        t->setMajorRadius(static_cast<float>(arg1));
    }
}


void Controls::on_minorRadius_valueChanged(double arg1) {
    if (auto* t = dynamic_cast<Torus*>(object.get())) {
        t->setMinorRadius(static_cast<float>(arg1));
    }
}