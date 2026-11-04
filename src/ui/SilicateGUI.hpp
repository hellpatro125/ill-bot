#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "../Config.hpp"
#include "../MacroManager.hpp"

enum class TabID {
    Record = 0,
    Autoclicker = 1,
    PhysicsTPS = 2,
    Trajectory = 3,
    Hitboxes = 4,
    Settings = 5
};

class SilicateGUI : public geode::Popup {
protected:
    TabID m_currentTab = TabID::Record;

    cocos2d::CCNode* m_contentContainer = nullptr;
    cocos2d::CCMenu* m_sidebarMenu = nullptr;
    cocos2d::CCDrawNode* m_backgroundNode = nullptr;

    bool init(float width, float height);

    void switchTab(TabID tab);
    void updateSidebarButtons();

    // Content Builders
    void buildRecordTab();
    void buildAutoclickerTab();
    void buildPhysicsTPSTab();
    void buildTrajectoryTab();
    void buildHitboxesTab();
    void buildSettingsTab();

    // Callbacks
    void onTabClicked(cocos2d::CCObject* sender);
    void onToggleP1Autoclicker(cocos2d::CCObject* sender);
    void onP1CPSMinus(cocos2d::CCObject* sender);
    void onP1CPSPlus(cocos2d::CCObject* sender);
    void onP1Preset(cocos2d::CCObject* sender);
    void onP1ToggleHold(cocos2d::CCObject* sender);

    void onToggleP2Autoclicker(cocos2d::CCObject* sender);
    void onP2CPSMinus(cocos2d::CCObject* sender);
    void onP2CPSPlus(cocos2d::CCObject* sender);
    void onP2Preset(cocos2d::CCObject* sender);
    void onP2ToggleHold(cocos2d::CCObject* sender);

    void onToggleTPS(cocos2d::CCObject* sender);
    void onTPSMinus(cocos2d::CCObject* sender);
    void onTPSPlus(cocos2d::CCObject* sender);
    void onTPSPreset(cocos2d::CCObject* sender);
    void onSpeedMinus(cocos2d::CCObject* sender);
    void onSpeedPlus(cocos2d::CCObject* sender);
    void onSpeedPreset(cocos2d::CCObject* sender);

    void onMacroRecord(cocos2d::CCObject* sender);
    void onMacroPlay(cocos2d::CCObject* sender);
    void onMacroOff(cocos2d::CCObject* sender);
    void onMacroSave(cocos2d::CCObject* sender);
    void onMacroLoad(cocos2d::CCObject* sender);
    void onMacroClear(cocos2d::CCObject* sender);

    void onToggleTrajectory(cocos2d::CCObject* sender);
    void onToggleHitboxes(cocos2d::CCObject* sender);
    void onToggleNoclip(cocos2d::CCObject* sender);
    void onToggleSafeMode(cocos2d::CCObject* sender);

public:
    static SilicateGUI* create();
};
