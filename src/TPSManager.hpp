#pragma once

#include <Geode/Geode.hpp>

class TPSManager {
public:
    static TPSManager& get() {
        static TPSManager instance;
        return instance;
    }

    void applyTPS();
};
