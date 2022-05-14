#ifndef PATCHCREATORMODULE_H
#define PATCHCREATORMODULE_H

#include <QWidget>
#include "Objects/Patch/patchCreator.h"
#include "scene.h"

namespace Ui {
class PatchCreatorModule;
}

class PatchCreatorModule : public QWidget
{
    Q_OBJECT

public:
    explicit PatchCreatorModule(std::shared_ptr<PatchCreator> creator, std::shared_ptr<Scene> scene,
                                QWidget *parent = nullptr);
    ~PatchCreatorModule();

private slots:
    void on_cylinderCheckBox_stateChanged(int arg1);

    void on_uSize_valueChanged(double arg1);

    void on_vSize_valueChanged(double arg1);

    void on_vSegments_valueChanged(int arg1);

    void on_uSegments_valueChanged(int arg1);

    void on_createButton_clicked();

private:
    std::shared_ptr<PatchCreator> creator;
    std::shared_ptr<Scene> scene;

    Ui::PatchCreatorModule *ui;
};

#endif // PATCHCREATORMODULE_H
