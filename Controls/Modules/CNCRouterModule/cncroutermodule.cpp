#include <QFileDialog>
#include "cncroutermodule.h"
#include "ui_cncroutermodule.h"

using namespace std;

CNCRouterModule::CNCRouterModule(shared_ptr<CNCRouter> router, QWidget *parent) :
        QWidget(parent),
        router(std::move(router)),
        ui(new Ui::CNCRouterModule) {
    ui->setupUi(this);
    // TODO: set fields based on info in router
}

CNCRouterModule::~CNCRouterModule() {
    delete ui;
}

void CNCRouterModule::on_sizeX_valueChanged(double arg1) {

}


void CNCRouterModule::on_sizeY_valueChanged(double arg1) {

}


void CNCRouterModule::on_sizeZ_valueChanged(double arg1) {

}


void CNCRouterModule::on_pointsDensityX_valueChanged(int arg1) {

}


void CNCRouterModule::on_pointsDensityY_valueChanged(int arg1) {

}


void CNCRouterModule::on_maxDepth_valueChanged(double arg1) {

}


void CNCRouterModule::on_toolType_currentIndexChanged(int index) {

}


void CNCRouterModule::on_toolSize_valueChanged(int arg1) {

}


void CNCRouterModule::on_loadFileButton_clicked() {
    QFileDialog dialog(this, "Select paths file");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setFilter(QDir::Files);
    dialog.setDirectory("../Paths");
    dialog.setNameFilter("Path (*.k* *.f*)");

    if (!dialog.exec()) return;

    auto path = filesystem::path(dialog.selectedFiles()[0].toStdString());
    router->loadPath(FileParser::parseCNCPath(path));

    ui->filenameLabel->setText(QString::fromStdString(path.filename().string()));
    ui->fileLoadedFrame->setEnabled(true);
}


void CNCRouterModule::on_startButton_clicked() {

}


void CNCRouterModule::on_skipButton_clicked() {

}


void CNCRouterModule::on_speed_valueChanged(int value) {

}


void CNCRouterModule::on_resetButton_clicked() {

}


void CNCRouterModule::on_showPaths_stateChanged(int arg1) {

}

