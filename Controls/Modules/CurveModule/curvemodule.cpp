#include "curvemodule.h"
#include "ui_curvemodule.h"

using namespace std;

CurveModule::CurveModule(shared_ptr<Curve> curve, QWidget *parent) :
        QWidget(parent),
        curve(std::move(curve)),
        ui(new Ui::CurveModule) {
    ui->setupUi(this);
    pointsHandler = this->curve->bindablePoints().addNotifier([this] { updateCurvePoints(); });
    updateCurvePoints();
    ui->polygonalCheckBox->setCheckState(this->curve->polygonal() ? Qt::Checked : Qt::Unchecked);
    ui->pointsList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

CurveModule::~CurveModule() {
    delete ui;
}

void CurveModule::updateCurvePoints() {
    points.clear();
    deletePointButtons.clear();
    ui->pointsList->clearContents();

    ui->pointsList->setRowCount(curve->points().size());
    for (auto &point: curve->points()) {
        if (auto pp = point.lock()) {
            auto item = make_unique<QTableWidgetItem>(pp->nameId());
            auto button = make_unique<QPushButton>("Delete");
            button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            int index = static_cast<int>(points.size());
            ui->pointsList->setItem(index, 0, item.get());
            ui->pointsList->setCellWidget(index, 1, button.get());

            connect(button.get(), &QPushButton::clicked, this, &CurveModule::onDeletePointButtonClicked);

            points.push_back(std::move(item));
            deletePointButtons.push_back(std::move(button));
        }
    }
    ui->pointsList->setRowCount(points.size());
}

void CurveModule::on_polygonalCheckBox_stateChanged(int arg1) {
    curve->setPolygonal(Qt::Checked == (Qt::CheckState) arg1);
}

void CurveModule::on_pointsList_itemSelectionChanged() {
    auto selected = ui->pointsList->selectedItems();
    auto disable = selected.empty();
    ui->movePointUp->setDisabled(disable);
    ui->movePointDown->setDisabled(disable);
}

void CurveModule::on_movePointUp_clicked() {
    auto selected = ui->pointsList->selectedItems();
    if (selected.empty()) return;

    curve->movePoint(selected[0]->row(), Direction::UP);
}

void CurveModule::on_movePointDown_clicked() {
    auto selected = ui->pointsList->selectedItems();
    if (selected.empty()) return;

    curve->movePoint(selected[0]->row(), Direction::DOWN);
}

void CurveModule::onDeletePointButtonClicked() {
    auto index = ui->pointsList->currentRow();
    curve->removePoint(index);
}