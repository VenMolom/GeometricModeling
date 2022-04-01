#ifndef MOVEMODULE_H
#define MOVEMODULE_H

#include <QWidget>

namespace Ui {
class MoveModule;
}

class MoveModule : public QWidget
{
    Q_OBJECT

public:
    explicit MoveModule(QWidget *parent = nullptr);
    ~MoveModule();

private:
    Ui::MoveModule *ui;
};

#endif // MOVEMODULE_H
