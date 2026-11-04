#include "SilicateGUI.hpp"
#include "../Autoclicker.hpp"
#include "../TPSManager.hpp"
#include "../TrajectoryVisualizer.hpp"
#include "../HitboxVisualizer.hpp"
#include <fmt/format.h>

using namespace geode::prelude;

SilicateGUI* SilicateGUI::create() {
    auto ret = new SilicateGUI();
    if (ret && ret->init(540.0f, 350.0f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool SilicateGUI::init(float width, float height) {
    if (!geode::Popup::init(width, height, "GJ_square01.png")) return false;

    // Hide default square sprite and render our custom Silicate glass styling
    if (this->m_bgSprite) {
        this->m_bgSprite->setVisible(false);
    }
    this->setTitle(""); // Custom header in sidebar

    // 1. Draw Custom Dark Ruby / Crimson Frosted Acrylic Background
    m_backgroundNode = CCDrawNode::create();
    if (m_backgroundNode) {
        CCPoint bgVerts[4] = {
            CCPoint(0.0f, 0.0f),
            CCPoint(width, 0.0f),
            CCPoint(width, height),
            CCPoint(0.0f, height)
        };
        // Deep wine/crimson acrylic fill
        ccColor4F fillColor = {0.08f, 0.02f, 0.04f, 0.96f};
        ccColor4F strokeColor = {0.40f, 0.10f, 0.18f, 0.95f};
        m_backgroundNode->drawPolygon(bgVerts, 4, fillColor, 1.8f, strokeColor);

        // Sidebar Divider line
        CCPoint dividerVerts[2] = { CCPoint(145.0f, 0.0f), CCPoint(145.0f, height) };
        m_backgroundNode->drawSegment(dividerVerts[0], dividerVerts[1], 1.0f, {0.25f, 0.06f, 0.12f, 0.85f});

        // Sidebar Header line
        CCPoint headerLine[2] = { CCPoint(0.0f, height - 48.0f), CCPoint(145.0f, height - 48.0f) };
        m_backgroundNode->drawSegment(headerLine[0], headerLine[1], 0.8f, {0.25f, 0.06f, 0.12f, 0.85f});

        this->m_mainLayer->addChild(m_backgroundNode, -1);
    }

    // 2. Sidebar Header: Cat Icon + "Silicate" / "ILLBot"
    auto logoTitle = CCLabelBMFont::create("Silicate", "bigFont.fnt");
    logoTitle->setScale(0.52f);
    logoTitle->setPosition({72.5f, height - 23.0f});
    this->m_mainLayer->addChild(logoTitle);

    auto subtitle = CCLabelBMFont::create("ILL BOT SUITE", "goldFont.fnt");
    subtitle->setScale(0.30f);
    subtitle->setPosition({72.5f, height - 38.0f});
    this->m_mainLayer->addChild(subtitle);

    // 3. Sidebar Navigation Menu
    m_sidebarMenu = CCMenu::create();
    m_sidebarMenu->setPosition({0.0f, 0.0f});
    this->m_mainLayer->addChild(m_sidebarMenu);

    updateSidebarButtons();

    // 4. Content Area Node
    m_contentContainer = CCNode::create();
    m_contentContainer->setPosition({155.0f, 10.0f});
    this->m_mainLayer->addChild(m_contentContainer);

    // Build the default tab (Record)
    switchTab(TabID::Record);

    return true;
}

void SilicateGUI::updateSidebarButtons() {
    if (!m_sidebarMenu) return;
    m_sidebarMenu->removeAllChildrenWithCleanup(true);

    const char* tabNames[] = {
        "Record",
        "Autoclicker",
        "Physics / TPS",
        "Trajectory",
        "Hitboxes",
        "Settings"
    };

    float startY = 270.0f;
    float stepY = 44.0f;

    for (int i = 0; i < 6; ++i) {
        bool isActive = (static_cast<int>(m_currentTab) == i);

        // Styling for active vs inactive tab buttons
        const char* bgSpriteName = isActive ? "GJ_button_01.png" : "GJ_button_04.png";
        const char* fontName = isActive ? "bigFont.fnt" : "chatFont.fnt";

        auto btnSpr = ButtonSprite::create(tabNames[i], fontName, bgSpriteName, 0.65f);
        btnSpr->setScale(0.72f);
        if (!isActive) {
            btnSpr->setColor({170, 160, 175});
        }

        auto btn = CCMenuItemSpriteExtra::create(
            btnSpr, this, menu_selector(SilicateGUI::onTabClicked)
        );
        btn->setTag(i);
        btn->setPosition({72.5f, startY - (i * stepY)});
        m_sidebarMenu->addChild(btn);
    }
}

void SilicateGUI::onTabClicked(CCObject* sender) {
    if (auto node = typeinfo_cast<CCNode*>(sender)) {
        m_currentTab = static_cast<TabID>(node->getTag());
        updateSidebarButtons();
        switchTab(m_currentTab);
    }
}

void SilicateGUI::switchTab(TabID tab) {
    if (!m_contentContainer) return;
    m_contentContainer->removeAllChildrenWithCleanup(true);

    switch (tab) {
        case TabID::Record: buildRecordTab(); break;
        case TabID::Autoclicker: buildAutoclickerTab(); break;
        case TabID::PhysicsTPS: buildPhysicsTPSTab(); break;
        case TabID::Trajectory: buildTrajectoryTab(); break;
        case TabID::Hitboxes: buildHitboxesTab(); break;
        case TabID::Settings: buildSettingsTab(); break;
    }
}

// =============================================================================
// TAB 1: RECORD & MACRO
// =============================================================================
void SilicateGUI::buildRecordTab() {
    auto title = CCLabelBMFont::create("Record", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 320.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& macro = MacroManager::get();
    auto& cfg = Config::get();

    // Mode Selector Buttons: [Off] [Record] [Play]
    auto offSpr = ButtonSprite::create("Off", "bigFont.fnt", cfg.macroMode == MacroMode::Off ? "GJ_button_01.png" : "GJ_button_04.png", 0.6f);
    offSpr->setScale(0.65f);
    auto offBtn = CCMenuItemSpriteExtra::create(offSpr, this, menu_selector(SilicateGUI::onMacroOff));
    offBtn->setPosition({60.0f, 260.0f});
    menu->addChild(offBtn);

    auto recSpr = ButtonSprite::create("Record", "bigFont.fnt", cfg.macroMode == MacroMode::Record ? "GJ_button_01.png" : "GJ_button_04.png", 0.6f);
    recSpr->setScale(0.65f);
    auto recBtn = CCMenuItemSpriteExtra::create(recSpr, this, menu_selector(SilicateGUI::onMacroRecord));
    recBtn->setPosition({160.0f, 260.0f});
    menu->addChild(recBtn);

    auto playSpr = ButtonSprite::create("Play", "bigFont.fnt", cfg.macroMode == MacroMode::Play ? "GJ_button_01.png" : "GJ_button_04.png", 0.6f);
    playSpr->setScale(0.65f);
    auto playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(SilicateGUI::onMacroPlay));
    playBtn->setPosition({260.0f, 260.0f});
    menu->addChild(playBtn);

    // Save & Load Replay
    auto loadSpr = ButtonSprite::create("Load Replay", "goldFont.fnt", "GJ_button_02.png", 0.6f);
    loadSpr->setScale(0.7f);
    auto loadBtn = CCMenuItemSpriteExtra::create(loadSpr, this, menu_selector(SilicateGUI::onMacroLoad));
    loadBtn->setPosition({95.0f, 205.0f});
    menu->addChild(loadBtn);

    auto saveSpr = ButtonSprite::create("Save Replay", "goldFont.fnt", "GJ_button_02.png", 0.6f);
    saveSpr->setScale(0.7f);
    auto saveBtn = CCMenuItemSpriteExtra::create(saveSpr, this, menu_selector(SilicateGUI::onMacroSave));
    saveBtn->setPosition({225.0f, 205.0f});
    menu->addChild(saveBtn);

    // Status Info Label
    std::string statusText = fmt::format(
        "Status: {}\nRecorded Actions: {}\nRecorded TPS: {:.1f}\nFile: {}.illmacro",
        cfg.macroMode == MacroMode::Record ? "RECORDING" : (cfg.macroMode == MacroMode::Play ? "PLAYING" : "IDLE"),
        macro.getActionCount(),
        macro.m_recordedTPS,
        macro.m_lastLoadedName
    );

    auto statusLbl = CCLabelBMFont::create(statusText.c_str(), "chatFont.fnt");
    statusLbl->setScale(0.75f);
    statusLbl->setAnchorPoint({0.0f, 1.0f});
    statusLbl->setPosition({20.0f, 160.0f});
    m_contentContainer->addChild(statusLbl);

    // Clear Macro Button
    auto clearSpr = ButtonSprite::create("Clear Macro", "bigFont.fnt", "GJ_button_06.png", 0.6f);
    clearSpr->setScale(0.6f);
    auto clearBtn = CCMenuItemSpriteExtra::create(clearSpr, this, menu_selector(SilicateGUI::onMacroClear));
    clearBtn->setPosition({160.0f, 50.0f});
    menu->addChild(clearBtn);
}

// =============================================================================
// TAB 2: AUTOCLICKER (P1 & P2 INDEPENDENT)
// =============================================================================
void SilicateGUI::buildAutoclickerTab() {
    auto title = CCLabelBMFont::create("Autoclicker", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 320.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& cfg = Config::get();

    // ---------------- Player 1 ----------------
    auto p1Toggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleP1Autoclicker), 0.65f
    );
    p1Toggle->toggle(cfg.p1Autoclicker);
    p1Toggle->setPosition({30.0f, 275.0f});
    menu->addChild(p1Toggle);

    auto p1Title = CCLabelBMFont::create("Player 1 Autoclicker", "bigFont.fnt");
    p1Title->setScale(0.38f);
    p1Title->setAnchorPoint({0.0f, 0.5f});
    p1Title->setPosition({52.0f, 275.0f});
    m_contentContainer->addChild(p1Title);

    // P1 CPS Buttons [-] [ 60.0 CPS ] [+]
    auto p1MinusSpr = ButtonSprite::create("-", "goldFont.fnt", "GJ_button_01.png", 0.6f);
    p1MinusSpr->setScale(0.65f);
    auto p1Minus = CCMenuItemSpriteExtra::create(p1MinusSpr, this, menu_selector(SilicateGUI::onP1CPSMinus));
    p1Minus->setPosition({60.0f, 240.0f});
    menu->addChild(p1Minus);

    auto p1CPSLbl = CCLabelBMFont::create(fmt::format("{:.1f} CPS", cfg.p1CPS).c_str(), "bigFont.fnt");
    p1CPSLbl->setScale(0.36f);
    p1CPSLbl->setPosition({130.0f, 240.0f});
    m_contentContainer->addChild(p1CPSLbl);

    auto p1PlusSpr = ButtonSprite::create("+", "goldFont.fnt", "GJ_button_01.png", 0.6f);
    p1PlusSpr->setScale(0.65f);
    auto p1Plus = CCMenuItemSpriteExtra::create(p1PlusSpr, this, menu_selector(SilicateGUI::onP1CPSPlus));
    p1Plus->setPosition({200.0f, 240.0f});
    menu->addChild(p1Plus);

    // P1 Hold / Spam Mode
    auto p1ModeSpr = ButtonSprite::create(cfg.p1HoldMode ? "Hold Click" : "Spam Always", "chatFont.fnt", "GJ_button_04.png", 0.6f);
    p1ModeSpr->setScale(0.68f);
    auto p1ModeBtn = CCMenuItemSpriteExtra::create(p1ModeSpr, this, menu_selector(SilicateGUI::onP1ToggleHold));
    p1ModeBtn->setPosition({285.0f, 240.0f});
    menu->addChild(p1ModeBtn);

    // P1 Presets: 15, 30, 60, 120, 240
    float presets[] = {15.0f, 30.0f, 60.0f, 120.0f, 240.0f};
    for (int i = 0; i < 5; ++i) {
        auto spr = ButtonSprite::create(fmt::format("{:g}", presets[i]).c_str(), "chatFont.fnt", "GJ_button_02.png", 0.5f);
        spr->setScale(0.6f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SilicateGUI::onP1Preset));
        btn->setTag(static_cast<int>(presets[i]));
        btn->setPosition({40.0f + (i * 55.0f), 205.0f});
        menu->addChild(btn);
    }

    // ---------------- Player 2 ----------------
    auto p2Toggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleP2Autoclicker), 0.65f
    );
    p2Toggle->toggle(cfg.p2Autoclicker);
    p2Toggle->setPosition({30.0f, 145.0f});
    menu->addChild(p2Toggle);

    auto p2Title = CCLabelBMFont::create("Player 2 Autoclicker (Dual)", "bigFont.fnt");
    p2Title->setScale(0.38f);
    p2Title->setAnchorPoint({0.0f, 0.5f});
    p2Title->setPosition({52.0f, 145.0f});
    m_contentContainer->addChild(p2Title);

    // P2 CPS Buttons [-] [ 60.0 CPS ] [+]
    auto p2MinusSpr = ButtonSprite::create("-", "goldFont.fnt", "GJ_button_01.png", 0.6f);
    p2MinusSpr->setScale(0.65f);
    auto p2Minus = CCMenuItemSpriteExtra::create(p2MinusSpr, this, menu_selector(SilicateGUI::onP2CPSMinus));
    p2Minus->setPosition({60.0f, 110.0f});
    menu->addChild(p2Minus);

    auto p2CPSLbl = CCLabelBMFont::create(fmt::format("{:.1f} CPS", cfg.p2CPS).c_str(), "bigFont.fnt");
    p2CPSLbl->setScale(0.36f);
    p2CPSLbl->setPosition({130.0f, 110.0f});
    m_contentContainer->addChild(p2CPSLbl);

    auto p2PlusSpr = ButtonSprite::create("+", "goldFont.fnt", "GJ_button_01.png", 0.6f);
    p2PlusSpr->setScale(0.65f);
    auto p2Plus = CCMenuItemSpriteExtra::create(p2PlusSpr, this, menu_selector(SilicateGUI::onP2CPSPlus));
    p2Plus->setPosition({200.0f, 110.0f});
    menu->addChild(p2Plus);

    // P2 Hold / Spam Mode
    auto p2ModeSpr = ButtonSprite::create(cfg.p2HoldMode ? "Hold Click" : "Spam Always", "chatFont.fnt", "GJ_button_04.png", 0.6f);
    p2ModeSpr->setScale(0.68f);
    auto p2ModeBtn = CCMenuItemSpriteExtra::create(p2ModeSpr, this, menu_selector(SilicateGUI::onP2ToggleHold));
    p2ModeBtn->setPosition({285.0f, 110.0f});
    menu->addChild(p2ModeBtn);

    // P2 Presets
    for (int i = 0; i < 5; ++i) {
        auto spr = ButtonSprite::create(fmt::format("{:g}", presets[i]).c_str(), "chatFont.fnt", "GJ_button_02.png", 0.5f);
        spr->setScale(0.6f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SilicateGUI::onP2Preset));
        btn->setTag(static_cast<int>(presets[i]));
        btn->setPosition({40.0f + (i * 55.0f), 75.0f});
        menu->addChild(btn);
    }
}

// =============================================================================
// TAB 3: PHYSICS & TPS
// =============================================================================
void SilicateGUI::buildPhysicsTPSTab() {
    auto title = CCLabelBMFont::create("Physics / TPS Changer", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 320.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& cfg = Config::get();

    // TPS Toggle
    auto tpsToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleTPS), 0.65f
    );
    tpsToggle->toggle(cfg.tpsEnabled);
    tpsToggle->setPosition({30.0f, 275.0f});
    menu->addChild(tpsToggle);

    auto tpsTitle = CCLabelBMFont::create("Custom TPS Bypass", "bigFont.fnt");
    tpsTitle->setScale(0.38f);
    tpsTitle->setAnchorPoint({0.0f, 0.5f});
    tpsTitle->setPosition({52.0f, 275.0f});
    m_contentContainer->addChild(tpsTitle);

    // TPS Adjuster [-] [ 240 TPS ] [+]
    auto tpsMinusSpr = ButtonSprite::create("-", "goldFont.fnt", "GJ_button_01.png", 0.6f);
    tpsMinusSpr->setScale(0.65f);
    auto tpsMinus = CCMenuItemSpriteExtra::create(tpsMinusSpr, this, menu_selector(SilicateGUI::onTPSMinus));
    tpsMinus->setPosition({60.0f, 240.0f});
    menu->addChild(tpsMinus);

    auto tpsValLbl = CCLabelBMFont::create(fmt::format("{:g} TPS", cfg.targetTPS).c_str(), "bigFont.fnt");
    tpsValLbl->setScale(0.38f);
    tpsValLbl->setPosition({140.0f, 240.0f});
    m_contentContainer->addChild(tpsValLbl);

    auto tpsPlusSpr = ButtonSprite::create("+", "goldFont.fnt", "GJ_button_01.png", 0.6f);
    tpsPlusSpr->setScale(0.65f);
    auto tpsPlus = CCMenuItemSpriteExtra::create(tpsPlusSpr, this, menu_selector(SilicateGUI::onTPSPlus));
    tpsPlus->setPosition({220.0f, 240.0f});
    menu->addChild(tpsPlus);

    // TPS Presets: 240, 360, 480, 1000, 2000, 50000
    int tpsPresets[] = {240, 360, 480, 1000, 2000, 50000};
    for (int i = 0; i < 6; ++i) {
        auto spr = ButtonSprite::create(fmt::format("{}", tpsPresets[i]).c_str(), "chatFont.fnt", "GJ_button_02.png", 0.5f);
        spr->setScale(0.6f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SilicateGUI::onTPSPreset));
        btn->setTag(tpsPresets[i]);
        btn->setPosition({30.0f + (i * 48.0f), 205.0f});
        menu->addChild(btn);
    }

    // Speedhack Section
    auto spdTitle = CCLabelBMFont::create("Speedhack", "bigFont.fnt");
    spdTitle->setScale(0.40f);
    spdTitle->setAnchorPoint({0.0f, 0.5f});
    spdTitle->setPosition({30.0f, 150.0f});
    m_contentContainer->addChild(spdTitle);

    auto spdMinusSpr = ButtonSprite::create("-", "goldFont.fnt", "GJ_button_01.png", 0.6f);
    spdMinusSpr->setScale(0.65f);
    auto spdMinus = CCMenuItemSpriteExtra::create(spdMinusSpr, this, menu_selector(SilicateGUI::onSpeedMinus));
    spdMinus->setPosition({60.0f, 115.0f});
    menu->addChild(spdMinus);

    auto spdValLbl = CCLabelBMFont::create(fmt::format("{:.2f}x", cfg.speedhackValue).c_str(), "bigFont.fnt");
    spdValLbl->setScale(0.38f);
    spdValLbl->setPosition({140.0f, 115.0f});
    m_contentContainer->addChild(spdValLbl);

    auto spdPlusSpr = ButtonSprite::create("+", "goldFont.fnt", "GJ_button_01.png", 0.6f);
    spdPlusSpr->setScale(0.65f);
    auto spdPlus = CCMenuItemSpriteExtra::create(spdPlusSpr, this, menu_selector(SilicateGUI::onSpeedPlus));
    spdPlus->setPosition({220.0f, 115.0f});
    menu->addChild(spdPlus);

    // Speed Presets: 0.25, 0.5, 0.75, 1.0
    float speedPresets[] = {0.25f, 0.50f, 0.75f, 1.00f};
    for (int i = 0; i < 4; ++i) {
        auto spr = ButtonSprite::create(fmt::format("{:.2g}x", speedPresets[i]).c_str(), "chatFont.fnt", "GJ_button_02.png", 0.5f);
        spr->setScale(0.65f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SilicateGUI::onSpeedPreset));
        btn->setTag(static_cast<int>(speedPresets[i] * 100));
        btn->setPosition({45.0f + (i * 65.0f), 75.0f});
        menu->addChild(btn);
    }
}

// =============================================================================
// TAB 4: TRAJECTORY
// =============================================================================
void SilicateGUI::buildTrajectoryTab() {
    auto title = CCLabelBMFont::create("Trajectory", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 320.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& cfg = Config::get();

    auto trajToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleTrajectory), 0.7f
    );
    trajToggle->toggle(cfg.showTrajectory);
    trajToggle->setPosition({35.0f, 260.0f});
    menu->addChild(trajToggle);

    auto trajLbl = CCLabelBMFont::create("Show Trajectory & Jump Arc", "bigFont.fnt");
    trajLbl->setScale(0.42f);
    trajLbl->setAnchorPoint({0.0f, 0.5f});
    trajLbl->setPosition({60.0f, 260.0f});
    m_contentContainer->addChild(trajLbl);

    std::string desc = 
        "- Calculates mathematical parabolic jump paths\n"
        "- Predicts Wave 45-degree and Ship flight curves\n"
        "- Real-time hazard detection with Pink Death Box markers\n"
        "- Supports gravity inversion and dual mode";

    auto descLbl = CCLabelBMFont::create(desc.c_str(), "chatFont.fnt");
    descLbl->setScale(0.85f);
    descLbl->setAnchorPoint({0.0f, 1.0f});
    descLbl->setPosition({35.0f, 210.0f});
    m_contentContainer->addChild(descLbl);
}

// =============================================================================
// TAB 5: HITBOXES
// =============================================================================
void SilicateGUI::buildHitboxesTab() {
    auto title = CCLabelBMFont::create("Hitboxes", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 320.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& cfg = Config::get();

    auto hitToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleHitboxes), 0.7f
    );
    hitToggle->toggle(cfg.showHitboxes);
    hitToggle->setPosition({35.0f, 260.0f});
    menu->addChild(hitToggle);

    auto hitLbl = CCLabelBMFont::create("Show Hitbox Wireframes", "bigFont.fnt");
    hitLbl->setScale(0.42f);
    hitLbl->setAnchorPoint({0.0f, 0.5f});
    hitLbl->setPosition({60.0f, 260.0f});
    m_contentContainer->addChild(hitLbl);

    std::string desc =
        "- Player 1 & Player 2 Inner and Outer hitboxes\n"
        "- Hazard hitboxes (Spikes, Saws) in Red\n"
        "- Solid blocks and platforms in Blue\n"
        "- High-performance GPU wireframe rendering";

    auto descLbl = CCLabelBMFont::create(desc.c_str(), "chatFont.fnt");
    descLbl->setScale(0.85f);
    descLbl->setAnchorPoint({0.0f, 1.0f});
    descLbl->setPosition({35.0f, 210.0f});
    m_contentContainer->addChild(descLbl);
}

// =============================================================================
// TAB 6: SETTINGS & KEYBINDS
// =============================================================================
void SilicateGUI::buildSettingsTab() {
    auto title = CCLabelBMFont::create("Settings & Keybinds", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 320.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& cfg = Config::get();

    // Noclip
    auto noclipToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleNoclip), 0.65f
    );
    noclipToggle->toggle(cfg.noclip);
    noclipToggle->setPosition({30.0f, 265.0f});
    menu->addChild(noclipToggle);

    auto noclipLbl = CCLabelBMFont::create("Noclip (Invincibility)", "bigFont.fnt");
    noclipLbl->setScale(0.38f);
    noclipLbl->setAnchorPoint({0.0f, 0.5f});
    noclipLbl->setPosition({52.0f, 265.0f});
    m_contentContainer->addChild(noclipLbl);

    // Safe Mode
    auto safeToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleSafeMode), 0.65f
    );
    safeToggle->toggle(cfg.safeMode);
    safeToggle->setPosition({30.0f, 215.0f});
    menu->addChild(safeToggle);

    auto safeLbl = CCLabelBMFont::create("Safe Mode (Disable Leaderboard Submissions)", "bigFont.fnt");
    safeLbl->setScale(0.36f);
    safeLbl->setAnchorPoint({0.0f, 0.5f});
    safeLbl->setPosition({52.0f, 215.0f});
    m_contentContainer->addChild(safeLbl);

    // Keybinds guide
    std::string keys = 
        "Keybinds:\n"
        "[Tab] / [Left Alt] / [F] : Toggle Silicate Menu\n"
        "\nILLBot Suite v1.0.0 - Built for Impossible Levels (ILL)\n"
        "Created with Geode SDK";

    auto keysLbl = CCLabelBMFont::create(keys.c_str(), "chatFont.fnt");
    keysLbl->setScale(0.85f);
    keysLbl->setAnchorPoint({0.0f, 1.0f});
    keysLbl->setPosition({20.0f, 165.0f});
    m_contentContainer->addChild(keysLbl);
}

// =============================================================================
// CALLBACKS IMPLEMENTATION
// =============================================================================
void SilicateGUI::onToggleP1Autoclicker(CCObject*) {
    Config::get().p1Autoclicker = !Config::get().p1Autoclicker;
    Config::get().saveToSettings();
}

void SilicateGUI::onP1CPSMinus(CCObject*) {
    auto& cfg = Config::get();
    cfg.p1CPS = std::max(1.0f, cfg.p1CPS - (cfg.p1CPS > 30.0f ? 5.0f : 1.0f));
    cfg.saveToSettings();
    switchTab(TabID::Autoclicker);
}

void SilicateGUI::onP1CPSPlus(CCObject*) {
    auto& cfg = Config::get();
    cfg.p1CPS = std::min(2000.0f, cfg.p1CPS + (cfg.p1CPS >= 30.0f ? 5.0f : 1.0f));
    cfg.saveToSettings();
    switchTab(TabID::Autoclicker);
}

void SilicateGUI::onP1Preset(CCObject* sender) {
    if (auto node = typeinfo_cast<CCNode*>(sender)) {
        Config::get().p1CPS = static_cast<float>(node->getTag());
        Config::get().saveToSettings();
        switchTab(TabID::Autoclicker);
    }
}

void SilicateGUI::onP1ToggleHold(CCObject*) {
    Config::get().p1HoldMode = !Config::get().p1HoldMode;
    Config::get().saveToSettings();
    switchTab(TabID::Autoclicker);
}

void SilicateGUI::onToggleP2Autoclicker(CCObject*) {
    Config::get().p2Autoclicker = !Config::get().p2Autoclicker;
    Config::get().saveToSettings();
}

void SilicateGUI::onP2CPSMinus(CCObject*) {
    auto& cfg = Config::get();
    cfg.p2CPS = std::max(1.0f, cfg.p2CPS - (cfg.p2CPS > 30.0f ? 5.0f : 1.0f));
    cfg.saveToSettings();
    switchTab(TabID::Autoclicker);
}

void SilicateGUI::onP2CPSPlus(CCObject*) {
    auto& cfg = Config::get();
    cfg.p2CPS = std::min(2000.0f, cfg.p2CPS + (cfg.p2CPS >= 30.0f ? 5.0f : 1.0f));
    cfg.saveToSettings();
    switchTab(TabID::Autoclicker);
}

void SilicateGUI::onP2Preset(CCObject* sender) {
    if (auto node = typeinfo_cast<CCNode*>(sender)) {
        Config::get().p2CPS = static_cast<float>(node->getTag());
        Config::get().saveToSettings();
        switchTab(TabID::Autoclicker);
    }
}

void SilicateGUI::onP2ToggleHold(CCObject*) {
    Config::get().p2HoldMode = !Config::get().p2HoldMode;
    Config::get().saveToSettings();
    switchTab(TabID::Autoclicker);
}

void SilicateGUI::onToggleTPS(CCObject*) {
    Config::get().tpsEnabled = !Config::get().tpsEnabled;
    TPSManager::get().applyTPS();
    Config::get().saveToSettings();
}

void SilicateGUI::onTPSMinus(CCObject*) {
    auto& cfg = Config::get();
    cfg.targetTPS = std::max(30.0f, cfg.targetTPS - 60.0f);
    TPSManager::get().applyTPS();
    cfg.saveToSettings();
    switchTab(TabID::PhysicsTPS);
}

void SilicateGUI::onTPSPlus(CCObject*) {
    auto& cfg = Config::get();
    cfg.targetTPS = std::min(50000.0f, cfg.targetTPS + 60.0f);
    TPSManager::get().applyTPS();
    cfg.saveToSettings();
    switchTab(TabID::PhysicsTPS);
}

void SilicateGUI::onTPSPreset(CCObject* sender) {
    if (auto node = typeinfo_cast<CCNode*>(sender)) {
        Config::get().targetTPS = static_cast<float>(node->getTag());
        TPSManager::get().applyTPS();
        Config::get().saveToSettings();
        switchTab(TabID::PhysicsTPS);
    }
}

void SilicateGUI::onSpeedMinus(CCObject*) {
    auto& cfg = Config::get();
    cfg.speedhackValue = std::max(0.05f, cfg.speedhackValue - 0.1f);
    cfg.speedhackEnabled = (cfg.speedhackValue != 1.0f);
    cfg.saveToSettings();
    switchTab(TabID::PhysicsTPS);
}

void SilicateGUI::onSpeedPlus(CCObject*) {
    auto& cfg = Config::get();
    cfg.speedhackValue = std::min(5.0f, cfg.speedhackValue + 0.1f);
    cfg.speedhackEnabled = (cfg.speedhackValue != 1.0f);
    cfg.saveToSettings();
    switchTab(TabID::PhysicsTPS);
}

void SilicateGUI::onSpeedPreset(CCObject* sender) {
    if (auto node = typeinfo_cast<CCNode*>(sender)) {
        Config::get().speedhackValue = static_cast<float>(node->getTag()) / 100.0f;
        Config::get().speedhackEnabled = (Config::get().speedhackValue != 1.0f);
        Config::get().saveToSettings();
        switchTab(TabID::PhysicsTPS);
    }
}

void SilicateGUI::onMacroOff(CCObject*) {
    Config::get().macroMode = MacroMode::Off;
    switchTab(TabID::Record);
}

void SilicateGUI::onMacroRecord(CCObject*) {
    Config::get().macroMode = MacroMode::Record;
    switchTab(TabID::Record);
}

void SilicateGUI::onMacroPlay(CCObject*) {
    Config::get().macroMode = MacroMode::Play;
    switchTab(TabID::Record);
}

void SilicateGUI::onMacroSave(CCObject*) {
    MacroManager::get().saveToFile(Config::get().replayName);
    switchTab(TabID::Record);
}

void SilicateGUI::onMacroLoad(CCObject*) {
    MacroManager::get().loadFromFile(Config::get().replayName);
    switchTab(TabID::Record);
}

void SilicateGUI::onMacroClear(CCObject*) {
    MacroManager::get().clear();
    switchTab(TabID::Record);
}

void SilicateGUI::onToggleTrajectory(CCObject*) {
    Config::get().showTrajectory = !Config::get().showTrajectory;
    Config::get().saveToSettings();
}

void SilicateGUI::onToggleHitboxes(CCObject*) {
    Config::get().showHitboxes = !Config::get().showHitboxes;
    Config::get().saveToSettings();
}

void SilicateGUI::onToggleNoclip(CCObject*) {
    Config::get().noclip = !Config::get().noclip;
    Config::get().saveToSettings();
}

void SilicateGUI::onToggleSafeMode(CCObject*) {
    Config::get().safeMode = !Config::get().safeMode;
    Config::get().saveToSettings();
}
