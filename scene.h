//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_SCENE_H
#define MG1_SCENE_H

#include <list>
#include <QObject>
#include <DirectXMath.h>
#include <Scene/Scene.h>
#include "Objects/Cursor/cursor.h"
#include "Objects/CompositeObject/compositeObject.h"
#include "Objects/Point/virtualPointsHolder.h"
#include "Objects/Grid/grid.h"
#include "Objects/objectFactory.h"
#include "Objects/creator.h"
#include "Handlers/intersectHandler.h"

class Scene : public QObject {
    Q_OBJECT

public:
    explicit Scene();

    void addObject(std::shared_ptr<Object> &&object, bool overrideCursor = false);

    std::shared_ptr<CompositeObject> addComposite(std::list<std::shared_ptr<Object>> &&objects);

    void addCreator(std::shared_ptr<Object> &&object);

    void createFromCreator();

    void removeSelected();

    void collapseSelected();

    void fillIn();

    void intersect(IntersectHandler &handler, Renderer &renderer);

    void addPoint(QPoint screenPosition);

    void selectOrAddCursor(QPoint screenPosition, bool multiple);

    void centerSelected();

    void selectFromScreen(QPointF start, QPointF end);

    void draw(Renderer &renderer);

    void serialize(MG1::Scene &scene);

    void load(MG1::Scene &scene);

    std::shared_ptr<Camera> camera() { return _camera; }

    std::weak_ptr<Object> selected() const { return _selected; }

    ObjectFactory &objectFactory() { return factory; }

    void setSelected(std::shared_ptr<Object> object);

    QBindable<std::weak_ptr<Object>> bindableSelected() { return &_selected; }

    bool hasCursor() { return static_cast<bool>(cursor); }

signals:
    void objectAdded(const std::shared_ptr<Object>& object, bool select);

private:
    QProperty<std::weak_ptr<Object>> _selected;
    std::list<std::shared_ptr<Object>> _objects;
    std::shared_ptr<Cursor> cursor;
    std::shared_ptr<Object> composite;
    std::shared_ptr<Object> creator;
    std::list<std::weak_ptr<VirtualPointsHolder>> virtualPointsHolders;
    // TODO: ?? look into removing composite to reduce complexity
    std::shared_ptr<Camera> _camera;
    ObjectFactory factory;
    Grid grid;

    Utils3D::XMFLOAT3RAY getRayFromScreenPosition(DirectX::XMINT2 screenPosition) const;

    std::shared_ptr<Object> findIntersectingObject(Utils3D::XMFLOAT3RAY ray);

    DirectX::XMFLOAT3 getPositionOnPlane(DirectX::XMINT2 screenPosition,
                                         DirectX::XMFLOAT3 normal, DirectX::XMFLOAT3 point) const;

    void addCursor(Utils3D::XMFLOAT3RAY ray, DirectX::XMINT2 screenPos);

    void removeComposite();

    DirectX::XMFLOAT2 project(DirectX::XMFLOAT3 position);
};


#endif //MG1_SCENE_H
