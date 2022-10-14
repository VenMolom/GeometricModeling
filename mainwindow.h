#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndexList>
#include "scene.h"
#include "ObjectListItem/objectListItem.h"
#include <Scene/SceneSerializer.h>

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

    void on_addPatchC0_clicked();

    void on_addPatchC2_clicked();

    void on_objectsList_itemSelectionChanged();

    void on_deleteObject_clicked();

    void on_centerObject_clicked();

    void onObjectAdded(const std::shared_ptr<Object> &object, bool select);

    void on_actionSave_triggered();

    void on_actionLoad_triggered();

    void on_actionCollapse_Points_triggered();

    void on_actionFill_In_triggered();

    void on_actionIntersect_triggered();

    void on_addCNCRouter_clicked();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;
    std::shared_ptr<Scene> scene;
    std::list<std::unique_ptr<ObjectListItem>> items;

    QPropertyNotifier selectedHandler;

    void updateSelection();

    MG1::SceneSerializer serializer{};

    std::vector<std::weak_ptr<Point>> getSelectedPoints();
    // TODO: ? lock modifications to specific axis

    // TODO: CNCRouter:
    // klasa z metodą update, callowana przed renderem co klatkę
    // storuje wysokość punktów w teksturze
    // ruch freza to narysowanie do tekstury prostokątu zakończonego półokręgami
    // shader używa informacji czy jest to frez kolisty czy płaski do wyliczenia gradientu
    // ruch IRT używa frame time i prędkości freza do policzenia długości ruchu,
    //  trzeba obsłużyć ruch po więcej niż 1 ścieżce w jednym update'cie
    // ruch w tle obrabia 1 całą ścieżkę co update
    // postęp liczony na podstawie ilośći obrobionych ścieżek
};

#endif // MAINWINDOW_H
