//
// Created by Molom on 10/11/2022.
//

#ifndef MG1_PATHSCREATOR_H
#define MG1_PATHSCREATOR_H

#include <filesystem>
#include "Objects/object.h"

class PathsCreator {
    static constexpr float BLOCK_SIZE_XY = 15.f;
    static constexpr float BLOCK_SIZE_Z = 5.f;
    static constexpr float BLOCK_BOTTOM = 1.5f;

public:
    static constexpr int TEX_SIZE = 4096;

    static void create(const std::filesystem::path &directory,
                       std::vector<std::shared_ptr<Object>> objects,
                       Renderer &renderer);

private:
    explicit PathsCreator(std::filesystem::path basePath, std::vector<std::shared_ptr<Object>> objects);

    void createRoughPaths(int toolSize, Renderer &renderer);

    void createFlatteningPaths(int toolSize);

    void createDetailPaths(int toolSize);

    static std::pair<mini::dx_ptr<ID3D11DepthStencilView>, mini::dx_ptr<ID3D11Texture2D>>
    createDepthTexture(const DxDevice &device);

    static void copyResource(const DxDevice &device, mini::dx_ptr<ID3D11DepthStencilView> &source,
                      mini::dx_ptr<ID3D11Texture2D> &target);

    std::filesystem::path basePath;
    std::vector<std::shared_ptr<Object>> objects;
};


#endif //MG1_PATHSCREATOR_H
