//
// Created by Molom on 13/10/2022.
//

#include "fileParser.h"
#include <fstream>
#include <ostream>
#include <regex>

using namespace std;
using namespace DirectX;

CNCPath FileParser::parseCNCPath(std::filesystem::path path) {
    ifstream input;
    input.open(path);

    CNCPath result{};
    smatch matches;
    static const regex rgx(R"(\.([k|f])(\d\d)$)");
    result.filename = path.filename().string();
    if (regex_search(result.filename, matches, rgx)) {
        switch (matches[1].str()[0]) {
            case 'k':
                result.type = CNCType::Round;
                break;
            case 'f':
                result.type = CNCType::Flat;
                break;
        }
        result.size = stoi(matches[2]);
    }

    string line;
    // ignore first line, we use metric system anyway
    while (getline(input, line)) {
        parseCNCLine(line, result);
    }

    input.close();

    return result;
}

void FileParser::parseCNCLine(const std::string &line, CNCPath &path) {
//    static const regex rgx(R"(N(\d*)(.*))");
    stringstream ss(line);
//    // return if doesn't have instruction number
//    if (!regex_match(line, matches, rgx)) { return; }
//    auto number = stoi(matches[1]);
//    auto instruction = matches[2].str();

    // global coordinates
//    if (regex_match(instruction, matches, regex("G40G90"))) {
//        path.globalCoordinates = true;
//        return;
//    }

//    // rotation speed
//    if (regex_match(instruction, matches, regex(R"(S(\d\d\d\d))"))) {
//        path.rotationSpeed = stoi(matches[1]);
//        return;
//    }
//
//    // linear speed
//    if (regex_match(instruction, matches, regex(R"(F(\d\d\d\d))"))) {
//        path.linearSpeed = stof(matches[1]) / 100.f;
//        return;
//    }

    //move
    // N\d*G0[0|1](X-?\d*\.?\d*)?(Y-?\d*\.?\d*)?(Z-?\d*\.?\d*)?
    CNCMove move{};
    ss.seekg(1); // skip N
    ss >> move.number;
    ss.seekg(3, stringstream::cur); // skip G0[0|1]

    if (path.moves.empty()) {
        move.target = XMFLOAT3(0, 0, 0);
    } else {
        move.target = path.moves[path.moves.size() - 1].target;
    }

    char axis;
    float value;

    while (!ss.eof()) {
        ss >> axis >> value;
        switch (axis) {
            case 'X':
                move.target.x = value / 10.f;
                break;
            case 'Y':
                move.target.y = value / 10.f;
                break;
            case 'Z':
                move.target.z = value / 10.f;
                break;
        }
    }

    path.moves.push_back(move);
}

void FileParser::saveCNCPath(const std::filesystem::path& path, const std::vector<DirectX::XMFLOAT3>& positions) {
    ofstream output;
    output.open(path, ios_base::out | ios_base::trunc);

    int i = 1;
    XMFLOAT3 last(-INFINITY, -INFINITY, -INFINITY);
    for (auto& pos : positions) {
        output << std::format("N{}G01", i++);
        if (abs(pos.x - last.x) > FLT_EPSILON) {
            output << std::format("X{:.3f}", pos.x);
        }
        if (abs(pos.y - last.y) > FLT_EPSILON) {
            output << std::format("Y{:.3f}", pos.y);
        }
        if (abs(pos.z - last.z) > FLT_EPSILON) {
            output << std::format("Z{:.3f}", pos.z);
        }
        output << "\n";
        last = pos;
    }

    output.close();
}
