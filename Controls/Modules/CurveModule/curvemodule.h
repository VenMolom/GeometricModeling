#ifndef CURVEMODULE_H
#define CURVEMODULE_H

#include <QWidget>
#include <QPushButton>
#include <QTableWidgetItem>
#include "Objects/Curve/curve.h"

namespace Ui {
class CurveModule;
}

class CurveModule : public QWidget {
    Q_OBJECT

public:
    explicit CurveModule(std::shared_ptr<Curve> curve, QWidget *parent = nullptr);
    ~CurveModule();

private slots:
    void on_polygonalCheckBox_stateChanged(int arg1);

    void on_pointsList_itemSelectionChanged();

    void on_movePointUp_clicked();

    void on_movePointDown_clicked();

    void onDeletePointButtonClicked();

private:
    std::shared_ptr<Curve> curve;
    std::list<std::unique_ptr<QTableWidgetItem>> points;
    std::list<std::unique_ptr<QPushButton>> deletePointButtons;
    QPropertyNotifier pointsHandler;

    Ui::CurveModule *ui;

    void updateCurvePoints();
};

#endif // CURVEMODULE_H
