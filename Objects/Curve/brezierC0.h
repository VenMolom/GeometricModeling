//
// Created by Molom on 2022-03-25.
//

#ifndef MG1_BREZIERC0_H
#define MG1_BREZIERC0_H

#include "brezierCurve.h"
#include <Models/Curves/BezierC0.h>

class BrezierC0 : public BrezierCurve {
public:
    BrezierC0(uint id, std::vector<std::weak_ptr<Point>> &&points);

    BrezierC0(const MG1::Bezier &curve, const std::list<std::shared_ptr<Object>> &sceneObjects);

    Type type() const override;

    MG1::BezierC0 serialize();
};


#endif //MG1_BREZIERC0_H
