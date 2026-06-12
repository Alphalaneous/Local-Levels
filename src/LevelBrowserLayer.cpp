#include "LevelBrowserLayer.hpp"
#include <Geode/ui/Button.hpp>
#include "GJGameLevel.hpp"
#include "LocalLevelsLayer.hpp"
#include "Utils.hpp"
#include <hjfod.gmd-api/include/GMD.hpp>

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

        auto transferButton = geode::Button::createWithSprite("transfer.png"_spr, [this] (auto sender) {
            int selCount = 0;
            
            for (auto level : m_levels->asExt<GJGameLevel>()) {
                if (level->m_selected) {
                    selCount++;
                }
            }

            if (selCount == 0) {
                createQuickPopup("Nothing here...", "No levels selected.", "OK", nullptr, nullptr);
            }
            else {
                auto alert = createQuickPopup("Transfer", fmt::format("Are you sure you want to <cr>transfer</c> the <cy>{}</c> selected <cg>levels</c> out of your save and to your local levels?", selCount), "Back", "Transfer", [this] (auto alert, bool selected) {
                    if (selected) {
                        for (auto level : m_levels->asExt<GJGameLevel>()) {
                            if (level->m_selected) {
                                auto myLevel = static_cast<MyGJGameLevel*>(level);
                                std::error_code err;
                                std::filesystem::create_directory(Mod::get()->getSaveDir() / "levels", err);

                                auto newPath = local_levels::utils::getNextAvailableName(Mod::get()->getSaveDir() / "levels" / fmt::format("{}.gmd", myLevel->m_levelName));

                                myLevel->setFilePath(newPath);
                                myLevel->setLocal(true);
                                
                                auto res = gmd::exportLevelAsGmd(myLevel, myLevel->getFilePath());
                                if (!res) log::error("{}", res.err());

                                LocalLevelManager::get()->m_localLevels->removeObject(myLevel);
                            }
                        }

                        auto y = m_list->m_listView->m_tableView->m_contentLayer->getPositionY();
                        auto browser = LevelBrowserLayer::create(GJSearchObject::create(SearchType::MyLevels));
                        browser->m_list->m_listView->m_tableView->m_contentLayer->setPositionY(y);

                        auto scene = CCScene::create();
                        scene->addChild(browser);

                        LocalLevelManager::get()->save();

                        CCDirector::get()->replaceScene(scene);
                    }
                });

                alert->m_button2->updateBGImage("GJ_button_06.png");
            }
        });

        transferButton->setScale(0.4f);

        transferButton->setPosition({winSize.width / 2 + m_list->getContentWidth() / 2 - transferButton->getScaledContentWidth() / 2 - 25.f, winSize.height / 2 - 122});

        addChild(transferButton);
    }

    return true;
}

