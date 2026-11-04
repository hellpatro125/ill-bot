#include "HitboxVisualizer.hpp"
#include "Config.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

static void drawWireBox(CCDrawNode* node, const CCRect& rect, const ccColor4F& color, float width = 1.2f) {
    if (!node) return;
    CCPoint verts[4] = {
        CCPoint(rect.origin.x, rect.origin.y),
        CCPoint(rect.origin.x + rect.size.width, rect.origin.y),
        CCPoint(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height),
        CCPoint(rect.origin.x, rect.origin.y + rect.size.height)
    };
    node->drawPolygon(verts, 4, {0, 0, 0, 0}, width, color);
}

void HitboxVisualizer::setupForPlayLayer(PlayLayer* playLayer) {
    if (!playLayer) return;

    if (m_drawNode) {
        m_drawNode->removeFromParentAndCleanup(true);
        m_drawNode = nullptr;
    }

    m_drawNode = CCDrawNode::create();
    if (!m_drawNode) return;

    m_drawNode->setBlendFunc(ccBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }));
    m_drawNode->setZOrder(9998);

    if (playLayer->m_objectLayer && playLayer->m_objectLayer->getParent()) {
        playLayer->m_objectLayer->getParent()->addChild(m_drawNode, 9998);
    } else {
        playLayer->addChild(m_drawNode, 9998);
    }
}

void HitboxVisualizer::clear() {
    if (m_drawNode) {
        m_drawNode->clear();
    }
}

void HitboxVisualizer::updateHitboxes(PlayLayer* playLayer) {
    if (!Config::get().showHitboxes || !playLayer) {
        clear();
        return;
    }

    if (!m_drawNode || !m_drawNode->getParent()) {
        setupForPlayLayer(playLayer);
        if (!m_drawNode) return;
    }

    m_drawNode->clear();

    // 1. Draw Player 1 Hitbox
    if (auto* p1 = playLayer->m_player1) {
        CCPoint p = p1->getPosition();
        // Inner hitbox (death box)
        CCRect innerBox(p.x - 10.0f, p.y - 10.0f, 20.0f, 20.0f);
        drawWireBox(m_drawNode, innerBox, {1.0f, 0.2f, 0.2f, 0.9f}, 1.5f);

        // Outer bounds
        CCRect outerBox(p.x - 15.0f, p.y - 15.0f, 30.0f, 30.0f);
        drawWireBox(m_drawNode, outerBox, {0.2f, 1.0f, 0.3f, 0.8f}, 1.0f);
    }

    // 2. Draw Player 2 Hitbox
    if (auto* p2 = playLayer->m_player2) {
        CCPoint p = p2->getPosition();
        CCRect innerBox(p.x - 10.0f, p.y - 10.0f, 20.0f, 20.0f);
        drawWireBox(m_drawNode, innerBox, {1.0f, 0.4f, 0.1f, 0.9f}, 1.5f);

        CCRect outerBox(p.x - 15.0f, p.y - 15.0f, 30.0f, 30.0f);
        drawWireBox(m_drawNode, outerBox, {0.1f, 0.8f, 1.0f, 0.8f}, 1.0f);
    }

    // 3. Draw Nearby Objects Hitboxes
    if (playLayer->m_objects && playLayer->m_player1) {
        float playerX = playLayer->m_player1->getPositionX();
        auto* objects = playLayer->m_objects;

        for (int i = 0; i < objects->count(); ++i) {
            auto* obj = static_cast<GameObject*>(objects->objectAtIndex(i));
            if (!obj) continue;

            CCPoint pos = obj->getPosition();
            if (std::abs(pos.x - playerX) > 280.0f) continue;

            CCRect rect = obj->getObjectRect();
            if (rect.size.width < 1.0f || rect.size.height < 1.0f) {
                rect = CCRect(pos.x - 15.0f, pos.y - 15.0f, 30.0f, 30.0f);
            }

            bool isHazard = (obj->m_objectType == GameObjectType::Hazard ||
                             obj->m_objectType == GameObjectType::AnimatedHazard ||
                             obj->m_objectID == 8 || obj->m_objectID == 9 || obj->m_objectID == 39);

            if (isHazard) {
                drawWireBox(m_drawNode, rect, {1.0f, 0.1f, 0.1f, 0.85f}, 1.2f);
            } else {
                drawWireBox(m_drawNode, rect, {0.2f, 0.5f, 1.0f, 0.7f}, 1.0f);
            }
        }
    }
}
