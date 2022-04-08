#ifndef OBJECTMODULE_H
#define OBJECTMODULE_H

#include <QWidget>
#include "Objects/object.h"

namespace Ui {
class ObjectModule;
}

class ObjectModule : public QWidget {
    Q_OBJECT

public:
    explicit ObjectModule(std::shared_ptr<Object> object, QWidget *parent = nullptr);
    ~ObjectModule();

private slots:
    void on_nameEdit_editingFinished();

private:
    std::shared_ptr<Object> object;
    QPropertyNotifier nameHandler;

    Ui::ObjectModule *ui;
};

#endif // OBJECTMODULE_H
