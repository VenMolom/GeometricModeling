#ifndef CNCROUTERMODULE_H
#define CNCROUTERMODULE_H

#include <QWidget>

namespace Ui {
class CNCRouterModule;
}

class CNCRouterModule : public QWidget
{
    Q_OBJECT

public:
    explicit CNCRouterModule(QWidget *parent = nullptr);
    ~CNCRouterModule();

private slots:
    void on_sizeX_valueChanged(double arg1);

    void on_sizeY_valueChanged(double arg1);

    void on_sizeZ_valueChanged(double arg1);

    void on_pointsDensityX_valueChanged(int arg1);

    void on_pointsDensityY_valueChanged(int arg1);

    void on_maxDepth_valueChanged(double arg1);

    void on_toolType_currentIndexChanged(int index);

    void on_toolSize_valueChanged(int arg1);

    void on_loadFileButton_clicked();

    void on_startButton_clicked();

    void on_skipButton_clicked();

    void on_speed_valueChanged(int value);

    void on_resetButton_clicked();

private:
    Ui::CNCRouterModule *ui;
};

#endif // CNCROUTERMODULE_H
