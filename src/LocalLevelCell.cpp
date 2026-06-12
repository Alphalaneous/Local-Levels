#include "LocalLevelCell.hpp"
#include "GJGameLevel.hpp"
#include <hjfod.gmd-api/include/GMD.hpp>
#include "LocalLevelsLayer.hpp"
#include "Utils.hpp"

LocalLevelCell* LocalLevelCell::create(const std::filesystem::path& path, LocalLevelsLayer* levelsLayer) {
    auto ret = new LocalLevelCell();
    if (ret->init(path, levelsLayer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}


bool LocalLevelCell::isSelected() {
    return m_selectToggle->isToggled();
}

void LocalLevelCell::setSelected(bool selected) {
    m_selectToggle->toggleWithCallback(selected);
}

bool LocalLevelCell::isFolder() {
    return m_isFolder;
}

void LocalLevelCell::checkMoveAllowed() {
    runAction(CallFuncExt::create([this] {
        auto selected = m_levelsLayer->getSelectedCells();

        bool allowed = !selected.empty();
        for (auto cell : selected) {
            if (cell == this) {
                allowed = false;
                break;
            }
        }

        if (allowed) {
            m_moveToFolderButton->setEnabled(true);
            m_moveToFolderButton->setOpacity(255);
            m_moveToFolderButton->setColor({255, 255, 255});
        }
        else {
            m_moveToFolderButton->setEnabled(false);
            m_moveToFolderButton->setOpacity(127);
            m_moveToFolderButton->setColor({150, 150, 150});
        }
    }));
}

void LocalLevelCell::deleteFile() {
    std::error_code err;
    std::filesystem::remove_all(m_path, err);
}

const std::filesystem::path& LocalLevelCell::getPath() {
    return m_path;
}

bool LocalLevelCell::init(const std::filesystem::path& path, LocalLevelsLayer* levelsLayer) {
    if (!CCLayerColor::initWithColor({0, 0, 0, 50})) return false;

    m_path = path;
    m_levelsLayer = levelsLayer;
    m_selectToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.6f, [this] (auto sender) {
        m_levelsLayer->checkFolders();
    });
    auto menu = CCMenu::create();
    menu->setContentSize(m_selectToggle->getScaledContentSize());
    menu->ignoreAnchorPointForPosition(false);
    menu->setZOrder(10);

    m_selectToggle->setPosition(menu->getContentSize() / 2);

    menu->addChild(m_selectToggle);
    addChild(menu);

    UploadPopup aa;

    std::error_code err;
    if (std::filesystem::is_directory(path, err)) {
        m_isFolder = true;
        setContentSize({356, 30});

        auto openFolderButton = geode::Button::createWithSpriteFrameName("GJ_arrow_01_001.png", [path] (auto sender) {
            auto scene = CCScene::create();
            scene->addChild(LocalLevelsLayer::create(path));
            CCDirector::get()->replaceScene(scene);
        });
        
        static_cast<CCSprite*>(openFolderButton->getDisplayNode())->setFlipX(true);
        openFolderButton->setScale(0.6f);

        openFolderButton->setPosition({getContentWidth() - openFolderButton->getScaledContentWidth() / 2 - 10.f, getContentHeight() / 2});

        addChild(openFolderButton);

        menu->setPosition({getContentWidth() - openFolderButton->getScaledContentWidth() - menu->getScaledContentWidth() / 2 - 20.f, getContentHeight() / 2});

        m_moveToFolderButton = geode::Button::createWithSpriteFrameName("gj_folderBtn_001.png", [this, path] (auto sender) {
            createQuickPopup("Move Selected", "Move selected items to folder?", "Cancel", "Move", [this] (auto alert, bool btn2) {
                if (btn2) {
                    auto selected = m_levelsLayer->getSelectedCells();

                    for (auto item : selected) {
                        std::error_code err;

                        auto destination = m_path / item->getPath().filename();
                        auto checkedDest = local_levels::utils::getNextAvailableName(destination);

                        std::filesystem::rename(item->getPath(), checkedDest, err);
                    }

                    auto scene = CCScene::create();
                    scene->addChild(LocalLevelsLayer::create(m_path.parent_path()));
                    CCDirector::get()->replaceScene(scene);
                }
            });
        });
        
        auto arrowMoveSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
        arrowMoveSpr->setFlipX(true);
        arrowMoveSpr->setPosition({m_moveToFolderButton->getContentWidth() - 2.f, m_moveToFolderButton->getContentHeight() / 2});
        arrowMoveSpr->setScale(0.4f);

        m_moveToFolderButton->addChild(arrowMoveSpr);
        m_moveToFolderButton->setEnabled(false);
        m_moveToFolderButton->setOpacity(127);
        m_moveToFolderButton->setColor({150, 150, 150});

        m_moveToFolderButton->setScale(0.6f);

        m_moveToFolderButton->setPosition({getContentWidth() - m_moveToFolderButton->getScaledContentWidth() / 2 - 30.f - menu->getScaledContentWidth() - openFolderButton->getScaledContentWidth(), getContentHeight() / 2});

        addChild(m_moveToFolderButton);

        auto folderSpr = CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png");

        folderSpr->setAnchorPoint({0.f, 0.5f});
        folderSpr->setPosition({10, getContentHeight() / 2});
        folderSpr->setScale(0.7f);

        addChild(folderSpr);

        auto label = CCLabelBMFont::create(utils::string::pathToString(path.filename()).c_str(), "bigFont.fnt");

        label->setAnchorPoint({0.f, 0.5f});
        label->setPosition({folderSpr->getScaledContentWidth() + 20, getContentHeight() / 2 + 2});
        label->limitLabelWidth(170, 0.6f, 0.05f);

        addChild(label);

        int count = 0;

        std::error_code ec;
        for (std::filesystem::directory_iterator it(path, ec), end; it != end && !ec; it.increment(ec)) {
            auto entry = *it;
            if (utils::string::toLower(utils::string::pathToString(entry.path().extension())) == ".gmd" || entry.is_directory()) {
                count++;
            }
        }

        auto countLabel = CCLabelBMFont::create(fmt::format("Files: {}", count).c_str(), "chatFont.fnt");
        countLabel->setScale(0.6f);
        countLabel->setOpacity(200);
        countLabel->setAnchorPoint({0.f, 0.5f});
        countLabel->setPosition({label->getPositionX() + label->getScaledContentWidth() + 6, getContentHeight() / 2});

        addChild(countLabel);

        auto height = 2 / CCDirector::get()->getContentScaleFactor();

        auto separatorTop = CCLayerColor::create({0, 0, 0, 80});
        separatorTop->setContentSize({getContentWidth(), height});
        separatorTop->setAnchorPoint({0.f, 1.f});
        separatorTop->setPosition({0, getContentHeight()});

        addChild(separatorTop);

        auto separatorBottom = CCLayerColor::create({0, 0, 0, 80});
        separatorBottom->setContentSize({getContentWidth(), height});
        separatorBottom->setAnchorPoint({0.f, 0.f});
        separatorBottom->setPosition({0, 0});

        addChild(separatorBottom);
    }
    else {
        setContentSize({356, 70});

        auto res = gmd::importGmdAsLevel(path);
        if (res) {
            m_level = static_cast<MyGJGameLevel*>(res.unwrap());
            m_level->setLocal(true);
            m_level->setFilePath(path);
        }
        else {
            log::error("{}", res.err());
        }
    }

    if (m_level) {
        auto realLevelCell = LevelCell::create(356, 70);
        realLevelCell->loadFromLevel(m_level);
        realLevelCell->setAnchorPoint({0, 0});
        realLevelCell->setPosition({0, 0});

        addChild(realLevelCell);

        menu->setPosition({getContentWidth() - menu->getScaledContentWidth() / 2 - 80.f, getContentHeight() / 2});
    }

    return true;
}

MyGJGameLevel* LocalLevelCell::getLevel() {
    return m_level;
}