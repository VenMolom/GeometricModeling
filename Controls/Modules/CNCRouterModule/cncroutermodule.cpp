#include "cncroutermodule.h"
#include "ui_cncroutermodule.h"

CNCRouterModule::CNCRouterModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CNCRouterModule)
{
    ui->setupUi(this);
}

CNCRouterModule::~CNCRouterModule()
{
    delete ui;
}

void CNCRouterModule::on_sizeX_valueChanged(double arg1)
{

}


void CNCRouterModule::on_sizeY_valueChanged(double arg1)
{

}


void CNCRouterModule::on_sizeZ_valueChanged(double arg1)
{

}


void CNCRouterModule::on_pointsDensityX_valueChanged(int arg1)
{

}


void CNCRouterModule::on_pointsDensityY_valueChanged(int arg1)
{

}


void CNCRouterModule::on_maxDepth_valueChanged(double arg1)
{

}


void CNCRouterModule::on_toolType_currentIndexChanged(int index)
{

}


void CNCRouterModule::on_toolSize_valueChanged(int arg1)
{

}


void CNCRouterModule::on_loadFileButton_clicked()
{

}


void CNCRouterModule::on_startButton_clicked()
{

}


void CNCRouterModule::on_skipButton_clicked()
{

}


void CNCRouterModule::on_speed_valueChanged(int value)
{

}


void CNCRouterModule::on_resetButton_clicked()
{

}

