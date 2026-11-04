#pragma once

#include <Geode/Geode.hpp>
#include <string>
#include <fmt/format.h>

enum class MacroMode {
    Off = 0,
    Record = 1,
    Play = 2
};

enum class BindAction {
    None = 0,
    ToggleMenu,
    ToggleP1Autoclicker,
    ToggleP2Autoclicker,
    ToggleTPS,
    ToggleTrajectory,
    ToggleHitboxes,
    ToggleMacroRecord,
    ToggleMacroPlay,
    ToggleNoclip
};

class Config {
public:
    static Config& get() {
        static Config instance;
        return instance;
    }

    // GUI Position & Dragging
    float guiPosX = -1.0f;
    float guiPosY = -1.0f;

    // Keybinds for EVERY function (default keycodes)
    int keyMenu = 9;            // Tab (9)
    int keyP1Auto = 90;         // Z (90)
    int keyP2Auto = 88;         // X (88)
    int keyTPS = 66;            // B (66)
    int keyTrajectory = 84;     // T (84)
    int keyHitboxes = 72;       // H (72)
    int keyMacroRecord = 82;    // R (82)
    int keyMacroPlay = 80;      // P (80)
    int keyNoclip = 78;         // N (78)

    BindAction listeningAction = BindAction::None;

    // Autoclicker Settings (Independent for P1 and P2)
    bool p1Autoclicker = false;
    float p1CPS = 60.0f;
    bool p1HoldMode = false;

    bool p2Autoclicker = false;
    float p2CPS = 60.0f;
    bool p2HoldMode = false;

    // TPS & Physics Changer
    bool tpsEnabled = false;
    float targetTPS = 240.0f;
    bool speedhackEnabled = false;
    float speedhackValue = 1.0f;

    // Macro Replay
    MacroMode macroMode = MacroMode::Off;
    std::string replayName = "Replay";
    bool frameAdvance = false;
    bool intentionalDeath = false;

    // Visuals & Assists
    bool showTrajectory = true;
    int trajectorySteps = 60;
    bool showDeathBox = false;
    bool showHitboxes = false;
    bool showPlayerHitbox = true;
    bool showHazardHitbox = true;

    // Utilities
    bool noclip = false;
    bool safeMode = true;

    int getKeyForAction(BindAction action) const {
        switch (action) {
            case BindAction::ToggleMenu: return keyMenu;
            case BindAction::ToggleP1Autoclicker: return keyP1Auto;
            case BindAction::ToggleP2Autoclicker: return keyP2Auto;
            case BindAction::ToggleTPS: return keyTPS;
            case BindAction::ToggleTrajectory: return keyTrajectory;
            case BindAction::ToggleHitboxes: return keyHitboxes;
            case BindAction::ToggleMacroRecord: return keyMacroRecord;
            case BindAction::ToggleMacroPlay: return keyMacroPlay;
            case BindAction::ToggleNoclip: return keyNoclip;
            default: return 0;
        }
    }

    void setKeyForAction(BindAction action, int key) {
        switch (action) {
            case BindAction::ToggleMenu: keyMenu = key; break;
            case BindAction::ToggleP1Autoclicker: keyP1Auto = key; break;
            case BindAction::ToggleP2Autoclicker: keyP2Auto = key; break;
            case BindAction::ToggleTPS: keyTPS = key; break;
            case BindAction::ToggleTrajectory: keyTrajectory = key; break;
            case BindAction::ToggleHitboxes: keyHitboxes = key; break;
            case BindAction::ToggleMacroRecord: keyMacroRecord = key; break;
            case BindAction::ToggleMacroPlay: keyMacroPlay = key; break;
            case BindAction::ToggleNoclip: keyNoclip = key; break;
            default: break;
        }
        saveToSettings();
    }

    std::string getActionDisplayName(BindAction action) const {
        switch (action) {
            case BindAction::ToggleMenu: return "Toggle Menu";
            case BindAction::ToggleP1Autoclicker: return "P1 Autoclicker";
            case BindAction::ToggleP2Autoclicker: return "P2 Autoclicker";
            case BindAction::ToggleTPS: return "TPS Bypass";
            case BindAction::ToggleTrajectory: return "Trajectory";
            case BindAction::ToggleHitboxes: return "Hitboxes";
            case BindAction::ToggleMacroRecord: return "Macro Record";
            case BindAction::ToggleMacroPlay: return "Macro Play";
            case BindAction::ToggleNoclip: return "Noclip";
            default: return "Unknown";
        }
    }

    std::string getKeyDisplayName(int code) const {
        if (code == 0) return "None";
        if (code >= '0' && code <= '9') return std::string(1, static_cast<char>(code));
        if (code >= 'A' && code <= 'Z') return std::string(1, static_cast<char>(code));
        if (code >= 112 && code <= 123) return fmt::format("F{}", code - 111);
        switch (code) {
            case 8: return "Backspace";
            case 9: return "Tab";
            case 13: return "Enter";
            case 16: return "Shift";
            case 17: return "Ctrl";
            case 18: return "Alt";
            case 19: return "Pause";
            case 20: return "CapsLock";
            case 27: return "Esc";
            case 32: return "Space";
            case 33: return "PageUp";
            case 34: return "PageDown";
            case 35: return "End";
            case 36: return "Home";
            case 37: return "Left";
            case 38: return "Up";
            case 39: return "Right";
            case 40: return "Down";
            case 45: return "Insert";
            case 46: return "Delete";
            case 186: return ";";
            case 187: return "=";
            case 188: return ",";
            case 189: return "-";
            case 190: return ".";
            case 191: return "/";
            case 192: return "~";
            case 219: return "[";
            case 220: return "\\";
            case 221: return "]";
            case 222: return "'";
            default: return fmt::format("#{:02X}", code);
        }
    }

    void loadFromSettings() {
        auto* mod = geode::Mod::get();
        if (!mod) return;

        try { guiPosX = static_cast<float>(mod->getSettingValue<double>("gui-pos-x")); } catch (...) {}
        try { guiPosY = static_cast<float>(mod->getSettingValue<double>("gui-pos-y")); } catch (...) {}

        try { keyMenu = static_cast<int>(mod->getSettingValue<int64_t>("key-menu")); } catch (...) {}
        try { keyP1Auto = static_cast<int>(mod->getSettingValue<int64_t>("key-p1-auto")); } catch (...) {}
        try { keyP2Auto = static_cast<int>(mod->getSettingValue<int64_t>("key-p2-auto")); } catch (...) {}
        try { keyTPS = static_cast<int>(mod->getSettingValue<int64_t>("key-tps")); } catch (...) {}
        try { keyTrajectory = static_cast<int>(mod->getSettingValue<int64_t>("key-trajectory")); } catch (...) {}
        try { keyHitboxes = static_cast<int>(mod->getSettingValue<int64_t>("key-hitboxes")); } catch (...) {}
        try { keyMacroRecord = static_cast<int>(mod->getSettingValue<int64_t>("key-macro-record")); } catch (...) {}
        try { keyMacroPlay = static_cast<int>(mod->getSettingValue<int64_t>("key-macro-play")); } catch (...) {}
        try { keyNoclip = static_cast<int>(mod->getSettingValue<int64_t>("key-noclip")); } catch (...) {}

        try { p1Autoclicker = mod->getSettingValue<bool>("p1-autoclicker"); } catch (...) {}
        try { p1CPS = static_cast<float>(mod->getSettingValue<double>("p1-cps")); } catch (...) {}
        try { p1HoldMode = mod->getSettingValue<bool>("p1-hold-mode"); } catch (...) {}

        try { p2Autoclicker = mod->getSettingValue<bool>("p2-autoclicker"); } catch (...) {}
        try { p2CPS = static_cast<float>(mod->getSettingValue<double>("p2-cps")); } catch (...) {}
        try { p2HoldMode = mod->getSettingValue<bool>("p2-hold-mode"); } catch (...) {}

        try { tpsEnabled = mod->getSettingValue<bool>("tps-enabled"); } catch (...) {}
        try { targetTPS = static_cast<float>(mod->getSettingValue<double>("tps-value")); } catch (...) {}
        try { speedhackEnabled = mod->getSettingValue<bool>("speedhack-enabled"); } catch (...) {}
        try { speedhackValue = static_cast<float>(mod->getSettingValue<double>("speedhack-value")); } catch (...) {}

        try { showTrajectory = mod->getSettingValue<bool>("show-trajectory"); } catch (...) {}
        try { showHitboxes = mod->getSettingValue<bool>("show-hitboxes"); } catch (...) {}
        try { noclip = mod->getSettingValue<bool>("noclip"); } catch (...) {}
        try { safeMode = mod->getSettingValue<bool>("safe-mode"); } catch (...) {}
    }

    void saveToSettings() {
        auto* mod = geode::Mod::get();
        if (!mod) return;

        (void)mod->setSettingValue<double>("gui-pos-x", static_cast<double>(guiPosX));
        (void)mod->setSettingValue<double>("gui-pos-y", static_cast<double>(guiPosY));

        (void)mod->setSettingValue<int64_t>("key-menu", static_cast<int64_t>(keyMenu));
        (void)mod->setSettingValue<int64_t>("key-p1-auto", static_cast<int64_t>(keyP1Auto));
        (void)mod->setSettingValue<int64_t>("key-p2-auto", static_cast<int64_t>(keyP2Auto));
        (void)mod->setSettingValue<int64_t>("key-tps", static_cast<int64_t>(keyTPS));
        (void)mod->setSettingValue<int64_t>("key-trajectory", static_cast<int64_t>(keyTrajectory));
        (void)mod->setSettingValue<int64_t>("key-hitboxes", static_cast<int64_t>(keyHitboxes));
        (void)mod->setSettingValue<int64_t>("key-macro-record", static_cast<int64_t>(keyMacroRecord));
        (void)mod->setSettingValue<int64_t>("key-macro-play", static_cast<int64_t>(keyMacroPlay));
        (void)mod->setSettingValue<int64_t>("key-noclip", static_cast<int64_t>(keyNoclip));

        (void)mod->setSettingValue<bool>("p1-autoclicker", p1Autoclicker);
        (void)mod->setSettingValue<double>("p1-cps", static_cast<double>(p1CPS));
        (void)mod->setSettingValue<bool>("p1-hold-mode", p1HoldMode);

        (void)mod->setSettingValue<bool>("p2-autoclicker", p2Autoclicker);
        (void)mod->setSettingValue<double>("p2-cps", static_cast<double>(p2CPS));
        (void)mod->setSettingValue<bool>("p2-hold-mode", p2HoldMode);

        (void)mod->setSettingValue<bool>("tps-enabled", tpsEnabled);
        (void)mod->setSettingValue<double>("tps-value", static_cast<double>(targetTPS));
        (void)mod->setSettingValue<bool>("speedhack-enabled", speedhackEnabled);
        (void)mod->setSettingValue<double>("speedhack-value", static_cast<double>(speedhackValue));

        (void)mod->setSettingValue<bool>("show-trajectory", showTrajectory);
        (void)mod->setSettingValue<bool>("show-hitboxes", showHitboxes);
        (void)mod->setSettingValue<bool>("noclip", noclip);
        (void)mod->setSettingValue<bool>("safe-mode", safeMode);
    }
};