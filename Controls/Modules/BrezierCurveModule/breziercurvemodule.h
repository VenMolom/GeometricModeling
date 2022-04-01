#ifndef BREZIERCURVEMODULE_H
#define BREZIERCURVEMODULE_H

#include <QWidget>
#include <QPushButton>
#include <QTableWidgetItem>
#include "Objects/BrezierCurve/brezierCurve.h"

namespace Ui {
class BrezierCurveModule;
}

class BrezierCurveModule : public QWidget
{
    Q_OBJECT

public:
    explicit BrezierCurveModule(std::shared_ptr<BrezierCurve> curve, QWidget *parent = nullptr);
    ~BrezierCurveModule();

private slots:
    void on_polygonalCheckBox_stateChanged(int arg1);

    void on_pointsList_itemSelectionChanged();

    void on_movePointUp_clicked();

    void on_movePointDown_clicked();

    void onDeletePointButtonClicked();

private:
    std::shared_ptr<BrezierCurve> curve;
    std::list<std::unique_ptr<QTableWidgetItem>> points;
    std::list<std::unique_ptr<QPushButton>> deletePointButtons;
    QPropertyNotifier pointsHandler;

    Ui::BrezierCurveModule *ui;

    void updateCurvePoints();
};

#endif // BREZIERCURVEMODULE_H
