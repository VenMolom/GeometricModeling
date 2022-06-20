#ifndef INTERSECTDIALOG_H
#define INTERSECTDIALOG_H

#include <QDialog>
#include "Handlers/intersectHandler.h"

namespace Ui {
    class IntersectDialog;
}

class IntersectDialog : public QDialog {
Q_OBJECT

public:
    explicit IntersectDialog(QWidget *parent, IntersectHandler &handler);

    ~IntersectDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::IntersectDialog *ui;

    IntersectHandler &handler;
};

#endif // INTERSECTDIALOG_H
