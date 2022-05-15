#ifndef PATCHMODULE_H
#define PATCHMODULE_H

#include <QWidget>
#include "Objects/Patch/patch.h"

namespace Ui {
    class PatchModule;
}

class PatchModule : public QWidget {
Q_OBJECT

public:
    PatchModule(std::shared_ptr<Patch> patch, QWidget *parent = nullptr);

    ~PatchModule();

private slots:

    void on_polygonalCheckBox_stateChanged(int arg1);

private:
    std::shared_ptr<Patch> patch;

    Ui::PatchModule *ui;
};

#endif // PATCHMODULE_H
