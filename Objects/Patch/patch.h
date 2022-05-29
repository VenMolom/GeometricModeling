//
// Created by Molom on 2022-05-13.
//

#ifndef MG1_PATCH_H
#define MG1_PATCH_H

#define NOMINMAX

#include <map>
#include <Models/Surfaces/BezierSurfaceC0.h>
#include <Models/Surfaces/BezierSurfaceC2.h>
#include "Objects/Parametric/parametricObject.h"
#include "Objects/Point/virtualPointsHolder.h"
#include "Objects/linelist.h"

#define PATCH_DIM 2

class Patch : public ParametricObject<PATCH_DIM>, public VirtualPointsHolder {
public:
    Patch(uint id, QString name, DirectX::XMFLOAT3 position, std::array<int, PATCH_DIM> density,
          std::array<int, PATCH_DIM> segments, bool cylinder, QBindable<std::weak_ptr<Object>> bindableSelected);

    Patch(const MG1::BezierSurfaceC0 &surface, std::vector<MG1::Point> &serializedPoints,
          QBindable<std::weak_ptr<Object>> bindableSelected);

    Patch(const MG1::BezierSurfaceC2 &surface, std::vector<MG1::Point> &serializedPoints,
          QBindable<std::weak_ptr<Object>> bindableSelected);

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
    bool loopedU = false, loopedV;
    Linelist bezierMesh;

    void densityUpdated() override {}

    void pointMoved(const std::weak_ptr<VirtualPoint> &point, int index);

    void addPoint(DirectX::XMFLOAT3 position);

    void updatePoints();

    void clear();

    void createSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size);

    void calculateCenter();

    virtual void drawMesh(Renderer &renderer, DrawType drawType);

    virtual void calculateMeshIndices(std::array<int, PATCH_DIM> segments, Linelist &linelist) = 0;

    virtual void createCylinderSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size) = 0;

    virtual void createPlaneSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size) = 0;

    template<typename T>
    T serialize(std::vector<MG1::Point> &serializedPoints);

private:
    std::vector<QPropertyNotifier> pointsHandlers{};
    std::vector<DirectX::XMFLOAT3> startingPositions{};
    DirectX::XMFLOAT3 startingPosition;
    DirectX::XMFLOAT4X4 modificationMatrixInverse;
    bool _polygonal{false};

    template<typename T>
    std::vector<T> serializePatches(std::vector<MG1::Point> &serializedPoints);

    template<typename T>
    MG1::BezierPatch serializePatch(int startIndex, int uSegment, int vSegment, int uPoints,
                                    const std::map<int, uint> &pointMap);

    template<typename T>
    void deserializePatches(const std::vector<T> &patches, std::vector<MG1::Point> &serializedPoints);

    void deserializePatch(const MG1::BezierPatch &patch, const std::map<uint, int> &pointMap);

    template<typename T>
    struct patchConfig {
        static const int pointsSegment = 1;
        static const int segmentModifier = 0;

        static bool wrap(int vSegment, int vSegments, int row) {
            return false;
        }
    };

    template<>
    struct patchConfig<MG1::BezierPatchC0> {
        static const int pointsSegment = 3;
        static const int segmentModifier = 1;

        static bool wrap(int vSegment, int vSegments, int row) {
            return vSegment == vSegments - 1 && row == 3;
        }
    };

    template<>
    struct patchConfig<MG1::BezierPatchC2> {
        static const int pointsSegment = 1;
        static const int segmentModifier = 3;

        static bool wrap(int vSegment, int vSegments, int row) {
            return vSegment == vSegments - row;
        }
    };

    template<typename T>
    struct patchFormat {
        using type = void;
    };

    template<>
    struct patchFormat<MG1::BezierSurfaceC0> {
        using type = MG1::BezierPatchC0;
    };

    template<>
    struct patchFormat<MG1::BezierSurfaceC2> {
        using type = MG1::BezierPatchC2;
    };
};

template<typename T>
T Patch::serialize(std::vector<MG1::Point> &serializedPoints) {
    static_assert(std::is_same_v<MG1::BezierSurfaceC0, T> || std::is_same_v<MG1::BezierSurfaceC2, T>,
                  "Must be BezierSurface");

    T surface{};
    auto object = static_cast<MG1::SceneObject *>(&surface);
    object->name = name().toStdString();
    object->SetId(id());

    if constexpr(std::is_same_v<MG1::BezierSurfaceC0, T>) {
        auto surfC0 = static_cast<MG1::BezierSurfaceC0 *>(&surface);
        surfC0->size = {static_cast<uint32_t>(segments[0]), static_cast<uint32_t>(segments[1])};
        surfC0->uWrapped = loopedU;
        surfC0->vWrapped = loopedV;
        surfC0->patches = serializePatches<patchFormat<T>::type>(serializedPoints);
    } else if constexpr(std::is_same_v<MG1::BezierSurfaceC2, T>) {
        auto surfC2 = static_cast<MG1::BezierSurfaceC2 *>(&surface);
        surfC2->size = {static_cast<uint32_t>(segments[0]), static_cast<uint32_t>(segments[1])};
        surfC2->uWrapped = loopedU;
        surfC2->vWrapped = loopedV;
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

    //make map of points id (index -> id)
    std::map<int, uint> pointMap;
    for (int i = 0; i < points.size(); ++i) {
        auto pos = points[i]->position();
        serializedPoints.emplace_back();
        serializedPoints.back().position = {pos.x, pos.y, pos.z};
        pointMap.insert(std::make_pair(i, serializedPoints.back().GetId()));
    }


    auto uPoints = segments[0] * pointsSegments + segmentsAdd;

    // u segments
    for (int i = 0; i < segments[0]; ++i) {
        // v segments
        for (int j = 0; j < segments[1]; ++j) {
            auto start = j * pointsSegments * uPoints + pointsSegments * i;
            patches.emplace_back(serializePatch<T>(start, i, j, uPoints, pointMap));
        }
    }

    return patches;
}

template<typename T>
MG1::BezierPatch Patch::serializePatch(int startIndex, int uSegment, int vSegment, int uPoints,
                                       const std::map<int, uint> &pointMap) {
    MG1::BezierPatch patch{};
    patch.SetId(id());
    patch.name = name().toStdString();
    patch.samples = {static_cast<uint32_t>(density()[0]), static_cast<uint32_t>(density()[1])};
    patch.controlPoints.resize(16, MG1::PointRef(0));

    static constexpr int pointsSegments = patchConfig<T>::pointsSegment;

    auto index = startIndex;
    // vRow
    for (int i = 0; i < 4; ++i) {
        if (loopedV && patchConfig<T>::wrap(vSegment, segments[1], i)) {
            index = pointsSegments * uSegment;
        }

        // uRow
        for (int j = 0; j < 4; ++j) {
            auto cpIndex = j * 4 + i;
            auto ref = pointMap.at(index + j);
            patch.controlPoints[cpIndex] = MG1::PointRef(ref);
        }
        index += uPoints;
    }

    return patch;
}

template<typename T>
void Patch::deserializePatches(const std::vector<T> &patches, std::vector<MG1::Point> &serializedPoints) {
    static_assert(std::is_base_of_v<MG1::BezierPatch, T>, "T must derive from MG1::BezierPatch");

    static constexpr int pointsSegments = patchConfig<T>::pointsSegment;
    static constexpr int segmentsAdd = patchConfig<T>::segmentModifier;

    auto uPoints = segments[0] * pointsSegments + segmentsAdd;
    auto vPoints = segments[1] * pointsSegments + segmentsAdd;

    //make map of points id (id -> index)
    std::map<uint, int> pointMap;
    // we go through all lines in v direction and u direction
    for (int i = 0; i < vPoints; ++i) {
        for (int j = 0; j < uPoints; ++j) {
            // calculate which patch this point belongs to
            int u = std::clamp(j / pointsSegments, 0, segments[0] - 1);
            int v = std::clamp(i / pointsSegments, 0, segments[1] - 1);
            int uu = j - pointsSegments * u, vv = i - pointsSegments * v;

            int patchIndex = segments[0] * u + v;
            int pointIndex = 4 * uu + vv;

            auto patch = static_cast<const MG1::BezierPatch *>(&patches[patchIndex]);
            auto pointRef = patch->controlPoints[pointIndex];

            // add to map if it's not in it and add to point list
            if (!pointMap.contains(pointRef.GetId())) {
                auto it = std::find_if(serializedPoints.begin(), serializedPoints.end(),
                                       [&pointRef](const MG1::Point &p) {
                                           return p.GetId() == pointRef.GetId();
                                       });

                if (it == serializedPoints.end()) {
                    continue;
                }

                MG1::Point point = *it;
                auto pos = point.position;
                addPoint({pos.x, pos.y, pos.z});

                pointMap.insert(std::make_pair(point.GetId(), points.size() - 1));

                serializedPoints.erase(it);
            }

        }
    }

    for (auto &patch: patches) {
        deserializePatch(patch, pointMap);
    }
}

#endif //MG1_PATCH_H
