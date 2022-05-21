//
// Created by Molom on 2022-05-13.
//

#ifndef MG1_PATCH_H
#define MG1_PATCH_H

#include "Objects/Parametric/parametricObject.h"
#include "Objects/Point/virtualPointsHolder.h"
#include "Objects/linelist.h"

#define PATCH_DIM 2

class Patch : public ParametricObject<PATCH_DIM>, public VirtualPointsHolder {
public:
    Patch(uint id, QString name, DirectX::XMFLOAT3 position, std::array<int, PATCH_DIM> density,
          bool cylinder, QBindable<std::weak_ptr<Object>> bindableSelected);

    std::array<bool, PATCH_DIM> looped() const override;

    const std::vector<std::shared_ptr<VirtualPoint>> &virtualPoints() override;

    void setPosition(DirectX::XMFLOAT3 position) override;

    void setRotation(DirectX::XMFLOAT3 rotation) override;

    void setScale(DirectX::XMFLOAT3 scale) override;

    boolean polygonal() const { return _polygonal; }

    void setPolygonal(bool draw) { _polygonal = draw; }

    void draw(Renderer &renderer, DrawType drawType) override;

protected:
    std::vector<std::shared_ptr<VirtualPoint>> points;
    bool cylinder;

    void densityUpdated() override {}

    void pointMoved(const std::weak_ptr<VirtualPoint> &point, int index);

    void addPoint(DirectX::XMFLOAT3 position);

    void updatePoints();

    void clear();

    void createSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size);

    virtual void drawMesh(Renderer &renderer, DrawType drawType);

    virtual void calculateMeshIndices(std::array<int, PATCH_DIM> segments, Linelist &linelist) = 0;

    virtual void createCylinderSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size) = 0;

    virtual void createPlaneSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size) = 0;

private:
    std::vector<QPropertyNotifier> pointsHandlers{};
    std::vector<DirectX::XMFLOAT3> startingPositions{};
    DirectX::XMFLOAT3 startingPosition;
    DirectX::XMFLOAT4X4 modificationMatrixInverse;
    Linelist bezierMesh;
    bool _polygonal{false};
};


#endif //MG1_PATCH_H
