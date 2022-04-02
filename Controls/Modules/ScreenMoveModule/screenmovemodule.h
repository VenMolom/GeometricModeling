#ifndef SCREENMOVEMODULE_H
#define SCREENMOVEMODULE_H

#include <QWidget>
#include "Objects/Cursor/cursor.h"

namespace Ui {
class ScreenMoveModule;
}

class ScreenMoveModule : public QWidget {
    Q_OBJECT

public:
    explicit ScreenMoveModule(std::shared_ptr<Cursor> cursor, QWidget *parent = nullptr);
    ~ScreenMoveModule();

private slots:
    void on_screenPosX_valueChanged(int arg1);

    void on_screenPosY_valueChanged(int arg1);

private:
    std::shared_ptr<Cursor> cursor;
    QPropertyNotifier screenPosHandler;
    Ui::ScreenMoveModule *ui;

    void updateScreenPosition();
};

#endif // SCREENMOVEMODULE_H
