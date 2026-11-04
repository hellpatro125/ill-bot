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
    log::info("ILLBot (Silicate Edition) loaded successfully!");
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
        if (auto popup = SilicateGUI::create()) {
            popup->show();
        }
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
        if (auto popup = SilicateGUI::create()) {
            popup->show();
        }
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

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        Autoclicker::get().onUpdate(this, dt);
        MacroManager::get().onFrameTick(this);
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
// KEYBOARD DISPATCHER (Toggle menu with Tab, Left Alt, or F)
// -----------------------------------------------------------------------------
#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_MACOS)
class $modify(ILLKeyboardDispatcher, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double time) {
        if ((key == KEY_Tab || key == KEY_LeftAlt || key == KEY_F) && isKeyDown && !isKeyRepeat) {
            auto scene = CCDirector::sharedDirector()->getRunningScene();
            if (scene) {
                if (auto existing = scene->getChildByID("SilicateGUI")) {
                    existing->removeFromParentAndCleanup(true);
                    return true;
                }
                if (auto popup = SilicateGUI::create()) {
                    popup->setID("SilicateGUI");
                    popup->show();
                    return true;
                }
            }
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, time);
    }
};
#endif
