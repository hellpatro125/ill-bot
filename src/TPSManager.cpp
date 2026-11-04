#include "TPSManager.hpp"
#include "Config.hpp"
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

void TPSManager::applyTPS() {
    auto& cfg = Config::get();
    if (cfg.tpsEnabled && cfg.targetTPS > 0.0f) {
        float interval = 1.0f / cfg.targetTPS;
        cocos2d::CCApplication::sharedApplication()->setAnimationInterval(interval);
    }
}

class $modify(TPSPlayLayer, PlayLayer) {
    void update(float dt) {
        auto& cfg = Config::get();
        if (cfg.tpsEnabled && cfg.targetTPS > 0.0f) {
            // Target physics tick delta
            float targetDt = 1.0f / cfg.targetTPS;
            
            // If delta time is significantly larger than target tick, sub-step updates
            if (dt > targetDt * 1.5f && targetDt > 0.00001f) {
                int steps = static_cast<int>(dt / targetDt);
                if (steps > 20) steps = 20; // safety ceiling
                for (int i = 0; i < steps; ++i) {
                    PlayLayer::update(targetDt);
                }
                float rem = std::fmod(dt, targetDt);
                if (rem > 0.0001f) {
                    PlayLayer::update(rem);
                }
                return;
            }
        }
        PlayLayer::update(dt);
    }
};

class $modify(TPSScheduler, cocos2d::CCScheduler) {
    void update(float dt) {
        auto& cfg = Config::get();
        if (cfg.speedhackEnabled && cfg.speedhackValue > 0.0f) {
            CCScheduler::update(dt * cfg.speedhackValue);
        } else {
            CCScheduler::update(dt);
        }
    }
};
