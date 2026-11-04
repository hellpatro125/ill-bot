#pragma once

#include <Geode/Geode.hpp>

class TrajectoryVisualizer {
public:
    static TrajectoryVisualizer& get() {
        static TrajectoryVisualizer instance;
        return instance;
    }

    cocos2d::CCDrawNode* m_drawNode = nullptr;

    void setupForPlayLayer(PlayLayer* playLayer);
    void updateTrajectory(PlayLayer* playLayer);
    void clear();
};
