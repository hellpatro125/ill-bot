#include "Autoclicker.hpp"
#include "Config.hpp"
#include <cmath>

using namespace geode::prelude;

void Autoclicker::onLevelReset() {
    m_p1Timer = 0.0f;
    m_p1IsDown = false;
    m_p1PhysicalHolding = false;

    m_p2Timer = 0.0f;
    m_p2IsDown = false;
    m_p2PhysicalHolding = false;
}

void Autoclicker::onPlayerInput(bool down, int button, bool isPlayer2) {
    if (button == 1) { // Jump button
        if (!isPlayer2) {
            m_p1PhysicalHolding = down;
        } else {
            m_p2PhysicalHolding = down;
        }
    }
}

void Autoclicker::onUpdate(PlayLayer* playLayer, float dt) {
    if (!playLayer || !playLayer->m_player1 || playLayer->m_player1->m_isDead) return;

    auto& cfg = Config::get();

    // -------------------------------------------------------------
    // Player 1 Autoclicker
    // -------------------------------------------------------------
    if (cfg.p1Autoclicker) {
        bool shouldClick = true;
        if (cfg.p1HoldMode && !m_p1PhysicalHolding) {
            shouldClick = false;
        }

        if (!shouldClick) {
            if (m_p1IsDown) {
                playLayer->handleButton(false, 1, false);
                m_p1IsDown = false;
            }
            m_p1Timer = 0.0f;
        } else {
            float cps = std::max(1.0f, cfg.p1CPS);
            float interval = 1.0f / cps;

            if (interval <= dt) {
                // High CPS mode: Alternate state every physics frame
                m_p1IsDown = !m_p1IsDown;
                playLayer->handleButton(m_p1IsDown, 1, false);
            } else {
                float halfInterval = interval * 0.5f;
                m_p1Timer += dt;

                if (m_p1Timer >= interval) {
                    m_p1Timer = std::fmod(m_p1Timer, interval);
                    if (!m_p1IsDown) {
                        playLayer->handleButton(true, 1, false);
                        m_p1IsDown = true;
                    }
                } else if (m_p1IsDown && m_p1Timer >= halfInterval) {
                    playLayer->handleButton(false, 1, false);
                    m_p1IsDown = false;
                }
            }
        }
    }

    // -------------------------------------------------------------
    // Player 2 Autoclicker (Dual mode)
    // -------------------------------------------------------------
    if (cfg.p2Autoclicker) {
        bool shouldClick = true;
        if (cfg.p2HoldMode && !m_p2PhysicalHolding) {
            shouldClick = false;
        }

        if (!shouldClick) {
            if (m_p2IsDown) {
                playLayer->handleButton(false, 1, true);
                m_p2IsDown = false;
            }
            m_p2Timer = 0.0f;
        } else {
            float cps = std::max(1.0f, cfg.p2CPS);
            float interval = 1.0f / cps;

            if (interval <= dt) {
                // High CPS mode for Player 2
                m_p2IsDown = !m_p2IsDown;
                playLayer->handleButton(m_p2IsDown, 1, true);
            } else {
                float halfInterval = interval * 0.5f;
                m_p2Timer += dt;

                if (m_p2Timer >= interval) {
                    m_p2Timer = std::fmod(m_p2Timer, interval);
                    if (!m_p2IsDown) {
                        playLayer->handleButton(true, 1, true);
                        m_p2IsDown = true;
                    }
                } else if (m_p2IsDown && m_p2Timer >= halfInterval) {
                    playLayer->handleButton(false, 1, true);
                    m_p2IsDown = false;
                }
            }
        }
    }
}
