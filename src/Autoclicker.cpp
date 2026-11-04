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
    m_isAutoclicking = false;
}

void Autoclicker::onPlayerInput(bool down, int button, bool isPlayer2) {
    // If the click is generated synthetically by the autoclicker itself,
    // do NOT overwrite the physical user holding state!
    if (m_isAutoclicking) return;

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

    auto dispatchClick = [this, playLayer](bool down, bool isPlayer2) {
        m_isAutoclicking = true;
        // Calling playLayer->handleButton routes through the Geode hook chain
        // so other mods (Silicate, Eclipse, xdbot, etc.) and ILLBot's MacroManager
        // can record every single click frame-accurately!
        playLayer->handleButton(down, 1, isPlayer2);
        m_isAutoclicking = false;
    };

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
                dispatchClick(false, false);
                m_p1IsDown = false;
            }
            m_p1Timer = 0.0f;
        } else {
            float cps = std::max(1.0f, cfg.p1CPS);
            float interval = 1.0f / cps;

            if (interval <= dt) {
                // High CPS mode: Alternate state every physics frame
                m_p1IsDown = !m_p1IsDown;
                dispatchClick(m_p1IsDown, false);
            } else {
                float halfInterval = interval * 0.5f;
                m_p1Timer += dt;

                if (m_p1Timer >= interval) {
                    m_p1Timer = std::fmod(m_p1Timer, interval);
                    if (!m_p1IsDown) {
                        dispatchClick(true, false);
                        m_p1IsDown = true;
                    }
                } else if (m_p1IsDown && m_p1Timer >= halfInterval) {
                    dispatchClick(false, false);
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
                dispatchClick(false, true);
                m_p2IsDown = false;
            }
            m_p2Timer = 0.0f;
        } else {
            float cps = std::max(1.0f, cfg.p2CPS);
            float interval = 1.0f / cps;

            if (interval <= dt) {
                // High CPS mode for Player 2
                m_p2IsDown = !m_p2IsDown;
                dispatchClick(m_p2IsDown, true);
            } else {
                float halfInterval = interval * 0.5f;
                m_p2Timer += dt;

                if (m_p2Timer >= interval) {
                    m_p2Timer = std::fmod(m_p2Timer, interval);
                    if (!m_p2IsDown) {
                        dispatchClick(true, true);
                        m_p2IsDown = true;
                    }
                } else if (m_p2IsDown && m_p2Timer >= halfInterval) {
                    dispatchClick(false, true);
                    m_p2IsDown = false;
                }
            }
        }
    }
}