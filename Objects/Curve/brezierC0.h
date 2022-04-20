//
// Created by Molom on 2022-03-25.
//

#ifndef MG1_BREZIERC0_H
#define MG1_BREZIERC0_H

#include "brezierCurve.h"

class BrezierC0 : public BrezierCurve {
public:
    BrezierC0(uint id, std::vector<std::weak_ptr<Point>> &&points);

    Type type() const override;
};


#endif //MG1_BREZIERC0_H
