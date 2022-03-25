#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndexList>
#include "Scene/scene.h"
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

    void on_objectsList_itemSelectionChanged();

    void on_deleteObject_clicked();

    void on_centerObject_clicked();

    void onObjectAdded(const std::shared_ptr<Object>& object);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;
    std::shared_ptr<Scene> scene;
    std::list<std::unique_ptr<ObjectListItem>> items;

    QPropertyNotifier selectedHandler;

    void updateSelection();

    // IN PROGRESS:
    // TODO: add segmented brezier curve

    // TODO: add curve on selected points
    // TODO: add point(s) to curve
    // TODO: show curve on scene objects list
    // TODO: delete curve (control points persist)
    // TODO: select curve (all new points are added into it)
    // TODO: remove any point from curve
    // TODO: curve updates when points move
    // TODO: posibility to show curve polygon


    // TODO: look into reducing complexity / interlocking
    // TODO: add grid (if possible)
};

#endif // MAINWINDOW_H
