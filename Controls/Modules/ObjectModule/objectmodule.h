#ifndef OBJECTMODULE_H
#define OBJECTMODULE_H

#include <QWidget>

namespace Ui {
class ObjectModule;
}

class ObjectModule : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectModule(QWidget *parent = nullptr);
    ~ObjectModule();

private:
    Ui::ObjectModule *ui;
};

#endif // OBJECTMODULE_H
