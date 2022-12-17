//
// Created by Molom on 14/11/2022.
//

#ifndef MG1_PATHSCREATORHELPER_H
#define MG1_PATHSCREATORHELPER_H

#include "Objects/objectFactory.h"
#include "Handlers/intersectHandler.h"

namespace PathsCreatorHelper {
    enum FlatteningSegment {
        HandleExternal,
        MainRight,
        MainLeft,
        Dziubek,
    };

    struct Textures {
        mini::dx_ptr<ID3D11ShaderResourceView> depthTexture;
        mini::dx_ptr<ID3D11DepthStencilView> depth;
        mini::dx_ptr<ID3D11UnorderedAccessView> allowedHeight;
        mini::dx_ptr<ID3D11Texture2D> allowedHeightStaging;
    };

    static constexpr int TEX_SIZE = 2048;

    static constexpr float BLOCK_SIZE_XY = 15.f;
    static constexpr float BLOCK_SIZE_Z = 5.f;
    static constexpr float BLOCK_BOTTOM = 1.5f;

    static constexpr float BLOCK_BOTTOM_LOCAL = BLOCK_BOTTOM * 10.f;
    static constexpr float BLOCK_END_LOCAL = BLOCK_SIZE_XY * 5.f;

    static constexpr float START_X = 88.f;
    static constexpr float START_Y = 88.f;
    static constexpr float START_Z = 66.f;

    void addPositionsOnLine(std::vector<DirectX::XMFLOAT3> &positions,
                            float *data, int dir,
                            float baseZ, float xSize,
                            float y, int texY);

    std::vector<DirectX::XMFLOAT3> createZigZagLines(const float *data, int toolSize);

    void zigZagLines(std::vector<DirectX::XMFLOAT3> &zigZag, const float *data, int yMoves, float lineDistance,
                     float &y, float &x, int &dir);

    std::vector<DirectX::XMFLOAT3>
    calculateToolDistantPath(ParametricObject<2> &patch, const std::vector<std::pair<float, float>> &parameters,
                             const std::vector<DirectX::XMFLOAT3> &points, int toolSize,
                             FlatteningSegment segment);

    void insertSlerpNormalPosition(std::vector<DirectX::XMFLOAT3> &path, DirectX::XMVECTOR position,
                                   DirectX::XMVECTOR normalStart, DirectX::XMVECTOR normalEnd, float normalLength);

    PathsCreatorHelper::Textures createDepthTextures(const DxDevice &device);

    void copyResource(const DxDevice &device, mini::dx_ptr<ID3D11UnorderedAccessView> &source,
                      mini::dx_ptr<ID3D11Texture2D> &target);

    std::vector<DirectX::XMFLOAT3> intersectAndCalculateToolDistant(IntersectHandler &intersect,
                                                                    Renderer &renderer,
                                                                    std::shared_ptr<ParametricObject<2>> patch,
                                                                    std::shared_ptr<ParametricObject<2>> object,
                                                                    const std::array<float, 4> &starting,
                                                                    FlatteningSegment segment,
                                                                    int toolSize);

    std::tuple<std::vector<DirectX::XMFLOAT3>::iterator, std::vector<DirectX::XMFLOAT3>::iterator, DirectX::XMFLOAT3>
    findIntersection(std::vector<DirectX::XMFLOAT3>::iterator path1, std::vector<DirectX::XMFLOAT3>::iterator path2);

    std::pair<std::vector<DirectX::XMFLOAT3>::const_iterator, DirectX::XMFLOAT3>
    findIntersection(std::vector<DirectX::XMFLOAT3>::const_iterator path1, DirectX::XMFLOAT3 start, DirectX::XMFLOAT3 end);

    std::vector<DirectX::XMFLOAT3>::iterator
    findIntersectionHeight(std::vector<DirectX::XMFLOAT3>::iterator path, float height);

    std::pair<std::pair<float, float>, size_t>
    findIntersection(const std::vector<std::pair<float, float>> &path, std::pair<float, float> start,
                     std::pair<float, float> end, bool mod = false);

    std::pair<std::pair<float, float>, size_t>
    findIntersectionEnd(const std::vector<std::pair<float, float>> &path, std::pair<float, float> start,
                        std::pair<float, float> end);

    std::vector<DirectX::XMFLOAT3> createHandlePath(const std::vector<std::pair<float, float>> &topRing,
                                                    const std::vector<std::pair<float, float>> &bottomRing,
                                                    const std::vector<std::pair<float, float>> &insideLine,
                                                    const std::vector<std::pair<float, float>> &outsideLine,
                                                    const std::shared_ptr<ParametricObject<2>> &distant);

    std::vector<DirectX::XMFLOAT3> createHandleContour(const std::vector<DirectX::XMFLOAT3> &topRing,
                                                       const std::vector<DirectX::XMFLOAT3> &bottomRing,
                                                       const std::vector<DirectX::XMFLOAT3> &insideLine,
                                                       const std::vector<DirectX::XMFLOAT3> &outsideLine);

    std::vector<DirectX::XMFLOAT3> createDziubekPath(const std::vector<std::pair<float, float>> &outline,
                                                     const std::vector<std::pair<float, float>> &mainRing,
                                                     const std::shared_ptr<ParametricObject<2>> &distant);

    std::vector<DirectX::XMFLOAT3> createDziubekContour(const std::vector<DirectX::XMFLOAT3> &outline,
                                                        const std::vector<DirectX::XMFLOAT3> &mainRing);

    std::vector<DirectX::XMFLOAT3> createMainPath(const std::vector<std::pair<float, float>> &outline,
                                                  const std::vector<std::pair<float, float>> &midRing,
                                                  const std::vector<std::pair<float, float>> &dziubekRing,
                                                  const std::vector<std::pair<float, float>> &topRing,
                                                  const std::vector<std::pair<float, float>> &bottomRing,
                                                  const std::shared_ptr<ParametricObject<2>> &distant);

    std::vector<DirectX::XMFLOAT3> createMainContour(const std::vector<DirectX::XMFLOAT3> &outline,
                                                     const std::vector<DirectX::XMFLOAT3> &midRing,
                                                     const std::vector<DirectX::XMFLOAT3> &dziubekRing,
                                                     const std::vector<DirectX::XMFLOAT3> &topRing,
                                                     const std::vector<DirectX::XMFLOAT3> &bottomRing);

    void transformAndAppend(std::vector<DirectX::XMFLOAT3> &positions,
                            const std::vector<DirectX::XMFLOAT3> &path,
                            float toolSize);
}


#endif //MG1_PATHSCREATORHELPER_H
