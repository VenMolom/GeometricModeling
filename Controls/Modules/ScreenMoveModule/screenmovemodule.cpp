#include "screenmovemodule.h"

#include <utility>
#include "ui_screenmovemodule.h"

using namespace std;

ScreenMoveModule::ScreenMoveModule(shared_ptr<Cursor> cursor, QWidget *parent) :
        QWidget(parent),
        cursor(std::move(cursor)),
        ui(new Ui::ScreenMoveModule) {
    ui->setupUi(this);
    screenPosHandler = this->cursor->bindableScreenPosition().addNotifier([&] { updateScreenPosition(); });
    updateScreenPosition();
}

ScreenMoveModule::~ScreenMoveModule() {
    delete ui;
}

void ScreenMoveModule::updateScreenPosition() {
    QSignalBlocker xBlocker(ui->screenPosX);
    QSignalBlocker yBlocker(ui->screenPosY);
    ui->screenPosX->setValue(cursor->screenPosition().x);
    ui->screenPosY->setValue(cursor->screenPosition().y);
}

void ScreenMoveModule::on_screenPosX_valueChanged(int arg1) {
    auto sp = cursor->screenPosition();
    sp.x = static_cast<int>(arg1);
    cursor->setScreenPosition(sp);
}

void ScreenMoveModule::on_screenPosY_valueChanged(int arg1) {
    auto sp = cursor->screenPosition();
    sp.y = static_cast<int>(arg1);
    cursor->setScreenPosition(sp);
}