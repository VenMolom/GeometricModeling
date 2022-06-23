#ifndef INTERSECTIONTEXTUREDIALOG_H
#define INTERSECTIONTEXTUREDIALOG_H

#include <QDialog>

namespace Ui {
    class IntersectionTextureDialog;
}

class IntersectionTextureDialog : public QDialog {
Q_OBJECT

public:
    explicit IntersectionTextureDialog(QWidget *parent = nullptr);

    ~IntersectionTextureDialog();

private:
    Ui::IntersectionTextureDialog *ui;
};

#endif // INTERSECTIONTEXTUREDIALOG_H
