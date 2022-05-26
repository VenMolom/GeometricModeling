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

private:
    std::vector<QPropertyNotifier> pointsHandlers{};
    std::vector<DirectX::XMFLOAT3> startingPositions{};
    DirectX::XMFLOAT3 startingPosition;
    DirectX::XMFLOAT4X4 modificationMatrixInverse;
    Linelist bezierMesh;
    bool _polygonal{false};

    template <typename T>
    std::vector<T> serializePatches(std::vector<MG1::Point> &serializedPoints);

    template <typename T>
    MG1::BezierPatch serializePatch(int startIndex, int uSegment, int vSegment, int uPoints,
                                    std::vector<MG1::Point> &serializedPoints);

    template<typename T>
    struct patchConfig{
        static const int pointsSegment = 1;
        static const int segmentModifier = 0;
        static bool wrap(int vSegment, int vSegments, int row) {
            return false;
        }
    };

    template<>
    struct patchConfig<MG1::BezierPatchC0>{
        static const int pointsSegment = 3;
        static const int segmentModifier = 1;
        static bool wrap(int vSegment, int vSegments, int row) {
            return vSegment == vSegments - 1 && row == 3;
        }
    };

    template<>
    struct patchConfig<MG1::BezierPatchC2>{
        static const int pointsSegment = 1;
        static const int segmentModifier = 3;
        static bool wrap(int vSegment, int vSegments, int row) {
            return vSegment == vSegments - row;
        }
    };

    template<typename T>
    struct patchFormat{
        using type = void;
    };

    template<>
    struct patchFormat<MG1::BezierSurfaceC0>{
        using type = MG1::BezierPatchC0;
    };

    template<>
    struct patchFormat<MG1::BezierSurfaceC2>{
        using type = MG1::BezierPatchC2;
    };
};

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
        surfC0->patches = serializePatches<patchFormat<T>::type>(serializedPoints);
    } else if constexpr(std::is_same_v<MG1::BezierSurfaceC2, T>) {
        auto surfC2 = static_cast<MG1::BezierSurfaceC2*>(&surface);
        surfC2->patches = serializePatches<patchFormat<T>::type>(serializedPoints);
    }

    return surface;
}

template<typename T>
std::vector<T> Patch::serializePatches(std::vector<MG1::Point> &serializedPoints) {
    static_assert(std::is_base_of_v<MG1::BezierPatch, T>, "T must derive from MG1::BezierPatch");

    std::vector<T> patches{};
    static constexpr int pointsSegments = patchConfig<T>::pointsSegment;
    static constexpr int segmentsAdd = patchConfig<T>::segmentModifier;

    auto uPoints = segments[0] * pointsSegments + segmentsAdd;

    // u segments
    for (int i = 0; i < segments[0]; ++i) {
        // v segments
        for (int j = 0; j < segments[1]; ++j) {
            auto start = j * pointsSegments * uPoints + pointsSegments * i;
            patches.emplace_back(serializePatch<T>(start, i, j, uPoints, serializedPoints));
        }
    }

    return patches;
}

template<typename T>
MG1::BezierPatch
Patch::serializePatch(int startIndex, int uSegment, int vSegment, int uPoints,
                      std::vector<MG1::Point> &serializedPoints) {
    MG1::BezierPatch patch{};
    patch.SetId(id());
    patch.name = name().toStdString();
    patch.samples = { static_cast<uint32_t>(density()[0]), static_cast<uint32_t>(density()[1]) };

    static constexpr int pointsSegments = patchConfig<T>::pointsSegment;

    auto index = startIndex;
    // vRow
    for (int i = 0; i < 4; ++i) {
        if (cylinder && patchConfig<T>::wrap(vSegment, segments[1], i)) {
            index = pointsSegments * uSegment;
        }

        // uRow
        for (int j = 0; j < 4; ++j) {
            // TODO: don't create new point if index was already referenced
            auto pos = points[index + j]->position();
            serializedPoints.emplace_back();
            serializedPoints.back().position = {pos.x, pos.y, pos.z};
            patch.controlPoints.emplace_back(serializedPoints.back().GetId());
        }
        index += uPoints;
    }

    return patch;
}

#endif //MG1_PATCH_H
