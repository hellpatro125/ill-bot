#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

#include "Config.hpp"
#include "MacroManager.hpp"
#include "Autoclicker.hpp"
#include "TPSManager.hpp"
#include "TrajectoryVisualizer.hpp"
#include "HitboxVisualizer.hpp"
#include "ui/SilicateGUI.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    Config::get().loadFromSettings();
    TPSManager::get().applyTPS();
    TrajectoryVisualizer::get().syncWithModEngines();
    log::info("ILLBot (Draggable Silicate Edition) loaded successfully!");
}

static void toggleILLBotMenu() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;

    if (auto existing = scene->getChildByID("SilicateGUI")) {
        existing->removeFromParentAndCleanup(true);
        return;
    }

    if (auto gui = SilicateGUI::create()) {
        gui->setID("SilicateGUI");
        scene->addChild(gui, 1000);
    }
}

// -----------------------------------------------------------------------------
// MAIN MENU BUTTON
// -----------------------------------------------------------------------------
class $modify(ILLMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (auto bottomMenu = this->getChildByID("bottom-menu")) {
            auto spr = ButtonSprite::create("ILL", "goldFont.fnt", "GJ_button_01.png", 0.7f);
            auto btn = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(ILLMenuLayer::onOpenILLMenu)
            );
            btn->setID("illbot-button"_spr);
            bottomMenu->addChild(btn);
            bottomMenu->updateLayout();
        }

        return true;
    }

    void onOpenILLMenu(CCObject*) {
        toggleILLBotMenu();
    }
};

// -----------------------------------------------------------------------------
// PAUSE MENU BUTTON
// -----------------------------------------------------------------------------
class $modify(ILLPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        if (auto rightMenu = this->getChildByID("right-button-menu")) {
            auto spr = ButtonSprite::create("ILL", "goldFont.fnt", "GJ_button_02.png", 0.7f);
            auto btn = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(ILLPauseLayer::onOpenILLPause)
            );
            btn->setID("illbot-pause-button"_spr);
            rightMenu->addChild(btn);
            rightMenu->updateLayout();
        }
    }

    void onOpenILLPause(CCObject*) {
        toggleILLBotMenu();
    }
};

// -----------------------------------------------------------------------------
// INPUT HOOKS (Recording, Autoclicker tracking)
// -----------------------------------------------------------------------------
class $modify(ILLBaseGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer2) {
        Autoclicker::get().onPlayerInput(down, button, isPlayer2);
        MacroManager::get().handlePlayerInput(down, button, isPlayer2);
        GJBaseGameLayer::handleButton(down, button, isPlayer2);
    }
};

// -----------------------------------------------------------------------------
// GAMEPLAY HOOKS (Autoclicker execution, Macro replay, Trajectory, Hitboxes, Noclip)
// -----------------------------------------------------------------------------
class $modify(ILLPlayLayer, PlayLayer) {
    void createObjectsFromSetupFinished() {
        PlayLayer::createObjectsFromSetupFinished();
        TrajectoryVisualizer::get().setupForPlayLayer(this);
        HitboxVisualizer::get().setupForPlayLayer(this);
    }

    void update(float dt) {
        // Crucial: Execute Autoclicker and Macro replay at the START of update,
        // BEFORE physics sub-stepping begins.
        // This ensures playLayer->handleButton calls are queued and dispatched
        // into all Geode hooks on the exact frame tick, allowing other bots
        // (Silicate, Eclipse, xdbot, etc.) to record them seamlessly!
        Autoclicker::get().onUpdate(this, dt);
        MacroManager::get().onFrameTick(this);

        PlayLayer::update(dt);
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        // Visualizations are drawn after physics calculations
        TrajectoryVisualizer::get().updateTrajectory(this);
        HitboxVisualizer::get().updateHitboxes(this);
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        Autoclicker::get().onLevelReset();
        MacroManager::get().onLevelReset();
        TrajectoryVisualizer::get().clear();
        HitboxVisualizer::get().clear();
    }

    void destroyPlayer(PlayerObject* player, GameObject* hazard) {
        if (Config::get().noclip) {
            return;
        }
        PlayLayer::destroyPlayer(player, hazard);
    }
};

// -----------------------------------------------------------------------------
// KEYBOARD DISPATCHER (Full Keybind Manager for ALL functions)
// -----------------------------------------------------------------------------
#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_MACOS)
class $modify(ILLKeyboardDispatcher, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double time) {
        auto& cfg = Config::get();

        // 1. If currently listening for a bind in Keybinds tab:
        if (cfg.listeningAction != BindAction::None && isKeyDown && !isKeyRepeat) {
            if (key == KEY_Escape) {
                // Cancel
                cfg.listeningAction = BindAction::None;
            } else if (key == KEY_Delete || key == KEY_BackSpace) {
                // Clear bind
                cfg.setKeyForAction(cfg.listeningAction, 0);
                cfg.listeningAction = BindAction::None;
            } else {
                cfg.setKeyForAction(cfg.listeningAction, static_cast<int>(key));
                cfg.listeningAction = BindAction::None;
            }
            if (auto gui = SilicateGUI::get()) {
                gui->refreshCurrentTab();
            }
            return true; // Swallow key
        }

        // 2. Dispatch hotkeys for all 9 functions on KeyDown
        if (isKeyDown && !isKeyRepeat) {
            int pressed = static_cast<int>(key);

            // Menu Toggle (Custom bind + fallback Tab)
            if (pressed == cfg.keyMenu || key == KEY_Tab) {
                toggleILLBotMenu();
                return true;
            }

            // P1 Autoclicker Toggle
            if (cfg.keyP1Auto != 0 && pressed == cfg.keyP1Auto) {
                cfg.p1Autoclicker = !cfg.p1Autoclicker;
                cfg.saveToSettings();
                Notification::create(
                    cfg.p1Autoclicker ? "P1 Autoclicker: ON" : "P1 Autoclicker: OFF",
                    cfg.p1Autoclicker ? NotificationIcon::Success : NotificationIcon::Warning
                )->show();
                if (auto gui = SilicateGUI::get()) gui->refreshCurrentTab();
                return true;
            }

            // P2 Autoclicker Toggle
            if (cfg.keyP2Auto != 0 && pressed == cfg.keyP2Auto) {
                cfg.p2Autoclicker = !cfg.p2Autoclicker;
                cfg.saveToSettings();
                Notification::create(
                    cfg.p2Autoclicker ? "P2 Autoclicker: ON" : "P2 Autoclicker: OFF",
                    cfg.p2Autoclicker ? NotificationIcon::Success : NotificationIcon::Warning
                )->show();
                if (auto gui = SilicateGUI::get()) gui->refreshCurrentTab();
                return true;
            }

            // TPS Bypass Toggle
            if (cfg.keyTPS != 0 && pressed == cfg.keyTPS) {
                cfg.tpsEnabled = !cfg.tpsEnabled;
                TPSManager::get().applyTPS();
                cfg.saveToSettings();
                Notification::create(
                    cfg.tpsEnabled ? fmt::format("TPS Bypass: {:.0f} TPS", cfg.targetTPS) : "TPS Bypass: OFF",
                    NotificationIcon::Info
                )->show();
                if (auto gui = SilicateGUI::get()) gui->refreshCurrentTab();
                return true;
            }

            // Trajectory Toggle
            if (cfg.keyTrajectory != 0 && pressed == cfg.keyTrajectory) {
                cfg.showTrajectory = !cfg.showTrajectory;
                TrajectoryVisualizer::get().syncWithModEngines();
                cfg.saveToSettings();
                Notification::create(
                    cfg.showTrajectory ? "Trajectory: ON" : "Trajectory: OFF",
                    NotificationIcon::Info
                )->show();
                if (auto gui = SilicateGUI::get()) gui->refreshCurrentTab();
                return true;
            }

            // Hitboxes Toggle
            if (cfg.keyHitboxes != 0 && pressed == cfg.keyHitboxes) {
                cfg.showHitboxes = !cfg.showHitboxes;
                cfg.saveToSettings();
                Notification::create(
                    cfg.showHitboxes ? "Hitboxes: ON" : "Hitboxes: OFF",
                    NotificationIcon::Info
                )->show();
                if (auto gui = SilicateGUI::get()) gui->refreshCurrentTab();
                return true;
            }

            // Macro Record Toggle
            if (cfg.keyMacroRecord != 0 && pressed == cfg.keyMacroRecord) {
                if (cfg.macroMode == MacroMode::Record) {
                    MacroManager::get().stop();
                    Notification::create("Macro: Record Stopped", NotificationIcon::Warning)->show();
                } else {
                    MacroManager::get().startRecord();
                    Notification::create("Macro: Recording Started", NotificationIcon::Success)->show();
                }
                if (auto gui = SilicateGUI::get()) gui->refreshCurrentTab();
                return true;
            }

            // Macro Play Toggle
            if (cfg.keyMacroPlay != 0 && pressed == cfg.keyMacroPlay) {
                if (cfg.macroMode == MacroMode::Play) {
                    MacroManager::get().stop();
                    Notification::create("Macro: Play Stopped", NotificationIcon::Warning)->show();
                } else {
                    MacroManager::get().startPlayback();
                    Notification::create("Macro: Playback Started", NotificationIcon::Success)->show();
                }
                if (auto gui = SilicateGUI::get()) gui->refreshCurrentTab();
                return true;
            }

            // Noclip Toggle
            if (cfg.keyNoclip != 0 && pressed == cfg.keyNoclip) {
                cfg.noclip = !cfg.noclip;
                cfg.saveToSettings();
                Notification::create(
                    cfg.noclip ? "Noclip: ON" : "Noclip: OFF",
                    cfg.noclip ? NotificationIcon::Warning : NotificationIcon::Info
                )->show();
                if (auto gui = SilicateGUI::get()) gui->refreshCurrentTab();
                return true;
            }
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, time);
    }
};
#endif