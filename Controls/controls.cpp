#include "controls.h"
#include "ui_controls.h"

#include "Objects/Parametric/torus.h"
#include "Objects/Cursor/cursor.h"
#include "Objects/Curve/brezierC2.h"

#include "Controls/Modules/MoveModule/movemodule.h"
#include "Controls/Modules/TorusModule/torusmodule.h"
#include "Controls/Modules/CurveModule/curvemodule.h"
#include "Controls/Modules/ObjectModule/objectmodule.h"
#include "Controls/Modules/BSplineModule/bsplinemodule.h"
#include "Controls/Modules/TransformModule/transformmodule.h"
#include "Controls/Modules/ScreenMoveModule/screenmovemodule.h"
#include "Controls/Modules/ParametricModule/parametricmodule.h"
#include "Controls/Modules/PatchCreatorModule/patchcreatormodule.h"

using namespace std;
using namespace DirectX;

Controls::Controls(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Controls) {
    ui->setupUi(this);
}

Controls::~Controls() {
    delete ui;
}

void Controls::setScene(shared_ptr<Scene> scenePtr) {
    scene = std::move(scenePtr);
    sceneHandler = scene->bindableSelected().addNotifier([this] { updateSelected(); });
    updateSelected();
}

void Controls::updateSelected() {
    auto object = scene->selected().lock();

    modules.clear();
    if (!object) return;

    if (object->type() & NAMEABLE) {
        modules.push_back(std::move(make_unique<ObjectModule>(object, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 0, 0, 1, 1);
    }

    if (object->type() & MOVABLE) {
        modules.push_back(std::move(make_unique<MoveModule>(object, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 1, 0, 1, 1);
    }

    if (object->type() & SCREENMOVABLE) {
        // TODO: ?? best add some screenMove base class
        auto cursor = static_pointer_cast<Cursor>(object);
        modules.push_back(std::move(make_unique<ScreenMoveModule>(cursor, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 2, 0, 1, 1);
    }

    if (object->type() & TRANSFORMABLE) {
        modules.push_back(std::move(make_unique<TransformModule>(object, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 3, 0, 1, 1);
    }

    if (object->type() & PARAMETRIC) {
        auto parametric = static_pointer_cast<ParametricObject<2>>(object);
        modules.push_back(std::move(make_unique<ParametricModule>(parametric, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 4, 0, 1, 1);
    }

    if (object->type() & TORUS) {
        auto torus = static_pointer_cast<Torus>(object);
        modules.push_back(std::move(make_unique<TorusModule>(torus, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 5, 0, 1, 1);
    }

    if (object->type() & BREZIERC2) {
        auto curve = static_pointer_cast<BrezierC2>(object);
        modules.push_back(std::move(make_unique<BSplineModule>(curve, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 6, 0, 1, 1);
    }

    if (object->type() & CURVE) {
        auto curve = static_pointer_cast<Curve>(object);
        modules.push_back(std::move(make_unique<CurveModule>(curve, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 7, 0, 1, 1);
    }

    if (object->type() & PATCHCREATOR) {
        auto creator = dynamic_pointer_cast<PatchCreator>(object);
        modules.push_back(std::move(make_unique<PatchCreatorModule>(creator, scene, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 8, 0, 1, 1);
    }
}
