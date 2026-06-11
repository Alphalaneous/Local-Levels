#include "GameLevelManager.hpp"
#include "GJGameLevel.hpp"

GJGameLevel* MyGameLevelManager::getLocalLevel(int uniqueID) {
    auto ell = CCScene::get()->getChildByType<EditLevelLayer>(0);
    if (ell) {
        auto level = static_cast<MyGJGameLevel*>(ell->m_level);
        if (level->isLocal() && level->m_M_ID == uniqueID) {
            return level;
        }
    }
    return GameLevelManager::getLocalLevel(uniqueID);
}