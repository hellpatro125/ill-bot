#pragma once

#include <Geode/Geode.hpp>
#include <string>

enum class MacroMode {
    Off = 0,
    Record = 1,
    Play = 2
};

class Config {
public:
    static Config& get() {
        static Config instance;
        return instance;
    }

    // Autoclicker Settings (Independent for P1 and P2)
    bool p1Autoclicker = false;
    float p1CPS = 60.0f;
    bool p1HoldMode = false; // false = spam always during gameplay, true = only while holding

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
    bool showDeathBox = true;
    bool showHitboxes = false;
    bool showPlayerHitbox = true;
    bool showHazardHitbox = true;

    // Utilities
    bool noclip = false;
    bool safeMode = true;

    void loadFromSettings() {
        auto* mod = geode::Mod::get();
        if (!mod) return;

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
