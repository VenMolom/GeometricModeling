//
// Created by Molom on 2022-04-06.
//

#ifndef MG1_VIRTUALPOINTSHOLDER_H
#define MG1_VIRTUALPOINTSHOLDER_H

#include "virtualPoint.h"

class VirtualPointsHolder {
public:
    VirtualPointsHolder() = delete;

    explicit VirtualPointsHolder(QBindable<std::weak_ptr<Object>> bindableSelected)
            : bindableSelected(bindableSelected) {
        selectedHandler = bindableSelected.addNotifier([this] {
            this->selected = this->bindableSelected.value();
        });
    }

    virtual const std::vector<std::shared_ptr<VirtualPoint>> &virtualPoints() = 0;

    virtual void replacePoint(std::shared_ptr<VirtualPoint> point, std::shared_ptr<VirtualPoint> newPoint) = 0;

    void setShowPoints(bool show) { _showPoints = show; }

    bool showPoints() const { return _showPoints; }

protected:
    std::weak_ptr<Object> selected;
    QBindable<std::weak_ptr<Object>> bindableSelected;

private:
    QPropertyNotifier selectedHandler;
    bool _showPoints{true};
};

#endif //MG1_VIRTUALPOINTSHOLDER_H
