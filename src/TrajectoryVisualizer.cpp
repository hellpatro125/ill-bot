#include "TrajectoryVisualizer.hpp"
#include "Config.hpp"
#include "eclipse/config.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

static void drawSegmentQuad(CCDrawNode* node, const CCPoint& from, const CCPoint& to, float radius, const ccColor4F& color) {
    if (!node) return;
    CCPoint diff = ccpSub(to, from);
    float len = ccpLength(diff);
    if (len < 0.001f) return;

    CCPoint dir = ccpMult(diff, 1.0f / len);
    CCPoint perp = ccp(-dir.y * radius, dir.x * radius);

    CCPoint verts[4] = {
        ccpAdd(from, perp),
        ccpSub(from, perp),
        ccpSub(to, perp),
        ccpAdd(to, perp)
    };

    node->drawPolygon(verts, 4, color, 0.0f, color);
}

void TrajectoryVisualizer::syncWithModEngines() {
    bool enabled = Config::get().showTrajectory;

    // Direct integration with Eclipse Menu's native ShowTrajectory engine
    if (geode::Loader::get()->isModLoaded("eclipse.eclipse-menu")) {
        eclipse::config::set<bool>("player.showtrajectory", enabled);
        eclipse::config::set<int>("player.showtrajectory.iterations", 180);
    }
}

void TrajectoryVisualizer::setupForPlayLayer(PlayLayer* playLayer) {
    if (!playLayer) return;

    syncWithModEngines();

    if (m_drawNode) {
        m_drawNode->removeFromParentAndCleanup(true);
        m_drawNode = nullptr;
    }

    // Only create fallback CCDrawNode if Eclipse is not handling trajectory
    if (!geode::Loader::get()->isModLoaded("eclipse.eclipse-menu")) {
        m_drawNode = CCDrawNode::create();
        if (!m_drawNode) return;

        m_drawNode->setBlendFunc(ccBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }));
        m_drawNode->setZOrder(9999);

        if (playLayer->m_objectLayer && playLayer->m_objectLayer->getParent()) {
            playLayer->m_objectLayer->getParent()->addChild(m_drawNode, 9999);
        } else {
            playLayer->addChild(m_drawNode, 9999);
        }
    }
}

void TrajectoryVisualizer::clear() {
    if (m_drawNode) {
        m_drawNode->clear();
    }
}

void TrajectoryVisualizer::updateTrajectory(PlayLayer* playLayer) {
    if (!Config::get().showTrajectory || !playLayer) {
        clear();
        return;
    }

    // If Eclipse is loaded, it renders the complete, 100% accurate trajectory engine
    if (geode::Loader::get()->isModLoaded("eclipse.eclipse-menu")) {
        eclipse::config::set<bool>("player.showtrajectory", true);
        clear();
        return;
    }

    // Fallback real-time forward Euler physics simulator (no fake hardcoded parabolas)
    auto* player = playLayer->m_player1;
    if (!player || player->m_isDead) {
        clear();
        return;
    }

    if (!m_drawNode || !m_drawNode->getParent()) {
        setupForPlayLayer(playLayer);
        if (!m_drawNode) return;
    }

    m_drawNode->clear();

    CCPoint currentPos = player->getPosition();
    float gravityDir = player->m_isUpsideDown ? -1.0f : 1.0f;
    float vx = player->m_playerSpeed;
    if (vx <= 0.0f) vx = 8.37f * 60.0f; // Standard 1x speed fallback

    float vy = static_cast<float>(player->m_yVelocity);
    float gravity = static_cast<float>(player->m_gravity) * 60.0f * 60.0f;
    if (gravity <= 0.0f) gravity = 0.958199f * 60.0f * 60.0f;

    const float dt = 1.0f / 60.0f;
    const int steps = 90;

    CCPoint simPos = currentPos;
    float simVy = vy;

    CCPoint prevPos = simPos;
    ccColor4F trajColor = {0.0f, 1.0f, 0.35f, 0.9f}; // Clean neon green line

    for (int i = 0; i < steps; ++i) {
        simPos.x += vx * dt;
        simPos.y += simVy * dt;

        if (player->m_isDart) {
            // Wave mode: follows diagonal angle
        } else if (!player->isFlying()) {
            // Cube / Ball / Robot / Swing: apply real gravitational acceleration
            simVy -= gravity * gravityDir * dt;
        }

        drawSegmentQuad(m_drawNode, prevPos, simPos, 2.0f, trajColor);
        prevPos = simPos;
    }
}
