#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndexList>
#include "scene.h"
#include "ObjectListItem/objectListItem.h"

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

    // TODO: think about:
    // * changing renderable to make possible rendering from multiple vertices lists with different topologies
    // * ?make object not inherit renderable (not all objects have to render, like compositeObject it only renders draws children) draw =/= render
    // * ?add new baseclass renderable object
    // * ?remove linestrip
    // * ?add method renderer.draw(RenderableObject renderableObject, ...)

    // TODO: add objects factory and IDs
};

#endif // MAINWINDOW_H
