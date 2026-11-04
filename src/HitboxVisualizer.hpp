#pragma once

#include <Geode/Geode.hpp>

class HitboxVisualizer {
public:
    static HitboxVisualizer& get() {
        static HitboxVisualizer instance;
        return instance;
    }

    cocos2d::CCDrawNode* m_drawNode = nullptr;

    void setupForPlayLayer(PlayLayer* playLayer);
    void updateHitboxes(PlayLayer* playLayer);
    void clear();
};
