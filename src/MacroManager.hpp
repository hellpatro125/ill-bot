#pragma once

#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include <cstdint>

struct MacroAction {
    uint64_t frame = 0;
    int button = 1;     // 1 = Jump, 2 = Left, 3 = Right
    bool player2 = false;
    bool down = false;
};

class MacroManager {
public:
    static MacroManager& get() {
        static MacroManager instance;
        return instance;
    }

    uint64_t m_currentFrame = 0;
    size_t m_playIndex = 0;
    std::vector<MacroAction> m_actions;
    float m_recordedTPS = 240.0f;
    std::string m_lastLoadedName = "Replay";
    bool m_isReplaying = false;

    void onLevelReset();
    void onFrameTick(PlayLayer* playLayer);
    void handlePlayerInput(bool down, int button, bool isPlayer2);

    void startRecord();
    void startPlayback();
    void stop();

    bool saveToFile(const std::string& name);
    bool loadFromFile(const std::string& name);
    void clear();

    size_t getActionCount() const { return m_actions.size(); }
    uint64_t getCurrentFrame() const { return m_currentFrame; }
};