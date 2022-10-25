//
// Created by Molom on 13/10/2022.
//

#include "fileParser.h"
#include <fstream>
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
    smatch matches;
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
    if (regex_match(line, matches,
                    regex(R"(N\d*G0[0|1](X-?\d*\.?\d*)?(Y-?\d*\.?\d*)?(Z-?\d*\.?\d*)?)"))) {
        CNCMove move{};

        if (path.moves.empty()) {
            move.target = XMFLOAT3(0, 0, 0);
        } else {
            move.target = path.moves[path.moves.size() - 1].target;
        }

        for (int i = 1; i < matches.size(); ++i) {
            auto match = matches[i].str();
            switch (match[0]) {
                case 'X':
                    move.target.x = stof(match.c_str() + 1) / 10.f;
                    break;
                case 'Y':
                    move.target.y = stof(match.c_str() + 1) / 10.f;
                    break;
                case 'Z':
                    move.target.z = stof(match.c_str() + 1) / 10.f;
                    break;
            }
        }
        path.moves.push_back(move);
        return;
    }
}
