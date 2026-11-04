#include "SilicateGUI.hpp"
#include "../Autoclicker.hpp"
#include "../TPSManager.hpp"
#include "../TrajectoryVisualizer.hpp"
#include "../HitboxVisualizer.hpp"
#include <algorithm>
#include <fmt/format.h>

using namespace geode::prelude;

SilicateGUI* SilicateGUI::create() {
    auto ret = new SilicateGUI();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool SilicateGUI::init() {
    if (!CCLayer::init()) return false;

    s_instance = this;

    const float WIN_WIDTH = 450.0f;
    const float WIN_HEIGHT = 280.0f;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // Determine initial position (saved or centered)
    float posX = (Config::get().guiPosX >= 0.0f) ? Config::get().guiPosX : (winSize.width - WIN_WIDTH) * 0.5f;
    float posY = (Config::get().guiPosY >= 0.0f) ? Config::get().guiPosY : (winSize.height - WIN_HEIGHT) * 0.5f;

    posX = std::clamp(posX, 0.0f, std::max(0.0f, winSize.width - WIN_WIDTH));
    posY = std::clamp(posY, 0.0f, std::max(0.0f, winSize.height - WIN_HEIGHT));

    // 1. Root floating window container node
    m_windowNode = CCNode::create();
    m_windowNode->setContentSize({WIN_WIDTH, WIN_HEIGHT});
    m_windowNode->setPosition({posX, posY});
    this->addChild(m_windowNode);

    // 2. Custom Frosted Dark Ruby / Wine Acrylic Background
    m_backgroundNode = CCDrawNode::create();
    if (m_backgroundNode) {
        CCPoint bgVerts[4] = {
            CCPoint(0.0f, 0.0f),
            CCPoint(WIN_WIDTH, 0.0f),
            CCPoint(WIN_WIDTH, WIN_HEIGHT),
            CCPoint(0.0f, WIN_HEIGHT)
        };
        // Deep wine/crimson acrylic fill
        ccColor4F fillColor = {0.08f, 0.02f, 0.04f, 0.96f};
        ccColor4F strokeColor = {0.42f, 0.11f, 0.19f, 0.95f};
        m_backgroundNode->drawPolygon(bgVerts, 4, fillColor, 1.8f, strokeColor);

        // Header Title Bar drag area background (y: 248 to 280)
        CCPoint headerVerts[4] = {
            CCPoint(0.0f, 248.0f),
            CCPoint(WIN_WIDTH, 248.0f),
            CCPoint(WIN_WIDTH, WIN_HEIGHT),
            CCPoint(0.0f, WIN_HEIGHT)
        };
        m_backgroundNode->drawPolygon(headerVerts, 4, {0.18f, 0.05f, 0.09f, 0.90f}, 0.0f, {0, 0, 0, 0});

        // Header Separator line
        CCPoint headerLine[2] = { CCPoint(0.0f, 248.0f), CCPoint(WIN_WIDTH, 248.0f) };
        m_backgroundNode->drawSegment(headerLine[0], headerLine[1], 1.0f, {0.35f, 0.09f, 0.16f, 0.85f});

        // Sidebar Divider vertical line (x: 120)
        CCPoint dividerVerts[2] = { CCPoint(120.0f, 0.0f), CCPoint(120.0f, 248.0f) };
        m_backgroundNode->drawSegment(dividerVerts[0], dividerVerts[1], 1.0f, {0.26f, 0.06f, 0.12f, 0.85f});

        m_windowNode->addChild(m_backgroundNode, -1);
    }

    // 3. Header Drag Bar Elements: Logo, Subtitle, Drag Hint, Close Button
    auto logoTitle = CCLabelBMFont::create("Silicate", "bigFont.fnt");
    logoTitle->setScale(0.42f);
    logoTitle->setAnchorPoint({0.0f, 0.5f});
    logoTitle->setPosition({12.0f, 264.0f});
    m_windowNode->addChild(logoTitle);

    auto subtitle = CCLabelBMFont::create("ILL BOT", "goldFont.fnt");
    subtitle->setScale(0.24f);
    subtitle->setAnchorPoint({0.0f, 0.5f});
    subtitle->setPosition({74.0f, 264.0f});
    m_windowNode->addChild(subtitle);

    auto dragHint = CCLabelBMFont::create(":: DRAG HERE ::", "chatFont.fnt");
    dragHint->setScale(0.55f);
    dragHint->setColor({160, 120, 135});
    dragHint->setPosition({235.0f, 264.0f});
    m_windowNode->addChild(dragHint);

    // Header Close Button
    auto headerMenu = CCMenu::create();
    headerMenu->setPosition({0.0f, 0.0f});
    m_windowNode->addChild(headerMenu, 10);

    auto closeSpr = ButtonSprite::create("X", "bigFont.fnt", "GJ_button_06.png", 0.5f);
    closeSpr->setScale(0.55f);
    auto closeBtn = CCMenuItemSpriteExtra::create(closeSpr, this, menu_selector(SilicateGUI::onClose));
    closeBtn->setPosition({WIN_WIDTH - 20.0f, 264.0f});
    headerMenu->addChild(closeBtn);

    // 4. Sidebar Navigation Menu
    m_sidebarMenu = CCMenu::create();
    m_sidebarMenu->setPosition({0.0f, 0.0f});
    m_windowNode->addChild(m_sidebarMenu);

    updateSidebarButtons();

    // 5. Content Area Node
    m_contentContainer = CCNode::create();
    m_contentContainer->setPosition({125.0f, 5.0f});
    m_windowNode->addChild(m_contentContainer);

    switchTab(TabID::Record);

    this->setTouchEnabled(true);
    return true;
}

void SilicateGUI::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -500, true);
}

bool SilicateGUI::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!m_windowNode || !this->isVisible()) return false;

    CCPoint touchPos = touch->getLocation();
    CCPoint localPos = m_windowNode->convertToNodeSpace(touchPos);

    const float WIN_WIDTH = 450.0f;
    const float WIN_HEIGHT = 280.0f;

    // Check if touch is within the top drag bar (excluding close button area on the right)
    if (localPos.x >= 0.0f && localPos.x <= (WIN_WIDTH - 45.0f) && localPos.y >= 245.0f && localPos.y <= WIN_HEIGHT) {
        m_isDragging = true;
        m_dragOffset = ccpSub(m_windowNode->getPosition(), touchPos);
        return true; // Claim touch to handle window dragging
    }

    // If touch is within window bounds (sidebar, content, close button), allow child CCMenu buttons to receive it
    if (localPos.x >= 0.0f && localPos.x <= WIN_WIDTH && localPos.y >= 0.0f && localPos.y <= WIN_HEIGHT) {
        return false;
    }

    // Touch outside window: do not swallow so game / pause buttons can still be touched
    return false;
}

void SilicateGUI::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    if (m_isDragging && m_windowNode) {
        CCPoint currentTouch = touch->getLocation();
        CCPoint newPos = ccpAdd(currentTouch, m_dragOffset);

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        const float WIN_WIDTH = 450.0f;
        const float WIN_HEIGHT = 280.0f;

        newPos.x = std::clamp(newPos.x, 0.0f, std::max(0.0f, winSize.width - WIN_WIDTH));
        newPos.y = std::clamp(newPos.y, 0.0f, std::max(0.0f, winSize.height - WIN_HEIGHT));

        m_windowNode->setPosition(newPos);
        Config::get().guiPosX = newPos.x;
        Config::get().guiPosY = newPos.y;
    }
}

void SilicateGUI::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    if (m_isDragging) {
        m_isDragging = false;
        Config::get().saveToSettings();
    }
}

void SilicateGUI::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
    m_isDragging = false;
}

void SilicateGUI::close() {
    s_instance = nullptr;
    this->removeFromParentAndCleanup(true);
}

void SilicateGUI::onClose(CCObject*) {
    close();
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
        "Keybinds"
    };

    float startY = 222.0f;
    float stepY = 36.0f;

    for (int i = 0; i < 6; ++i) {
        bool isActive = (static_cast<int>(m_currentTab) == i);

        const char* bgSpriteName = isActive ? "GJ_button_01.png" : "GJ_button_04.png";
        const char* fontName = isActive ? "bigFont.fnt" : "chatFont.fnt";

        auto btnSpr = ButtonSprite::create(tabNames[i], fontName, bgSpriteName, 0.58f);
        btnSpr->setScale(0.60f);
        if (!isActive) {
            btnSpr->setColor({170, 160, 175});
        }

        auto btn = CCMenuItemSpriteExtra::create(
            btnSpr, this, menu_selector(SilicateGUI::onTabClicked)
        );
        btn->setTag(i);
        btn->setPosition({60.0f, startY - (i * stepY)});
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
        case TabID::Keybinds: buildKeybindsTab(); break;
    }
}

// =============================================================================
// TAB 1: RECORD & MACRO
// =============================================================================
void SilicateGUI::buildRecordTab() {
    auto title = CCLabelBMFont::create("Record & Macro", "bigFont.fnt");
    title->setScale(0.44f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 240.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& macro = MacroManager::get();
    auto& cfg = Config::get();

    // Mode Selector: [Off] [Record] [Play]
    auto offSpr = ButtonSprite::create("Off", "bigFont.fnt", cfg.macroMode == MacroMode::Off ? "GJ_button_01.png" : "GJ_button_04.png", 0.55f);
    offSpr->setScale(0.55f);
    auto offBtn = CCMenuItemSpriteExtra::create(offSpr, this, menu_selector(SilicateGUI::onMacroOff));
    offBtn->setPosition({50.0f, 195.0f});
    menu->addChild(offBtn);

    auto recSpr = ButtonSprite::create("Record", "bigFont.fnt", cfg.macroMode == MacroMode::Record ? "GJ_button_01.png" : "GJ_button_04.png", 0.55f);
    recSpr->setScale(0.55f);
    auto recBtn = CCMenuItemSpriteExtra::create(recSpr, this, menu_selector(SilicateGUI::onMacroRecord));
    recBtn->setPosition({135.0f, 195.0f});
    menu->addChild(recBtn);

    auto playSpr = ButtonSprite::create("Play", "bigFont.fnt", cfg.macroMode == MacroMode::Play ? "GJ_button_01.png" : "GJ_button_04.png", 0.55f);
    playSpr->setScale(0.55f);
    auto playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(SilicateGUI::onMacroPlay));
    playBtn->setPosition({220.0f, 195.0f});
    menu->addChild(playBtn);

    // Save & Load Replay
    auto loadSpr = ButtonSprite::create("Load Replay", "goldFont.fnt", "GJ_button_02.png", 0.55f);
    loadSpr->setScale(0.60f);
    auto loadBtn = CCMenuItemSpriteExtra::create(loadSpr, this, menu_selector(SilicateGUI::onMacroLoad));
    loadBtn->setPosition({80.0f, 150.0f});
    menu->addChild(loadBtn);

    auto saveSpr = ButtonSprite::create("Save Replay", "goldFont.fnt", "GJ_button_02.png", 0.55f);
    saveSpr->setScale(0.60f);
    auto saveBtn = CCMenuItemSpriteExtra::create(saveSpr, this, menu_selector(SilicateGUI::onMacroSave));
    saveBtn->setPosition({190.0f, 150.0f});
    menu->addChild(saveBtn);

    // Status Label
    std::string statusText = fmt::format(
        "Status: {}\nActions: {}\nRecorded TPS: {:.1f}\nFile: {}.illmacro",
        cfg.macroMode == MacroMode::Record ? "RECORDING" : (cfg.macroMode == MacroMode::Play ? "PLAYING" : "IDLE"),
        macro.getActionCount(),
        macro.m_recordedTPS,
        macro.m_lastLoadedName
    );

    auto statusLbl = CCLabelBMFont::create(statusText.c_str(), "chatFont.fnt");
    statusLbl->setScale(0.68f);
    statusLbl->setAnchorPoint({0.0f, 1.0f});
    statusLbl->setPosition({15.0f, 115.0f});
    m_contentContainer->addChild(statusLbl);

    // Clear Button
    auto clearSpr = ButtonSprite::create("Clear Macro", "bigFont.fnt", "GJ_button_06.png", 0.55f);
    clearSpr->setScale(0.50f);
    auto clearBtn = CCMenuItemSpriteExtra::create(clearSpr, this, menu_selector(SilicateGUI::onMacroClear));
    clearBtn->setPosition({135.0f, 25.0f});
    menu->addChild(clearBtn);
}

// =============================================================================
// TAB 2: AUTOCLICKER (P1 & P2 INDEPENDENT)
// =============================================================================
void SilicateGUI::buildAutoclickerTab() {
    auto title = CCLabelBMFont::create("Autoclicker (P1 & P2)", "bigFont.fnt");
    title->setScale(0.44f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 240.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& cfg = Config::get();

    // ---------------- Player 1 ----------------
    auto p1Toggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleP1Autoclicker), 0.55f
    );
    p1Toggle->toggle(cfg.p1Autoclicker);
    p1Toggle->setPosition({20.0f, 205.0f});
    menu->addChild(p1Toggle);

    auto p1Title = CCLabelBMFont::create("Player 1 Autoclicker", "bigFont.fnt");
    p1Title->setScale(0.32f);
    p1Title->setAnchorPoint({0.0f, 0.5f});
    p1Title->setPosition({38.0f, 205.0f});
    m_contentContainer->addChild(p1Title);

    // P1 CPS Buttons [-] [ 60.0 CPS ] [+]
    auto p1MinusSpr = ButtonSprite::create("-", "goldFont.fnt", "GJ_button_01.png", 0.55f);
    p1MinusSpr->setScale(0.55f);
    auto p1Minus = CCMenuItemSpriteExtra::create(p1MinusSpr, this, menu_selector(SilicateGUI::onP1CPSMinus));
    p1Minus->setPosition({35.0f, 175.0f});
    menu->addChild(p1Minus);

    auto p1CPSLbl = CCLabelBMFont::create(fmt::format("{:.1f} CPS", cfg.p1CPS).c_str(), "bigFont.fnt");
    p1CPSLbl->setScale(0.30f);
    p1CPSLbl->setPosition({95.0f, 175.0f});
    m_contentContainer->addChild(p1CPSLbl);

    auto p1PlusSpr = ButtonSprite::create("+", "goldFont.fnt", "GJ_button_01.png", 0.55f);
    p1PlusSpr->setScale(0.55f);
    auto p1Plus = CCMenuItemSpriteExtra::create(p1PlusSpr, this, menu_selector(SilicateGUI::onP1CPSPlus));
    p1Plus->setPosition({155.0f, 175.0f});
    menu->addChild(p1Plus);

    // P1 Mode: Hold / Spam
    auto p1ModeSpr = ButtonSprite::create(cfg.p1HoldMode ? "Hold Only" : "Spam Always", "chatFont.fnt", "GJ_button_04.png", 0.55f);
    p1ModeSpr->setScale(0.58f);
    auto p1ModeBtn = CCMenuItemSpriteExtra::create(p1ModeSpr, this, menu_selector(SilicateGUI::onP1ToggleHold));
    p1ModeBtn->setPosition({235.0f, 175.0f});
    menu->addChild(p1ModeBtn);

    // P1 Presets: 15, 30, 60, 120, 240
    float presets[] = {15.0f, 30.0f, 60.0f, 120.0f, 240.0f};
    for (int i = 0; i < 5; ++i) {
        auto spr = ButtonSprite::create(fmt::format("{:g}", presets[i]).c_str(), "chatFont.fnt", "GJ_button_02.png", 0.5f);
        spr->setScale(0.52f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SilicateGUI::onP1Preset));
        btn->setTag(static_cast<int>(presets[i]));
        btn->setPosition({28.0f + (i * 44.0f), 145.0f});
        menu->addChild(btn);
    }

    // ---------------- Player 2 ----------------
    auto p2Toggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleP2Autoclicker), 0.55f
    );
    p2Toggle->toggle(cfg.p2Autoclicker);
    p2Toggle->setPosition({20.0f, 95.0f});
    menu->addChild(p2Toggle);

    auto p2Title = CCLabelBMFont::create("Player 2 Autoclicker (Dual)", "bigFont.fnt");
    p2Title->setScale(0.32f);
    p2Title->setAnchorPoint({0.0f, 0.5f});
    p2Title->setPosition({38.0f, 95.0f});
    m_contentContainer->addChild(p2Title);

    // P2 CPS Buttons [-] [ 60.0 CPS ] [+]
    auto p2MinusSpr = ButtonSprite::create("-", "goldFont.fnt", "GJ_button_01.png", 0.55f);
    p2MinusSpr->setScale(0.55f);
    auto p2Minus = CCMenuItemSpriteExtra::create(p2MinusSpr, this, menu_selector(SilicateGUI::onP2CPSMinus));
    p2Minus->setPosition({35.0f, 65.0f});
    menu->addChild(p2Minus);

    auto p2CPSLbl = CCLabelBMFont::create(fmt::format("{:.1f} CPS", cfg.p2CPS).c_str(), "bigFont.fnt");
    p2CPSLbl->setScale(0.30f);
    p2CPSLbl->setPosition({95.0f, 65.0f});
    m_contentContainer->addChild(p2CPSLbl);

    auto p2PlusSpr = ButtonSprite::create("+", "goldFont.fnt", "GJ_button_01.png", 0.55f);
    p2PlusSpr->setScale(0.55f);
    auto p2Plus = CCMenuItemSpriteExtra::create(p2PlusSpr, this, menu_selector(SilicateGUI::onP2CPSPlus));
    p2Plus->setPosition({155.0f, 65.0f});
    menu->addChild(p2Plus);

    // P2 Mode: Hold / Spam
    auto p2ModeSpr = ButtonSprite::create(cfg.p2HoldMode ? "Hold Only" : "Spam Always", "chatFont.fnt", "GJ_button_04.png", 0.55f);
    p2ModeSpr->setScale(0.58f);
    auto p2ModeBtn = CCMenuItemSpriteExtra::create(p2ModeSpr, this, menu_selector(SilicateGUI::onP2ToggleHold));
    p2ModeBtn->setPosition({235.0f, 65.0f});
    menu->addChild(p2ModeBtn);

    // P2 Presets
    for (int i = 0; i < 5; ++i) {
        auto spr = ButtonSprite::create(fmt::format("{:g}", presets[i]).c_str(), "chatFont.fnt", "GJ_button_02.png", 0.5f);
        spr->setScale(0.52f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SilicateGUI::onP2Preset));
        btn->setTag(static_cast<int>(presets[i]));
        btn->setPosition({28.0f + (i * 44.0f), 35.0f});
        menu->addChild(btn);
    }
}

// =============================================================================
// TAB 3: PHYSICS & TPS
// =============================================================================
void SilicateGUI::buildPhysicsTPSTab() {
    auto title = CCLabelBMFont::create("Physics / TPS Changer", "bigFont.fnt");
    title->setScale(0.44f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 240.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& cfg = Config::get();

    // TPS Toggle
    auto tpsToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleTPS), 0.55f
    );
    tpsToggle->toggle(cfg.tpsEnabled);
    tpsToggle->setPosition({20.0f, 205.0f});
    menu->addChild(tpsToggle);

    auto tpsTitle = CCLabelBMFont::create("Custom TPS Bypass", "bigFont.fnt");
    tpsTitle->setScale(0.32f);
    tpsTitle->setAnchorPoint({0.0f, 0.5f});
    tpsTitle->setPosition({38.0f, 205.0f});
    m_contentContainer->addChild(tpsTitle);

    // TPS Stepper [-] [ 240 TPS ] [+]
    auto tpsMinusSpr = ButtonSprite::create("-", "goldFont.fnt", "GJ_button_01.png", 0.55f);
    tpsMinusSpr->setScale(0.55f);
    auto tpsMinus = CCMenuItemSpriteExtra::create(tpsMinusSpr, this, menu_selector(SilicateGUI::onTPSMinus));
    tpsMinus->setPosition({45.0f, 175.0f});
    menu->addChild(tpsMinus);

    auto tpsValLbl = CCLabelBMFont::create(fmt::format("{:g} TPS", cfg.targetTPS).c_str(), "bigFont.fnt");
    tpsValLbl->setScale(0.32f);
    tpsValLbl->setPosition({120.0f, 175.0f});
    m_contentContainer->addChild(tpsValLbl);

    auto tpsPlusSpr = ButtonSprite::create("+", "goldFont.fnt", "GJ_button_01.png", 0.55f);
    tpsPlusSpr->setScale(0.55f);
    auto tpsPlus = CCMenuItemSpriteExtra::create(tpsPlusSpr, this, menu_selector(SilicateGUI::onTPSPlus));
    tpsPlus->setPosition({195.0f, 175.0f});
    menu->addChild(tpsPlus);

    // TPS Presets: 240, 360, 480, 1000, 2000, 50000
    int tpsPresets[] = {240, 360, 480, 1000, 2000, 50000};
    for (int i = 0; i < 6; ++i) {
        auto spr = ButtonSprite::create(fmt::format("{}", tpsPresets[i]).c_str(), "chatFont.fnt", "GJ_button_02.png", 0.5f);
        spr->setScale(0.50f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SilicateGUI::onTPSPreset));
        btn->setTag(tpsPresets[i]);
        btn->setPosition({24.0f + (i * 42.0f), 145.0f});
        menu->addChild(btn);
    }

    // Speedhack Section
    auto spdTitle = CCLabelBMFont::create("Speedhack", "bigFont.fnt");
    spdTitle->setScale(0.32f);
    spdTitle->setAnchorPoint({0.0f, 0.5f});
    spdTitle->setPosition({15.0f, 100.0f});
    m_contentContainer->addChild(spdTitle);

    auto spdMinusSpr = ButtonSprite::create("-", "goldFont.fnt", "GJ_button_01.png", 0.55f);
    spdMinusSpr->setScale(0.55f);
    auto spdMinus = CCMenuItemSpriteExtra::create(spdMinusSpr, this, menu_selector(SilicateGUI::onSpeedMinus));
    spdMinus->setPosition({45.0f, 70.0f});
    menu->addChild(spdMinus);

    auto spdValLbl = CCLabelBMFont::create(fmt::format("{:.2f}x", cfg.speedhackValue).c_str(), "bigFont.fnt");
    spdValLbl->setScale(0.32f);
    spdValLbl->setPosition({120.0f, 70.0f});
    m_contentContainer->addChild(spdValLbl);

    auto spdPlusSpr = ButtonSprite::create("+", "goldFont.fnt", "GJ_button_01.png", 0.55f);
    spdPlusSpr->setScale(0.55f);
    auto spdPlus = CCMenuItemSpriteExtra::create(spdPlusSpr, this, menu_selector(SilicateGUI::onSpeedPlus));
    spdPlus->setPosition({195.0f, 70.0f});
    menu->addChild(spdPlus);

    // Speed Presets: 0.25, 0.5, 0.75, 1.0
    float speedPresets[] = {0.25f, 0.50f, 0.75f, 1.00f};
    for (int i = 0; i < 4; ++i) {
        auto spr = ButtonSprite::create(fmt::format("{:.2g}x", speedPresets[i]).c_str(), "chatFont.fnt", "GJ_button_02.png", 0.5f);
        spr->setScale(0.55f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SilicateGUI::onSpeedPreset));
        btn->setTag(static_cast<int>(speedPresets[i] * 100));
        btn->setPosition({35.0f + (i * 55.0f), 35.0f});
        menu->addChild(btn);
    }
}

// =============================================================================
// TAB 4: TRAJECTORY
// =============================================================================
void SilicateGUI::buildTrajectoryTab() {
    auto title = CCLabelBMFont::create("Trajectory Visualizer", "bigFont.fnt");
    title->setScale(0.44f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 240.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& cfg = Config::get();

    auto trajToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleTrajectory), 0.60f
    );
    trajToggle->toggle(cfg.showTrajectory);
    trajToggle->setPosition({25.0f, 195.0f});
    menu->addChild(trajToggle);

    auto trajLbl = CCLabelBMFont::create("Show Trajectory & Jump Arc", "bigFont.fnt");
    trajLbl->setScale(0.35f);
    trajLbl->setAnchorPoint({0.0f, 0.5f});
    trajLbl->setPosition({48.0f, 195.0f});
    m_contentContainer->addChild(trajLbl);

    std::string desc = 
        "- Calculates exact parabolic jump paths\n"
        "- Predicts Wave 45-deg and Ship curves\n"
        "- Detects hazards with Pink Death Boxes\n"
        "- Supports gravity inversion & dual modes";

    auto descLbl = CCLabelBMFont::create(desc.c_str(), "chatFont.fnt");
    descLbl->setScale(0.72f);
    descLbl->setAnchorPoint({0.0f, 1.0f});
    descLbl->setPosition({20.0f, 155.0f});
    m_contentContainer->addChild(descLbl);
}

// =============================================================================
// TAB 5: HITBOXES
// =============================================================================
void SilicateGUI::buildHitboxesTab() {
    auto title = CCLabelBMFont::create("Hitbox Visualizer", "bigFont.fnt");
    title->setScale(0.44f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 240.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& cfg = Config::get();

    auto hitToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleHitboxes), 0.60f
    );
    hitToggle->toggle(cfg.showHitboxes);
    hitToggle->setPosition({25.0f, 195.0f});
    menu->addChild(hitToggle);

    auto hitLbl = CCLabelBMFont::create("Show Hitbox Wireframes", "bigFont.fnt");
    hitLbl->setScale(0.35f);
    hitLbl->setAnchorPoint({0.0f, 0.5f});
    hitLbl->setPosition({48.0f, 195.0f});
    m_contentContainer->addChild(hitLbl);

    std::string desc =
        "- Player 1 & Player 2 Inner/Outer hitboxes\n"
        "- Hazard hitboxes (Spikes, Saws) in Red\n"
        "- Solid blocks and platforms in Blue\n"
        "- High-performance GPU wireframe rendering";

    auto descLbl = CCLabelBMFont::create(desc.c_str(), "chatFont.fnt");
    descLbl->setScale(0.72f);
    descLbl->setAnchorPoint({0.0f, 1.0f});
    descLbl->setPosition({20.0f, 155.0f});
    m_contentContainer->addChild(descLbl);
}

// =============================================================================
// TAB 6: KEYBINDS & SETTINGS
// =============================================================================
void SilicateGUI::buildKeybindsTab() {
    auto title = CCLabelBMFont::create("Keybinds & Settings", "bigFont.fnt");
    title->setScale(0.44f);
    title->setAnchorPoint({0.0f, 1.0f});
    title->setPosition({10.0f, 240.0f});
    m_contentContainer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    m_contentContainer->addChild(menu);

    auto& cfg = Config::get();

    // Keybind Section
    auto keyTitle = CCLabelBMFont::create("Menu Toggle Key:", "bigFont.fnt");
    keyTitle->setScale(0.32f);
    keyTitle->setAnchorPoint({0.0f, 0.5f});
    keyTitle->setPosition({15.0f, 200.0f});
    m_contentContainer->addChild(keyTitle);

    std::string btnText = cfg.isListeningForKey ? "[ Press Key... ]" : fmt::format("[ {} ]", cfg.getMenuKeyName());
    auto keySpr = ButtonSprite::create(btnText.c_str(), "goldFont.fnt", cfg.isListeningForKey ? "GJ_button_01.png" : "GJ_button_04.png", 0.55f);
    keySpr->setScale(0.60f);
    auto keyBtn = CCMenuItemSpriteExtra::create(keySpr, this, menu_selector(SilicateGUI::onChangeMenuKey));
    keyBtn->setPosition({200.0f, 200.0f});
    menu->addChild(keyBtn);

    // Noclip Toggle
    auto noclipToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleNoclip), 0.55f
    );
    noclipToggle->toggle(cfg.noclip);
    noclipToggle->setPosition({20.0f, 155.0f});
    menu->addChild(noclipToggle);

    auto noclipLbl = CCLabelBMFont::create("Noclip (Invincibility)", "bigFont.fnt");
    noclipLbl->setScale(0.32f);
    noclipLbl->setAnchorPoint({0.0f, 0.5f});
    noclipLbl->setPosition({38.0f, 155.0f});
    m_contentContainer->addChild(noclipLbl);

    // Safe Mode Toggle
    auto safeToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(SilicateGUI::onToggleSafeMode), 0.55f
    );
    safeToggle->toggle(cfg.safeMode);
    safeToggle->setPosition({20.0f, 115.0f});
    menu->addChild(safeToggle);

    auto safeLbl = CCLabelBMFont::create("Safe Mode (Disable Leaderboard Submissions)", "bigFont.fnt");
    safeLbl->setScale(0.30f);
    safeLbl->setAnchorPoint({0.0f, 0.5f});
    safeLbl->setPosition({38.0f, 115.0f});
    m_contentContainer->addChild(safeLbl);

    // Drag Instructions
    std::string tips =
        "- Drag the top title bar to move this window anywhere!\n"
        "- Press your configured Menu Key to toggle anytime\n"
        "- Fallback hotkeys: Tab, F, Left Alt";

    auto tipsLbl = CCLabelBMFont::create(tips.c_str(), "chatFont.fnt");
    tipsLbl->setScale(0.68f);
    tipsLbl->setAnchorPoint({0.0f, 1.0f});
    tipsLbl->setPosition({15.0f, 80.0f});
    m_contentContainer->addChild(tipsLbl);
}

void SilicateGUI::onChangeMenuKey(CCObject*) {
    Config::get().isListeningForKey = !Config::get().isListeningForKey;
    refreshCurrentTab();
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
