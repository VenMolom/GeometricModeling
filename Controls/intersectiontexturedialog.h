#ifndef INTERSECTIONTEXTUREDIALOG_H
#define INTERSECTIONTEXTUREDIALOG_H

#include <QDialog>
#include "Objects/Intersection/intersection.h"

namespace Ui {
    class IntersectionTextureDialog;
}

class IntersectionTextureDialog : public QDialog {
Q_OBJECT

public:
    explicit IntersectionTextureDialog(std::shared_ptr<Intersection> intersection, QWidget *parent = nullptr);

    ~IntersectionTextureDialog();

private:
    Ui::IntersectionTextureDialog *ui;
};

#endif // INTERSECTIONTEXTUREDIALOG_H
