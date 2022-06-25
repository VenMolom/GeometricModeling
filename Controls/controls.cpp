#include "controls.h"
#include "ui_controls.h"

#include "Objects/Parametric/torus.h"
#include "Objects/Cursor/cursor.h"
#include "Objects/Curve/brezierC2.h"

#include "Controls/Modules/MoveModule/movemodule.h"
#include "Controls/Modules/PatchModule/patchmodule.h"
#include "Controls/Modules/TorusModule/torusmodule.h"
#include "Controls/Modules/CurveModule/curvemodule.h"
#include "Controls/Modules/ObjectModule/objectmodule.h"
#include "Controls/Modules/BSplineModule/bsplinemodule.h"
#include "Controls/Modules/TransformModule/transformmodule.h"
#include "Controls/Modules/ScreenMoveModule/screenmovemodule.h"
#include "Controls/Modules/ParametricModule/parametricmodule.h"
#include "Controls/Modules/PatchCreatorModule/patchcreatormodule.h"
#include "Controls/Modules/IntersectionModule/intersectionmodule.h"
#include "Controls/Modules/IntersectionInstanceModule/intersectioninstancemodule.h"

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
    object = scene->selected();
    auto objectLocked = scene->selected().lock();

    modules.clear();
    if (!objectLocked) return;

    if (objectLocked->type() & NAMEABLE) {
        modules.push_back(std::move(make_unique<ObjectModule>(objectLocked, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 0, 0, 1, 1);
    }

    if (objectLocked->type() & MOVABLE) {
        modules.push_back(std::move(make_unique<MoveModule>(objectLocked, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 1, 0, 1, 1);
    }

    if (objectLocked->type() & SCREENMOVABLE) {
        // TODO: ?? best add some screenMove base class
        auto cursor = static_pointer_cast<Cursor>(objectLocked);
        modules.push_back(std::move(make_unique<ScreenMoveModule>(cursor, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 2, 0, 1, 1);
    }

    if (objectLocked->type() & TRANSFORMABLE) {
        modules.push_back(std::move(make_unique<TransformModule>(objectLocked, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 3, 0, 1, 1);
    }

    if (objectLocked->type() & PARAMETRIC) {
        auto parametric = static_pointer_cast<ParametricObject<2>>(objectLocked);
        modules.push_back(std::move(make_unique<ParametricModule>(parametric, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 4, 0, 1, 1);
    }

    if (objectLocked->type() & TORUS) {
        auto torus = static_pointer_cast<Torus>(objectLocked);
        modules.push_back(std::move(make_unique<TorusModule>(torus, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 5, 0, 1, 1);
    }

    if (objectLocked->type() & BREZIERC2) {
        auto curve = static_pointer_cast<BrezierC2>(objectLocked);
        modules.push_back(std::move(make_unique<BSplineModule>(curve, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 6, 0, 1, 1);
    }

    if (objectLocked->type() & CURVE) {
        auto curve = static_pointer_cast<Curve>(objectLocked);
        modules.push_back(std::move(make_unique<CurveModule>(curve, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 7, 0, 1, 1);
    }

    if (objectLocked->type() & PATCH) {
        auto patch = static_pointer_cast<Patch>(objectLocked);
        modules.push_back(std::move(make_unique<PatchModule>(patch, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 8, 0, 1, 1);
    }

    if (objectLocked->type() & PATCHCREATOR) {
        auto creator = dynamic_pointer_cast<PatchCreator>(objectLocked);
        modules.push_back(std::move(make_unique<PatchCreatorModule>(creator, scene, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 10, 0, 1, 1);
    }

    if (objectLocked->type() & PARAMETRIC) {
        auto parametric = static_pointer_cast<ParametricObject<2>>(objectLocked);
        if (parametric->intersectionInstance()) {
            auto module = make_unique<IntersectionInstanceModule>(parametric->intersectionInstance(), this);
            connect(module.get(), &IntersectionInstanceModule::removeIntersectionInstance,
                    this, &Controls::removeIntersection);
            modules.push_back(std::move(module));
            ui->modulesLayout->addWidget(modules.back().get(), 11, 0, 1, 1);
        }
    }

    if (objectLocked->type() & INTERSECTION) {
        auto intersection = dynamic_pointer_cast<Intersection>(objectLocked);
        modules.push_back(std::move(make_unique<IntersectionModule>(intersection, this)));
        ui->modulesLayout->addWidget(modules.back().get(), 12, 0, 1, 1);
    }
}

void Controls::removeIntersection() {
    auto objectLocked = object.lock();
    if (objectLocked && objectLocked->type() & PARAMETRIC) {
        auto parametric = static_pointer_cast<ParametricObject<2>>(objectLocked);
        parametric->setIntersectionInstance({});
        ui->modulesLayout->removeWidget(modules.back().get());
        modules.pop_back();
    }
}
