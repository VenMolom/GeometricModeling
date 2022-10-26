#ifndef CNCROUTERMODULE_H
#define CNCROUTERMODULE_H

#include <QWidget>
#include "Objects/CNC/CNCRouter.h"

namespace Ui {
    class CNCRouterModule;
}

class CNCRouterModule : public QWidget {
Q_OBJECT

public:
    explicit CNCRouterModule(std::shared_ptr<CNCRouter> router, QWidget *parent = nullptr);

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

    void on_stopButton_clicked();

    void on_speed_valueChanged(int value);

    void on_resetButton_clicked();

    void on_showPaths_stateChanged(int arg1);

    void on_wireframe_stateChanged(int arg1);

    void on_workingHeight_valueChanged(int arg1);

private:
    std::shared_ptr<CNCRouter> router;
    QPropertyNotifier stateHandler, progressHandler;

    Ui::CNCRouterModule *ui;

    void updateState();

    void updateProgress();
};

#endif // CNCROUTERMODULE_H
