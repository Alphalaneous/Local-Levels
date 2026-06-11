#include "LevelBrowserLayer.hpp"
#include <Geode/ui/Button.hpp>
#include "LocalLevelsLayer.hpp"

bool MyLevelBrowserLayer::init(GJSearchObject* object) {
    if (!LevelBrowserLayer::init(object)) return false;

    if (object->m_searchType == SearchType::MyLevels) {
        auto toLocalBtn = geode::Button::createWithSpriteFrameName("gj_folderBtn_001.png", [] (auto sender) {
            auto scene = CCScene::create();
            scene->addChild(LocalLevelsLayer::create(Mod::get()->getSaveDir() / "levels"));
            auto trans = CCTransitionFade::create(0.5f, scene);
            CCDirector::get()->pushScene(trans);
        });
        toLocalBtn->setScale(0.8f);

        auto plusLabel = CCLabelBMFont::create("L", "bigFont.fnt");
        plusLabel->setPosition(toLocalBtn->getContentSize() / 2);
        plusLabel->setScale(0.5f);

        toLocalBtn->addChild(plusLabel);

        auto winSize = CCDirector::get()->getWinSize();

        toLocalBtn->setPosition({winSize.width / 2 + 150, winSize.height / 2 + 120});
        toLocalBtn->setZOrder(10);

        addChild(toLocalBtn);
    }

    return true;
}

