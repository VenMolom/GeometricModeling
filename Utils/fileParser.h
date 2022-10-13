//
// Created by Molom on 13/10/2022.
//

#ifndef MG1_FILEPARSER_H
#define MG1_FILEPARSER_H

#include <DirectXMath.h>
#include <vector>
#include <filesystem>

struct CNCMove {
    int number;
    bool fast;
    DirectX::XMFLOAT3 target;
};

enum CNCType {
    Flat,
    Round
};

struct CNCPath {
    CNCType type;
    int size;
    bool globalCoordinates;
    int rotationSpeed;
    float linearSpeed;

    std::vector<CNCMove> moves{};
};
namespace FileParser {
    CNCPath parseCNCPath(std::filesystem::path path);
    void parseCNCLine(const std::string& line, CNCPath &path);
};


#endif //MG1_FILEPARSER_H
