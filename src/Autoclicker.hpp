#pragma once

#include <Geode/Geode.hpp>

class Autoclicker {
public:
    static Autoclicker& get() {
        static Autoclicker instance;
        return instance;
    }

    // Prevents recursion and prevents synthetic clicks from clobbering physical hold state
    bool m_isAutoclicking = false;

    // P1 Autoclicker State
    float m_p1Timer = 0.0f;
    bool m_p1IsDown = false;
    bool m_p1PhysicalHolding = false;

    // P2 Autoclicker State
    float m_p2Timer = 0.0f;
    bool m_p2IsDown = false;
    bool m_p2PhysicalHolding = false;

    void onUpdate(PlayLayer* playLayer, float dt);
    void onLevelReset();
    void onPlayerInput(bool down, int button, bool isPlayer2);
};