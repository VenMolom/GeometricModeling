//
// Created by Molom on 2022-05-13.
//

#ifndef MG1_PATCH_H
#define MG1_PATCH_H

#include <Models/Surfaces/BezierSurfaceC0.h>
#include <Models/Surfaces/BezierSurfaceC2.h>
#include "Objects/Parametric/parametricObject.h"
#include "Objects/Point/virtualPointsHolder.h"
#include "Objects/linelist.h"

#define PATCH_DIM 2

class Patch : public ParametricObject<PATCH_DIM>, public VirtualPointsHolder {
public:
    Patch(uint id, QString name, DirectX::XMFLOAT3 position,  std::array<int, PATCH_DIM> density,
          std::array<int, PATCH_DIM> segments, bool cylinder, QBindable<std::weak_ptr<Object>> bindableSelected);

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
    std::array<int, PATCH_DIM> segments;
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

    template <typename T>
    T serialize(std::vector<MG1::Point> &serializedPoints);

    template <typename T>
    std::vector<T> serializePatches();

private:
    std::vector<QPropertyNotifier> pointsHandlers{};
    std::vector<DirectX::XMFLOAT3> startingPositions{};
    DirectX::XMFLOAT3 startingPosition;
    DirectX::XMFLOAT4X4 modificationMatrixInverse;
    Linelist bezierMesh;
    bool _polygonal{false};
};

template<typename T>
std::vector<T> Patch::serializePatches() {
    static_assert(std::is_base_of_v<MG1::BezierPatch, T>, "T must derive from MG1::BezierPatch");

    MG1::BezierPatch p{};
    std::vector<T> patches{};
    patches.emplace_back(std::move(p));

    return patches;
}

template<typename T>
T Patch::serialize(std::vector<MG1::Point> &serializedPoints) {
    static_assert(std::is_same_v<MG1::BezierSurfaceC0, T> || std::is_same_v<MG1::BezierSurfaceC2, T>,
                  "Must be BezierSurface");

    T surface{};
    auto object = static_cast<MG1::SceneObject*>(&surface);
    object->name = name().toStdString();
    object->SetId(id());

    if constexpr(std::is_same_v<MG1::BezierSurfaceC0, T>) {
        auto surfC0 = static_cast<MG1::BezierSurfaceC0*>(&surface);
        surfC0->patches = serializePatches<MG1::BezierPatchC0>();
    } else if constexpr(std::is_same_v<MG1::BezierSurfaceC2, T>) {
        auto surfC2 = static_cast<MG1::BezierSurfaceC2*>(&surface);
        surfC2->patches = serializePatches<MG1::BezierPatchC2>();
    }

    return surface;
}

#endif //MG1_PATCH_H
