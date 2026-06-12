#include "EditLevelLayer.hpp"
#include "GJGameLevel.hpp"
#include "LocalLevelsLayer.hpp"
#include <hjfod.gmd-api/include/GMD.hpp>
#include <Geode/utils/base64.hpp>
#include "Utils.hpp"

void MyEditLevelLayer::onBack(cocos2d::CCObject* sender){
    auto level = static_cast<MyGJGameLevel*>(m_level);
    if (level->isLocal()) {
        if (m_fields->m_modified || (!level->m_fields->m_isUploaded && level->m_isUploaded)) {
            level->m_levelDesc = utils::base64::encode(static_cast<CCTextInputNode*>(getChildByID("description-input"))->getString());
            
            auto parent = level->getFilePath().parent_path();

            auto newPath = local_levels::utils::getNextAvailableName(parent / fmt::format("{}.gmd", level->m_levelName));

            std::error_code err;
            std::filesystem::rename(level->getFilePath(), newPath, err);
            
            level->setFilePath(newPath);

            auto res = gmd::exportLevelAsGmd(m_level, newPath);
            if (!res) log::error("{}", res.err());
        }

        auto scene = CCScene::create();
		scene->addChild(LocalLevelsLayer::create(level->getFilePath().parent_path()));
		auto trans = CCTransitionFade::create(0.5f, scene);
        CCDirector::get()->replaceScene(trans);
        return;
    }

    EditLevelLayer::onBack(sender);
}

void MyEditLevelLayer::confirmDelete(cocos2d::CCObject* sender) {
    auto level = static_cast<MyGJGameLevel*>(m_level);
    
    if (level->isLocal()) {
        createQuickPopup("Delete Level", "Are you sure you want to <cr>delete</c> this level?\n<cy>This action cannot be undone.</c>", "NO", "YES", [level] (auto alert, bool btn2) {
            if (btn2) {
                std::error_code err;
                std::filesystem::remove(level->getFilePath(), err);

                auto scene = CCScene::create();
                scene->addChild(LocalLevelsLayer::create(level->getFilePath().parent_path()));
                auto trans = CCTransitionFade::create(0.5f, scene);
                CCDirector::get()->replaceScene(trans);
            }
        });
        return;
    }
    EditLevelLayer::confirmDelete(sender);
}

void MyEditLevelLayer::confirmClone(cocos2d::CCObject* sender) {
    auto level = static_cast<MyGJGameLevel*>(m_level);
    
    if (level->isLocal()) {
        createQuickPopup("Clone Level", "Create a <cb>copy</c> of this <cg>level</c>?", "NO", "YES", [level] (auto alert, bool btn2) {
            if (btn2) {
                auto copyLevel = static_cast<MyGJGameLevel*>(GJGameLevel::create());
                copyLevel->copyLevelInfo(level);
                copyLevel->setLocal(true);

                std::error_code err;
                std::filesystem::create_directory(Mod::get()->getSaveDir() / "levels", err);

                auto newPath = local_levels::utils::getNextAvailableName(level->getFilePath().parent_path() / fmt::format("{}.gmd", copyLevel->m_levelName));

                copyLevel->setFilePath(newPath);
                
                auto res = gmd::exportLevelAsGmd(copyLevel, copyLevel->getFilePath());
                if (!res) log::error("{}", res.err());
                
                auto editLevelLayer = EditLevelLayer::create(copyLevel);

                auto scene = CCScene::create();
                scene->addChild(editLevelLayer);

                auto trans = CCTransitionFade::create(0.5f, scene);
                CCDirector::get()->replaceScene(trans);
            }
        });
        return;
    }
    EditLevelLayer::confirmClone(sender);
}

void MyEditLevelLayer::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    auto level = static_cast<MyGJGameLevel*>(m_level);

    if (level->isLocal() && layer->getTag() == 4 && btn2) {
        std::error_code err;
        std::filesystem::remove(level->getFilePath(), err);

        auto scene = CCScene::create();
		scene->addChild(LocalLevelsLayer::create(level->getFilePath().parent_path()));
		auto trans = CCTransitionFade::create(0.5f, scene);
        CCDirector::get()->replaceScene(trans);

        return;
    }

    EditLevelLayer::FLAlert_Clicked(layer, btn2);
}

bool MyEditLevelLayer::init(GJGameLevel* level) {
    if (!EditLevelLayer::init(level)) return false;

    auto myLevel = static_cast<MyGJGameLevel*>(m_level);
    auto levelActionsMenu = getChildByID("level-actions-menu");

    if (myLevel->isLocal()) {
        auto folderMenu = getChildByID("folder-menu");
        if (folderMenu) {
            auto folderButton = folderMenu->getChildByID("folder-button");
            if (folderButton) {
                folderButton->setVisible(false);
            }

            auto backupsButton = folderMenu->getChildByID("hjfod.betteredit/backups-list");
            if (backupsButton) {
                backupsButton->setVisible(false);
            }
            folderMenu->updateLayout();
        }

        if (levelActionsMenu) {
            auto toTopButton = levelActionsMenu->getChildByID("move-to-top-button");
            if (toTopButton) {
                toTopButton->setVisible(false);
            }
            levelActionsMenu->updateLayout();
        }
    }

    if (levelActionsMenu) {
        auto transferBtn = CCMenuItemExt::createSpriteExtraWithFilename("transfer.png"_spr, 1.f, [this, myLevel] (auto sender) {
            if (myLevel->isLocal()) {
                createQuickPopup("Transfer Level", "Transfer this <cg>level</c> to your <cb>save file</c>?", "NO", "YES", [this, myLevel] (auto alert, bool btn2) {
                    if (btn2) {
                        myLevel->setLocal(false);
                        std::error_code err;
                        std::filesystem::remove_all(myLevel->getFilePath(), err);

                        LocalLevelManager::get()->m_localLevels->insertObject(myLevel, 0);

                        auto editLevelLayer = EditLevelLayer::create(myLevel);

                        auto scene = CCScene::create();
                        scene->addChild(editLevelLayer);

                        LocalLevelManager::get()->save();
                        
                        CCDirector::get()->popScene();

                        auto trans = CCTransitionFade::create(0.5f, scene);
                        CCDirector::get()->replaceScene(trans);
                    }
                });
            }
            else {
                createQuickPopup("Transfer Level", "Transfer this <cg>level</c> out of your <cb>save file</c> and into to a local level?", "NO", "YES", [this, myLevel] (auto alert, bool btn2) {
                    if (btn2) {
                        std::error_code err;
                        std::filesystem::create_directory(Mod::get()->getSaveDir() / "levels", err);

                        auto newPath = local_levels::utils::getNextAvailableName(Mod::get()->getSaveDir() / "levels" / fmt::format("{}.gmd", myLevel->m_levelName));

                        myLevel->setFilePath(newPath);
                        myLevel->setLocal(true);
                        
                        auto res = gmd::exportLevelAsGmd(myLevel, myLevel->getFilePath());
                        if (!res) log::error("{}", res.err());

                        LocalLevelManager::get()->m_localLevels->removeObject(myLevel);
                        
                        auto editLevelLayer = EditLevelLayer::create(myLevel);

                        auto scene = CCScene::create();
                        scene->addChild(editLevelLayer);

                        LocalLevelManager::get()->save();

                        auto trans = CCTransitionFade::create(0.5f, scene);
                        CCDirector::get()->replaceScene(trans);
                    }
                });
            }
        });

        levelActionsMenu->addChild(transferBtn);
        levelActionsMenu->updateLayout();
    }

    return true;
}

void MyEditLevelLayer::textChanged(CCTextInputNode* node) {
    EditLevelLayer::textChanged(node);

    auto fields = m_fields.self();
    if (fields->m_inputOpenedOnce) {
        fields->m_modified = true;
    }
}

void MyEditLevelLayer::textInputOpened(CCTextInputNode* node) {
    EditLevelLayer::textInputOpened(node);
    m_fields->m_inputOpenedOnce = true;
}