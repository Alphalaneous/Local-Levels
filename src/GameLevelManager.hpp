#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/GameLevelManager.hpp>

using namespace geode::prelude;

class $modify(MyGameLevelManager, GameLevelManager) {
    GJGameLevel* getLocalLevel(int uniqueID);
};
