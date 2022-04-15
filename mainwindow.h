#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndexList>
#include "scene.h"
#include "Objects/ObjectListItem/objectListItem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:

    void on_addPoint_clicked();

    void on_addTorus_clicked();

    void on_addBrezierC0_clicked();

    void on_addBrezierC2_clicked();

    void on_addInterpolationC2_clicked();

    void on_objectsList_itemSelectionChanged();

    void on_deleteObject_clicked();

    void on_centerObject_clicked();

    void onObjectAdded(const std::shared_ptr<Object> &object, bool select);

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;
    std::shared_ptr<Scene> scene;
    std::list<std::unique_ptr<ObjectListItem>> items;

    QPropertyNotifier selectedHandler;

    void updateSelection();

    std::vector<std::weak_ptr<Point>> getSelectedPoints();

    /* Interpolation Curve C2
     * TODO: new Curve type
     * TODO: topology is 2_CONTROL_POINTS_PATCHLIST (c and distance to next node) (c_0 is 0)
     * TODO: tesselation shader calculating polynomial value from c and distance (from tau = 0 to d_i)
     * TODO: distance is chord length
     * TODO: solve equations in O(n) time and memory (moves will need to only recalculate up to 3 c) !This is hardest part
     */
};

#endif // MAINWINDOW_H
