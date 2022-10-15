#include <QFileDialog>
#include "cncroutermodule.h"
#include "ui_cncroutermodule.h"

using namespace std;

CNCRouterModule::CNCRouterModule(shared_ptr<CNCRouter> router, QWidget *parent) :
        QWidget(parent),
        router(std::move(router)),
        ui(new Ui::CNCRouterModule) {
    ui->setupUi(this);
    stateHandler = this->router->bindableState().addNotifier([this] { updateState(); });
    progressHandler = this->router->bindableProgress().addNotifier([this] { updateProgress(); });
    updateState();
    updateProgress();

    auto size = this->router->size();
    ui->sizeX->setValue(size.x);
    ui->sizeY->setValue(size.y);
    ui->sizeZ->setValue(size.z);

    auto density = this->router->pointsDensity();
    ui->pointsDensityX->setValue(density.first);
    ui->pointsDensityY->setValue(density.second);

    ui->maxDepth->setValue(this->router->maxDepth());

    ui->toolType->setCurrentIndex((int)this->router->toolType());
    ui->toolSize->setValue(this->router->toolSize());

    ui->showPaths->setChecked(this->router->showPaths());
    ui->speed->setValue(this->router->simulationSpeed());
}

CNCRouterModule::~CNCRouterModule() {
    delete ui;
}

void CNCRouterModule::on_sizeX_valueChanged(double arg1) {
    auto size = router->size();
    size.x = arg1;
    router->setSize(size);
}


void CNCRouterModule::on_sizeY_valueChanged(double arg1) {
    auto size = router->size();
    size.y = arg1;
    router->setSize(size);
}


void CNCRouterModule::on_sizeZ_valueChanged(double arg1) {
    auto size = router->size();
    size.z = arg1;
    router->setSize(size);
}


void CNCRouterModule::on_pointsDensityX_valueChanged(int arg1) {
    auto density = router->pointsDensity();
    density.first = arg1;
    router->setPointsDensity(density);
}


void CNCRouterModule::on_pointsDensityY_valueChanged(int arg1) {
    auto density = router->pointsDensity();
    density.second = arg1;
    router->setPointsDensity(density);
}


void CNCRouterModule::on_maxDepth_valueChanged(double arg1) {
    router->setMaxDepth(arg1);
}


void CNCRouterModule::on_toolType_currentIndexChanged(int index) {
    router->setToolType((CNCType) index);
}


void CNCRouterModule::on_toolSize_valueChanged(int arg1) {
    router->setToolSize(arg1);
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
}

void CNCRouterModule::on_showPaths_stateChanged(int arg1) {
    router->setShowPaths(Qt::Checked == (Qt::CheckState) arg1);
}

void CNCRouterModule::on_speed_valueChanged(int value) {
    router->setSimulationSpeed(value);
}

void CNCRouterModule::on_startButton_clicked() {
    router->start();
}

void CNCRouterModule::on_skipButton_clicked() {
    router->skip();
}

void CNCRouterModule::on_resetButton_clicked() {
    router->reset();
}

void CNCRouterModule::updateState() {
    switch (router->state()) {
        case Created:
            ui->fileLoadedFrame->setEnabled(false);
            ui->materialGroupBox->setEnabled(true);
            ui->toolGroupBox->setEnabled(true);
            ui->loadFileButton->setEnabled(true);
            ui->startButton->setEnabled(true);
            ui->skipButton->setEnabled(true);
            ui->resetButton->setEnabled(true);
            ui->speed->setEnabled(true);
            break;
        case FirstPathLoaded:
            ui->filenameLabel->setText(router->filename());
            ui->fileLoadedFrame->setEnabled(true);
            ui->materialGroupBox->setEnabled(true);
            ui->toolGroupBox->setEnabled(true);
            ui->loadFileButton->setEnabled(true);
            ui->startButton->setEnabled(true);
            ui->skipButton->setEnabled(true);
            ui->resetButton->setEnabled(true);
            ui->speed->setEnabled(true);
            break;
        case NextPathLoaded:
            ui->filenameLabel->setText(router->filename());
            ui->fileLoadedFrame->setEnabled(true);
            ui->materialGroupBox->setEnabled(false);
            ui->toolGroupBox->setEnabled(true);
            ui->loadFileButton->setEnabled(true);
            ui->startButton->setEnabled(true);
            ui->skipButton->setEnabled(true);
            ui->resetButton->setEnabled(true);
            ui->speed->setEnabled(true);
            break;
        case Started:
            ui->filenameLabel->setText(router->filename());
            ui->fileLoadedFrame->setEnabled(true);
            ui->materialGroupBox->setEnabled(false);
            ui->toolGroupBox->setEnabled(false);
            ui->loadFileButton->setEnabled(false);
            ui->startButton->setEnabled(false);
            ui->skipButton->setEnabled(true);
            ui->resetButton->setEnabled(false);
            ui->speed->setEnabled(true);
            break;
        case Skipped:
            ui->filenameLabel->setText(router->filename());
            ui->fileLoadedFrame->setEnabled(true);
            ui->materialGroupBox->setEnabled(false);
            ui->toolGroupBox->setEnabled(false);
            ui->loadFileButton->setEnabled(false);
            ui->startButton->setEnabled(false);
            ui->skipButton->setEnabled(false);
            ui->resetButton->setEnabled(false);
            ui->speed->setEnabled(false);
            break;
        case Finished:
            ui->filenameLabel->setText(router->filename());
            ui->fileLoadedFrame->setEnabled(true);
            ui->materialGroupBox->setEnabled(false);
            ui->toolGroupBox->setEnabled(true);
            ui->loadFileButton->setEnabled(true);
            ui->startButton->setEnabled(false);
            ui->skipButton->setEnabled(false);
            ui->resetButton->setEnabled(true);
            ui->speed->setEnabled(false);
            break;
    }
}

void CNCRouterModule::updateProgress() {
    ui->progressBar->setValue(router->progress());
}