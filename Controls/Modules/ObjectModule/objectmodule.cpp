#include "objectmodule.h"
#include "ui_objectmodule.h"

using namespace std;

ObjectModule::ObjectModule(shared_ptr<Object> object, QWidget *parent) :
    QWidget(parent),
    object(std::move(object)),
    ui(new Ui::ObjectModule)
{
    ui->setupUi(this);
    nameHandler = this->object->bindableName().addNotifier([&] { ui->nameEdit->setText(object->name()); });
    ui->nameEdit->setText(this->object->name());
}

ObjectModule::~ObjectModule()
{
    delete ui;
}

void ObjectModule::on_nameEdit_editingFinished() {
    object->setName(ui->nameEdit->text());
}