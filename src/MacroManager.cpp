#include "MacroManager.hpp"
#include "Config.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace geode::prelude;

void MacroManager::onLevelReset() {
    m_currentFrame = 0;
    m_playIndex = 0;
    m_isReplaying = false;
}

void MacroManager::startRecord() {
    Config::get().macroMode = MacroMode::Record;
    clear();
}

void MacroManager::startPlayback() {
    Config::get().macroMode = MacroMode::Play;
    m_playIndex = 0;
    m_currentFrame = 0;
}

void MacroManager::stop() {
    Config::get().macroMode = MacroMode::Off;
    m_isReplaying = false;
}

void MacroManager::handlePlayerInput(bool down, int button, bool isPlayer2) {
    if (m_isReplaying) return;
    if (Config::get().macroMode == MacroMode::Record) {
        MacroAction act;
        act.frame = m_currentFrame;
        act.button = button;
        act.player2 = isPlayer2;
        act.down = down;
        m_actions.push_back(act);
    }
}

void MacroManager::onFrameTick(PlayLayer* playLayer) {
    if (!playLayer || !playLayer->m_player1 || playLayer->m_player1->m_isDead) return;

    if (Config::get().macroMode == MacroMode::Play) {
        m_isReplaying = true;
        while (m_playIndex < m_actions.size() && m_actions[m_playIndex].frame <= m_currentFrame) {
            const auto& act = m_actions[m_playIndex];
            playLayer->handleButton(act.down, act.button, act.player2);
            m_playIndex++;
        }
        m_isReplaying = false;
    }

    m_currentFrame++;
}

void MacroManager::clear() {
    m_actions.clear();
    m_playIndex = 0;
    m_currentFrame = 0;
    m_isReplaying = false;
}

static std::filesystem::path getMacrosDir() {
    auto dir = geode::dirs::getSaveDir() / "macros";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir;
}

bool MacroManager::saveToFile(const std::string& name) {
    auto path = getMacrosDir() / (name + ".illmacro");
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "ILLMACRO_V1\n";
    file << (Config::get().tpsEnabled ? Config::get().targetTPS : 240.0f) << "\n";
    file << m_actions.size() << "\n";

    for (const auto& act : m_actions) {
        file << act.frame << " " << act.button << " " << (act.player2 ? 1 : 0) << " " << (act.down ? 1 : 0) << "\n";
    }

    m_lastLoadedName = name;
    return true;
}

bool MacroManager::loadFromFile(const std::string& name) {
    auto path = getMacrosDir() / (name + ".illmacro");
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string header;
    file >> header;
    if (header != "ILLMACRO_V1") return false;

    float tps = 240.0f;
    file >> tps;
    m_recordedTPS = tps;

    size_t count = 0;
    file >> count;

    m_actions.clear();
    m_actions.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        MacroAction act;
        int p2 = 0;
        int down = 0;
        file >> act.frame >> act.button >> p2 >> down;
        act.player2 = (p2 != 0);
        act.down = (down != 0);
        m_actions.push_back(act);
    }

    m_playIndex = 0;
    m_currentFrame = 0;
    m_lastLoadedName = name;
    return true;
}