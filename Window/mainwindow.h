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

    void on_objectsList_itemSelectionChanged();

    void on_deleteObject_clicked();

private:
    Ui::MainWindow *ui;
    std::shared_ptr<Scene> scene;
    std::list<std::unique_ptr<ObjectListItem>> items;

    QPropertyNotifier selectedHandler;

    void addObjectToScene(std::shared_ptr<Object> &&object);

    void updateSelection();
};

#endif // MAINWINDOW_H
