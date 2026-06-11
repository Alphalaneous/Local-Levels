#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/EditLevelLayer.hpp>

using namespace geode::prelude;

class $modify(MyEditLevelLayer, EditLevelLayer) {

    struct Fields {
        bool m_modified;
        bool m_inputOpenedOnce;
    };

    void onBack(cocos2d::CCObject* sender);
    void confirmDelete(cocos2d::CCObject* sender);
    void confirmClone(cocos2d::CCObject* sender);

    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2);
    void textChanged(CCTextInputNode* node);
    void textInputOpened(CCTextInputNode* node);

    bool init(GJGameLevel* level);

    static void onModify(auto& self) {
        (void) self.setHookPriority("EditLevelLayer::init", Priority::LatePost);
    }
};
