//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_SCENE_H
#define MG1_SCENE_H

#include <list>
#include <QObject>
#include <DirectXMath.h>
#include "Objects/Cursor/cursor.h"
#include "Objects/Point/point.h"
#include "Objects/Brezier/brezierC0.h"
#include "Objects/CompositeObject/compositeObject.h"
#include "Objects/Point/virtualPointsHolder.h"
#include "Objects/Grid/grid.h"

class Scene : public QObject {
    Q_OBJECT

public:
    explicit Scene();

    void addObject(std::shared_ptr<Object> &&object, bool overrideCursor = false);

    void addComposite(std::list<std::shared_ptr<Object>>&& objects);

    void removeSelected();

    void moveSelected(QPoint screenPosition);

    void addPoint(QPoint screenPosition);

    void selectOrAddCursor(QPoint screenPosition, bool multiple);

    void centerSelected();

    void draw(Renderer &renderer);

    Camera &camera() { return _camera; }

    std::weak_ptr<Object> selected() const { return _selected; }

    void setSelected (std::shared_ptr<Object> object);

    QBindable<std::weak_ptr<Object>> bindableSelected() { return &_selected; }

signals:
    void objectAdded(const std::shared_ptr<Object>& object, bool select);

private:
    QProperty<std::weak_ptr<Object>> _selected;
    std::list<std::shared_ptr<Object>> _objects;
    std::shared_ptr<Cursor> cursor;
    std::shared_ptr<Object> composite;
    std::list<std::weak_ptr<VirtualPointsHolder>> virtualPointsHolders;
    // TODO: look into removing composite to reduce complexity
    Camera _camera;
    Grid grid;

    Utils3D::XMFLOAT3RAY getRayFromScreenPosition(DirectX::XMINT2 screenPosition) const;

    std::shared_ptr<Object> findIntersectingObject(Utils3D::XMFLOAT3RAY ray);

    DirectX::XMFLOAT3 getPositionOnPlane(DirectX::XMINT2 screenPosition,
                                         DirectX::XMFLOAT3 normal, DirectX::XMFLOAT3 point) const;

    void addCursor(Utils3D::XMFLOAT3RAY ray, DirectX::XMINT2 screenPos);

    void removeComposite();

    void updateVirtualPointsHolders();
};


#endif //MG1_SCENE_H
