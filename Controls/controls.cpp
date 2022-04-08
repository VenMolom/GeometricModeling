#include "controls.h"
#include "ui_controls.h"

#include "Objects/Parametric/torus.h"
#include "Objects/Cursor/cursor.h"
#include "Objects/Brezier/brezierC2.h"

#include "Controls/Modules/MoveModule/movemodule.h"
#include "Controls/Modules/TorusModule/torusmodule.h"
#include "Controls/Modules/ObjectModule/objectmodule.h"
#include "Controls/Modules/BSplineModule/bsplinemodule.h"
#include "Controls/Modules/TransformModule/transformmodule.h"
#include "Controls/Modules/ScreenMoveModule/screenmovemodule.h"
#include "Controls/Modules/ParametricModule/parametricmodule.h"
#include "Controls/Modules/BrezierCurveModule/breziercurvemodule.h"

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

    if (object->type() & MOVEABLE) {
        modules.push_back(std::move(make_unique<MoveModule>(object, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 1, 0, 1, 1);
    }

    if (object->type() & SCREENMOVEABLE) {
        // TODO: best add some screenMove base class
        auto cursor = static_pointer_cast<Cursor>(object);
        modules.push_back(std::move(make_unique<ScreenMoveModule>(cursor, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 2, 0, 1, 1);
    }

    if (object->type() & TRANSFORMABLE) {
        modules.push_back(std::move(make_unique<TransformModule>(object, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 3, 0, 1, 1);
    }

    if (object->type() & TORUS) {
        auto torus = static_pointer_cast<Torus>(object);
        modules.push_back(std::move(make_unique<ParametricModule>(torus, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 4, 0, 1, 1);

        modules.push_back(std::move(make_unique<TorusModule>(torus, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 5, 0, 1, 1);
    }

    if (object->type() & BREZIERC2) {
        auto curve = static_pointer_cast<BrezierC2>(object);
        modules.push_back(std::move(make_unique<BSplineModule>(curve, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 6, 0, 1, 1);
    }

    if (object->type() & BREZIERCURVE) {
        auto curve = static_pointer_cast<BrezierCurve>(object);
        modules.push_back(std::move(make_unique<BrezierCurveModule>(curve, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 7, 0, 1, 1);
    }
}
