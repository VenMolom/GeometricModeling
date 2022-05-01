#include "scenecontrols.h"
#include "ui_scenecontrols.h"
#include <QColorDialog>

using namespace std;

SceneControls::SceneControls(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SceneControls) {
    ui->setupUi(this);

    setColor(ui->leftEyeColor, leftColor);
    setColor(ui->rightEyeColor, rightColor);
}

SceneControls::~SceneControls() {
    delete ui;
    renderer.release();
}

void SceneControls::setScene(shared_ptr<Scene> scenePtr) {
    scene = std::move(scenePtr);

    scene->camera()->setEyesDistance(ui->eyesDistance->value());
    scene->camera()->setFocusDistance(ui->focusDistance->value());
}

void SceneControls::setRenderer(unique_ptr<StereoscopicRenderer> rendererPtr) {
    renderer = std::move(rendererPtr);

    renderer->setLeftEyeColor({leftColor.redF(), leftColor.greenF(), leftColor.blueF()});
    renderer->setRightEyeColor({rightColor.redF(), rightColor.greenF(), rightColor.blueF()});
}

void SceneControls::on_stereoscopicCheckBox_stateChanged(int arg1) {
    renderer->enableStereoscopy(Qt::Checked == (Qt::CheckState) arg1);
}


void SceneControls::on_eyesDistance_valueChanged(double arg1) {
    scene->camera()->setEyesDistance(arg1);
}


void SceneControls::on_focusDistance_valueChanged(double arg1) {
    scene->camera()->setFocusDistance(arg1);
}


void SceneControls::on_leftEyeColor_clicked() {
    auto color = QColorDialog::getColor(leftColor, this);
    if (!color.isValid()) return;

    leftColor = color;
    setColor(ui->leftEyeColor, leftColor);
    renderer->setLeftEyeColor({color.redF(), color.greenF(), color.blueF()});
}


void SceneControls::on_rightEyeColor_clicked() {
    auto color = QColorDialog::getColor(rightColor, this);
    if (!color.isValid()) return;

    rightColor = color;
    setColor(ui->rightEyeColor, rightColor);
    renderer->setRightEyeColor({color.redF(), color.greenF(), color.blueF()});
}

void SceneControls::setColor(QPushButton *item, const QColor &color) {
    auto qss = QString("background-color: %1").arg(color.name());
    item->setStyleSheet(qss);
}

