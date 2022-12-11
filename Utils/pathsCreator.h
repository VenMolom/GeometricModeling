//
// Created by Molom on 10/11/2022.
//

#ifndef MG1_PATHSCREATOR_H
#define MG1_PATHSCREATOR_H

#include <filesystem>
#include "Objects/object.h"
#include "Objects/objectFactory.h"

class PathsCreator {
public:
    static void create(const std::filesystem::path &directory,
                       std::vector<std::shared_ptr<Object>> objects,
                       ObjectFactory &factory,
                       Renderer &renderer);

private:
    explicit PathsCreator(std::filesystem::path basePath, std::vector<std::shared_ptr<Object>> objects);

    void createRoughPaths(int toolSize, Renderer &renderer);

    void createFlatteningPaths(int toolSize, Renderer &renderer, ObjectFactory &factory);

    void createDetailPaths(int toolSize, Renderer &renderer, ObjectFactory &factory);

    std::filesystem::path basePath;
    std::vector<std::shared_ptr<Object>> objects;
    mini::dx_ptr<ID3D11Texture2D> allowedHeightStaging;

    std::shared_ptr<Patch> patch;
};


#endif //MG1_PATHSCREATOR_H
