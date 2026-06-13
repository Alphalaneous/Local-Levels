#include "LocalLevelsLayer.hpp"
#include "GJGameLevel.hpp"
#include "LocalLevelCell.hpp"
#include <hjfod.gmd-api/include/GMD.hpp>
#include "SortState.hpp"
#include "Utils.hpp"
#include <Geode/ui/Button.hpp>
#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include <Geode/external/fts/fts_fuzzy_match.h>

LocalLevelsLayer* LocalLevelsLayer::create(const std::filesystem::path& path) {
    auto ret = new LocalLevelsLayer();
    if (ret->init(path)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LocalLevelsLayer::init(const std::filesystem::path& path) {
    if (!CCLayer::init()) return false;

    std::error_code err;
    std::filesystem::create_directory(Mod::get()->getSaveDir() / "levels", err);

    m_path = path;
    bool isRoot = m_path == Mod::get()->getSaveDir() / "levels";

    auto bg = geode::createLayerBG();
    addChild(bg);
    geode::addSideArt(this, SideArt::BottomLeft | SideArt::BottomRight);

    auto winSize = CCDirector::get()->getWinSize();

    setKeypadEnabled(true);
    setKeyboardEnabled(true);

    auto backBtn = geode::Button::createWithSpriteFrameName("GJ_arrow_03_001.png", [this] (auto sender) {
        keyBackClicked();
    });

    backBtn->setPosition({7.75f + backBtn->getScaledContentWidth() / 2, winSize.height - 2.75f - backBtn->getScaledContentHeight() / 2});

    addChild(backBtn);
    
    auto rightContainer = CCNode::create();
    rightContainer->setAnchorPoint({1.f, 0.f});
    rightContainer->setPosition({winSize.width - 10, 10});
    rightContainer->setContentSize({40, 100});

    auto layout = static_cast<SimpleColumnLayout*>(ScrollLayer::createDefaultListLayout(5));
    layout->setMainAxisDirection(AxisDirection::BottomToTop);

    rightContainer->setLayout(layout);

    auto newBtn = geode::Button::createWithSpriteFrameName("GJ_newBtn_001.png", [this] (auto sender) {

        auto level = static_cast<MyGJGameLevel*>(GJGameLevel::create());
        level->m_levelType = GJLevelType::Editor;
        level->setLocal(true);
        level->m_levelName = "New Level";

        std::error_code err;
        std::filesystem::create_directory(Mod::get()->getSaveDir() / "levels", err);

        auto newPath = local_levels::utils::getNextAvailableName(m_path / fmt::format("{}.gmd", level->m_levelName));

        level->setFilePath(newPath);
        
        auto res = gmd::exportLevelAsGmd(level, level->getFilePath());
        if (!res) log::error("{}", res.err());
        
        auto editLevelLayer = EditLevelLayer::create(level);
        auto nameInput = static_cast<CCTextInputNode*>(editLevelLayer->getChildByID("level-name-input"));

        if (nameInput) {
            nameInput->setString("");
            editLevelLayer->m_levelName = level->m_levelName;
        }

        auto scene = CCScene::create();
        scene->addChild(editLevelLayer);

		auto trans = CCTransitionFade::create(0.5f, scene);
		CCDirector::get()->replaceScene(trans);
    });

    rightContainer->addChild(newBtn);

    if (!isRoot) {
        auto editBtn = geode::Button::createWithSpriteFrameName("GJ_editModeBtn_001.png", [this] (auto sender) {
            auto popup = SetTextPopup::create(utils::string::pathToString(m_path.filename()), "Set Folder Name", 256, "Folder Name", "Ok", true, 60.f);
            popup->m_delegate = this;
            popup->m_input->setAllowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!@#$%^&()-=_+[]{};' ");
            popup->setTag(2);
            popup->show();
        });

        rightContainer->addChild(editBtn);
    }

    auto newFolderBtn = geode::Button::createWithSpriteFrameName("gj_folderBtn_001.png", [this, path] (auto sender) {
        auto popup = SetTextPopup::create("", "Create Folder", 256, "Folder Name", "Ok", true, 60.f);
        popup->m_delegate = this;
        popup->m_input->setAllowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!@#$%^&()-=_+[]{};' ");
        popup->setTag(1);
        popup->show();
    });

    auto plusLabel = CCLabelBMFont::create("+", "bigFont.fnt");
    plusLabel->setAnchorPoint({1.f, 0.f});
    plusLabel->setPosition({newFolderBtn->getContentWidth() + 3, -2});

    newFolderBtn->addChild(plusLabel);

    rightContainer->addChild(newFolderBtn);
    rightContainer->updateLayout();

    addChild(rightContainer);

    if (!isRoot) {
        auto upDirSpr1 = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto upDirSpr2 = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        
        upDirSpr1->setAnchorPoint({0.f, 0.5f});
        upDirSpr2->setAnchorPoint({0.f, 0.5f});

        upDirSpr2->setZOrder(-1);
    
        auto upDirButton = geode::Button::create([this] (auto sender) {
            auto scene = CCScene::create();
            scene->addChild(LocalLevelsLayer::create(m_path.parent_path()));
            CCDirector::get()->replaceScene(scene);
        });
        upDirButton->setScale(0.6f);
        upDirButton->setZOrder(10);

        upDirButton->setContentSize({(upDirSpr1->getScaledContentWidth() / 2) * 3, upDirSpr1->getScaledContentHeight()});

        upDirSpr1->setPosition({0, upDirButton->getContentHeight() / 2});
        upDirSpr2->setPosition({upDirSpr1->getScaledContentWidth() / 2, upDirButton->getContentHeight() / 2});

        upDirButton->addChild(upDirSpr1);
        upDirButton->addChild(upDirSpr2);

        upDirButton->setPosition({25, winSize.height / 2 + 80});

        addChild(upDirButton);
    }
    
    auto listLayer = GJListLayer::create(nullptr, "", {0, 0, 0, 50}, 356, 220, 0);
    listLayer->setPosition(winSize / 2 - CCPoint{0, 5});
    listLayer->setAnchorPoint({0.5f, 0.5f});
    listLayer->ignoreAnchorPointForPosition(false);

    addChild(listLayer);

    auto searchBar = CCLayerColor::create({0, 40, 110, 255});
    searchBar->setAnchorPoint({0.5f, 1.f});
    searchBar->setPosition({listLayer->getContentWidth() / 2, listLayer->getContentHeight()});
    searchBar->ignoreAnchorPointForPosition(false);
    searchBar->setContentSize({listLayer->getContentWidth(), 30});

    listLayer->addChild(searchBar);

    auto input = geode::TextInput::create((listLayer->getContentWidth() - 52) / 0.7f, "Search...");
    input->setAnchorPoint({0.f, 0.5f});
    input->setPosition({10, searchBar->getContentHeight() / 2});
    input->setTextAlign(TextInputAlign::Left);
    input->setScale(0.7f);
    input->setCallback([this] (auto& str) {
        m_search = str;
        loadLevelsForPath(m_path);
    });

    searchBar->addChild(input);

    auto clearSearchBtn = geode::Button::createWithSpriteFrameName("GJ_longBtn07_001.png", [this, input] (auto sender) {
        input->setString("", true);
    });

    clearSearchBtn->setScale(0.7f);

    clearSearchBtn->setPosition({input->getPositionX() + input->getScaledContentWidth() + 5.f + clearSearchBtn->getScaledContentWidth() / 2, searchBar->getContentHeight() / 2});

    searchBar->addChild(clearSearchBtn);

    m_scrollLayer = ScrollLayer::create({356, 185.5f});
    m_scrollLayer->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout(0));
    m_scrollLayer->setAnchorPoint({0.5f, 0.5f});
    m_scrollLayer->ignoreAnchorPointForPosition(false);

    m_scrollLayer->setPosition(listLayer->getContentSize() / 2 - CCPoint{0, 12.5});

    listLayer->addChild(m_scrollLayer);

    m_pathContainer = CCNode::create();
    m_pathContainer->setAnchorPoint({0.5f, 0.5f});
    m_pathContainer->setContentSize({listLayer->getContentWidth() - 40, 20});
    m_pathContainer->setPosition({winSize.width / 2, winSize.height / 2 + 120});

    auto pathLayout = SimpleRowLayout::create();
    pathLayout->setMainAxisAlignment(MainAxisAlignment::Start);
    pathLayout->setMainAxisDirection(AxisDirection::BackToFront);
    pathLayout->setGap(3);

    m_pathContainer->setLayout(pathLayout);

    m_nameLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_nameLabel->setAnchorPoint({1.f, 0.f});
    m_nameLabel->setZOrder(10);
    m_nameLabel->limitLabelWidth(150, 0.6f, 0.01f);

    m_pathLabel = CCLabelBMFont::create("", "chatFont.fnt");
    m_pathLabel->setAnchorPoint({1.f, 0.f});
    m_pathLabel->setZOrder(10);
    m_pathLabel->setColor({64, 64, 64});
    m_pathLabel->limitLabelWidth(150, 0.5f, 0.01f);

    m_pathContainer->addChild(m_nameLabel);
    m_pathContainer->addChild(m_pathLabel);

    addChild(m_pathContainer);

    auto selectAllToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.5f, [this] (auto sender) {
        for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
            child->setSelected(!sender->isToggled());
        }
    });

    auto menu = CCMenu::create();
    menu->setContentSize(selectAllToggle->getScaledContentSize());
    menu->ignoreAnchorPointForPosition(false);
    menu->setZOrder(10);

    selectAllToggle->setPosition(menu->getContentSize() / 2);

    menu->addChild(selectAllToggle);
    addChild(menu);

    menu->setPosition({winSize.width / 2 - listLayer->getContentWidth() / 2 + menu->getContentWidth() / 2 + 51.3f, winSize.height / 2 - 122});

    auto deleteBtnSpr = ButtonSprite::create(CCSprite::createWithSpriteFrameName("edit_delBtn_001.png"), 25, true, 25.f, "GJ_button_04.png", 1.f);

    auto deleteBtn = geode::Button::createWithNode(deleteBtnSpr, [this] (auto sender) {
        int selCount = 0;
        int folderCount = 0;
        for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
            if (child->isSelected()) {
                selCount++;
                if (child->isFolder()) {
                    folderCount++;
                }
            }
        }

        if (selCount == 0) {
            createQuickPopup("Nothing here...", "No levels or folders selected.", "OK", nullptr, nullptr);
        }
        else {
            int levelCount = selCount - folderCount;

            std::string desc;
            if (folderCount != 0 && levelCount == 0) {
                desc = fmt::format("Are you sure you want to <cr>delete</c> the <cy>{}</c> selected <cg>folders</c>?\n\nThis will <cr>delete</c> the <cy>contents</c> of each selected folder too!", folderCount);
            }
            else if (levelCount != 0 && folderCount == 0) {
                desc = fmt::format("Are you sure you want to <cr>delete</c> the <cy>{}</c> selected <cg>levels</c>?", levelCount);
            }
            else {
                desc = fmt::format("Are you sure you want to <cr>delete</c> the <cy>{}</c> selected <cg>levels</c> and <cy>{}</c> selected <cg>folders</c>?\n\nThis will <cr>delete</c> the <cy>contents</c> of each selected folder too!", levelCount, folderCount);
            }

            auto alert = createQuickPopup("Delete", desc.c_str(), "Back", "Delete", [this] (auto alert, bool selected) {
                if (selected) {
                    for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
                        if (child->isSelected()) {
                            child->deleteFile();
                        }
                    }

                    auto y = m_scrollLayer->m_contentLayer->getPositionY();
                    loadLevelsForPath(m_path);
                    m_scrollLayer->m_contentLayer->setPositionY(y);
                }
            });

            alert->m_button2->updateBGImage("GJ_button_06.png");
        }
    });

    deleteBtn->setScale(0.4f);

    deleteBtn->setPosition({winSize.width / 2 - listLayer->getContentWidth() / 2 + deleteBtn->getScaledContentWidth() / 2 + 25.f, winSize.height / 2 - 122});

    addChild(deleteBtn);

    auto transferButton = geode::Button::createWithSprite("transfer.png"_spr, [this] (auto sender) {
        int selCount = 0;
        int folderCount = 0;
        for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
            if (child->isSelected()) {
                selCount++;
                if (child->isFolder()) {
                    folderCount++;
                }
            }
        }

        if (selCount == 0) {
            createQuickPopup("Nothing here...", "No levels selected.", "OK", nullptr, nullptr);
        }
        else if (folderCount > 1) {
            createQuickPopup("Oops!", "Cannot transfer any folders to your save. Please only select levels.", "OK", nullptr, nullptr);
        }
        else {
            auto alert = createQuickPopup("Transfer", fmt::format("Are you sure you want to <cr>transfer</c> the <cy>{}</c> selected <cg>levels</c> to your save?", selCount), "Back", "Transfer", [this] (auto alert, bool selected) {
                if (selected) {
                    for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
                        auto level = child->getLevel();
                        if (child->isSelected()) {
                            level->setLocal(false);
                            std::error_code err;
                            std::filesystem::remove_all(level->getFilePath(), err);

                            LocalLevelManager::get()->m_localLevels->insertObject(level, 0);
                        }
                    }
                    LocalLevelManager::get()->save();

                    auto y = m_scrollLayer->m_contentLayer->getPositionY();
                    loadLevelsForPath(m_path);
                    m_scrollLayer->m_contentLayer->setPositionY(y);
                }
            });

            alert->m_button2->updateBGImage("GJ_button_06.png");
        }
    });

    transferButton->setScale(0.4f);

    transferButton->setPosition({winSize.width / 2 + listLayer->getContentWidth() / 2 - transferButton->getScaledContentWidth() / 2 - 25.f, winSize.height / 2 - 122});

    addChild(transferButton);

    if (!isRoot) {
        auto moveOutButton = geode::Button::createWithSpriteFrameName("gj_folderBtn_001.png", [this] (auto sender) {
            int selCount = 0;
            int folderCount = 0;
            for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
                if (child->isSelected()) {
                    selCount++;
                    if (child->isFolder()) {
                        folderCount++;
                    }
                }
            }
            if (selCount == 0) {
                createQuickPopup("Nothing here...", "No levels or folders selected.", "OK", nullptr, nullptr);
            }
            else {
                int levelCount = selCount - folderCount;

                std::string desc;
                if (folderCount != 0 && levelCount == 0) {
                    desc = fmt::format("Are you sure you want to <cb>move</c> the <cy>{}</c> selected <cg>folders</c> out of this folder?", folderCount);
                }
                else if (levelCount != 0 && folderCount == 0) {
                    desc = fmt::format("Are you sure you want to <cb>move</c> the <cy>{}</c> selected <cg>levels</c> out of this folder?", levelCount);
                }
                else {
                    desc = fmt::format("Are you sure you want to <cb>move</c> the <cy>{}</c> selected <cg>levels</c> and <cy>{}</c> selected <cg>folders</c> out of this folder?", levelCount, folderCount);
                }

                auto alert = createQuickPopup("Move", desc.c_str(), "Back", "Move", [this] (auto alert, bool selected) {
                    if (selected) {
                        for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
                            if (child->isSelected()) {
                                std::error_code err;

                                auto destination = m_path.parent_path() / child->getPath().filename();
                                auto checkedDest = local_levels::utils::getNextAvailableName(destination);

                                std::filesystem::rename(child->getPath(), checkedDest, err);
                            }
                        }

                        auto y = m_scrollLayer->m_contentLayer->getPositionY();
                        loadLevelsForPath(m_path);
                        m_scrollLayer->m_contentLayer->setPositionY(y);
                    }
                });

                alert->m_button2->updateBGImage("GJ_button_03.png");
            }
        });

        moveOutButton->setScale(0.4f);

        moveOutButton->setPosition({winSize.width / 2 + listLayer->getContentWidth() / 2 - moveOutButton->getScaledContentWidth() / 2 - 51.3f, winSize.height / 2 - 122});

        auto arrowMoveSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
        arrowMoveSpr->setPosition({2.f, moveOutButton->getContentHeight() / 2});
        arrowMoveSpr->setScale(0.4f);

        moveOutButton->addChild(arrowMoveSpr);

        addChild(moveOutButton);
    }

    auto allLabel = CCLabelBMFont::create("All", "bigFont.fnt");
    allLabel->setScale(0.4f);

    allLabel->setPosition({allLabel->getScaledContentWidth() / 2 + menu->getPositionX() + menu->getScaledContentWidth() + 1.f, winSize.height / 2 - 122});

    addChild(allLabel);

    m_itemCountLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_itemCountLabel->setScale(0.515f);
    m_itemCountLabel->setAnchorPoint({1.f, 1.f});
    m_itemCountLabel->setPosition(winSize - CCPoint{7, 3});

    addChild(m_itemCountLabel);

    auto sortMenu = CCMenu::create();
    sortMenu->setContentSize({30, 120});
    sortMenu->setLayout(ScrollLayer::createDefaultListLayout());
    sortMenu->setAnchorPoint({1.f, 1.f});
    
    auto onDirSpr = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_sortIcon_001.png"), 25, true, 25.f, "GJ_button_02.png", 1.f);
    auto offDirSpr = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_sortIcon_001.png"), 25, true, 25.f, "GJ_button_01.png", 1.f);

    onDirSpr->setScale(0.7f);
    offDirSpr->setScale(0.7f);

    float offset = -1;

    onDirSpr->updateSpriteOffset({0, offset});
    offDirSpr->updateSpriteOffset({0, offset});

    auto dirToggler = CCMenuItemExt::createToggler(onDirSpr, offDirSpr, [this] (auto sender) {
        SortState::get()->setSortAscending(!sender->isToggled());
        loadLevelsForPath(m_path);
    });

    if (SortState::get()->getSortAscending()) {
        dirToggler->toggle(true);
    }

    sortMenu->addChild(dirToggler);

    auto spacer = CCNode::create();
    spacer->setContentSize({1, 10});

    sortMenu->addChild(spacer);

    auto onDateSpr = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png"), 25, true, 25.f, "GJ_button_02.png", 1.f);
    auto offDateSpr = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png"), 25, true, 25.f, "GJ_button_01.png", 1.f);

    onDateSpr->setScale(0.7f);
    offDateSpr->setScale(0.7f);

    onDateSpr->updateSpriteOffset({0, offset});
    offDateSpr->updateSpriteOffset({0, offset});

    auto dateToggler = CCMenuItemExt::createToggler(onDateSpr, offDateSpr, [this] (CCMenuItemToggler* sender) {
        SortState::get()->setSortMode(SortState::SortMode::Date);
        sender->toggle(true);
        sender->m_notClickable = true;
        for (auto btn : m_sortModeButtons) {
            if (btn != sender) {
                btn->m_notClickable = false;
                btn->toggle(false);
            }
        }
        loadLevelsForPath(m_path);
    });

    sortMenu->addChild(dateToggler);
    m_sortModeButtons.push_back(dateToggler);

    auto label = CCLabelBMFont::create("A", "bigFont.fnt");
    auto child = static_cast<CCSprite*>(label->getChildByIndex(0));

    auto onNameSpr = ButtonSprite::create(CCSprite::createWithTexture(child->getTexture(), child->getTextureRect()), 25, true, 25.f, "GJ_button_02.png", 1.f);
    auto offNameSpr = ButtonSprite::create(CCSprite::createWithTexture(child->getTexture(), child->getTextureRect()), 25, true, 25.f, "GJ_button_01.png", 1.f);

    onNameSpr->setScale(0.7f);
    offNameSpr->setScale(0.7f);

    onNameSpr->updateSpriteOffset({0, offset});
    offNameSpr->updateSpriteOffset({0, offset});

    auto nameToggler = CCMenuItemExt::createToggler(onNameSpr, offNameSpr, [this] (CCMenuItemToggler* sender) {
        SortState::get()->setSortMode(SortState::SortMode::Name);
        sender->toggle(true);
        sender->m_notClickable = true;
        for (auto btn : m_sortModeButtons) {
            if (btn != sender) {
                btn->m_notClickable = false;
                btn->toggle(false);
            }
        }
        loadLevelsForPath(m_path);
    });

    sortMenu->addChild(nameToggler);
    m_sortModeButtons.push_back(nameToggler);

    sortMenu->updateLayout();
    sortMenu->setPosition({winSize.width / 2 - listLayer->getContentWidth() / 2 - 20.f, winSize.height / 2 + 60.f});

    addChild(sortMenu);

    switch (SortState::get()->getSortMode()) {
        case SortState::SortMode::Date: {
            dateToggler->toggle(true);
            dateToggler->m_notClickable = true;
            break;
        }
        case SortState::SortMode::Name: {
            nameToggler->toggle(true);
            nameToggler->m_notClickable = true;
            break;
        }
    }
    
    loadLevelsForPath(path);

    return true;
}

void LocalLevelsLayer::setPathLabel() {
    std::string name;
    std::string pathText = "";
    if (m_path == Mod::get()->getSaveDir() / "levels") {
        name = "Root";
    }
    else {
        name = utils::string::pathToString(m_path.filename());
        auto mainPath = utils::string::pathToString(Mod::get()->getSaveDir() / "levels");
        pathText = "Root" + utils::string::replace(utils::string::pathToString(m_path.parent_path()).substr(mainPath.size()), "\\", " \\ ") + " \\ ";
    }

    m_nameLabel->setString(name.c_str());
    m_pathLabel->setString(pathText.c_str());

    m_pathContainer->updateLayout();

    m_nameLabel->setPositionY(m_nameLabel->getPositionY() + 2);
}

void LocalLevelsLayer::setTextPopupClosed(SetTextPopup* popup, gd::string text) {
    if (popup->getTag() == 1) {
        auto newPath = m_path / std::string(text);

        if (utils::string::trim(text).empty()) {
            createQuickPopup("Error", fmt::format("A folder must have a name."), "OK", nullptr, nullptr);
            return;
        }

        {
            std::error_code err;
            if (std::filesystem::exists(newPath, err)) {
                createQuickPopup("Error", fmt::format("A folder with name '{}' already exists.", text), "OK", nullptr, nullptr);
                return;
            }
        }

        {
            std::error_code err;
            std::filesystem::create_directory(newPath, err);

            if (err) log::error("{}", err.message());
            else loadLevelsForPath(m_path);
        }
    }
    else if (popup->getTag() == 2) {
        auto newPath = m_path.parent_path() / std::string(text);

        if (utils::string::trim(text).empty()) {
            createQuickPopup("Error", fmt::format("A folder must have a name."), "OK", nullptr, nullptr);
            return;
        }

        {
            std::error_code err;
            if (std::filesystem::exists(newPath, err)) {
                createQuickPopup("Error", fmt::format("A folder with name '{}' already exists.", text), "OK", nullptr, nullptr);
                return;
            }
        }

        {
            std::error_code err;
            std::filesystem::rename(m_path, newPath, err);

            if (err) log::error("{}", err.message());
            else loadLevelsForPath(newPath);
        }
    }
}

void LocalLevelsLayer::keyBackClicked() {
    CCLayer::keyBackClicked();
    CCDirector::get()->popScene();
    auto scene = LevelBrowserLayer::scene(GJSearchObject::create(SearchType::MyLevels));
    auto trans = CCTransitionFade::create(0.5f, scene);

    CCDirector::get()->replaceScene(trans);
}

void LocalLevelsLayer::loadLevelsForPath(const std::filesystem::path& path) {
    for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
        m_selectedPaths[child->getPath()] = child->isSelected();
    }

    m_scrollLayer->m_contentLayer->removeAllChildren();
    m_path = path;

    auto lower = geode::utils::string::toLower(m_search);

    std::vector<std::filesystem::directory_entry> entries;
    std::error_code ec;
    for (std::filesystem::directory_iterator it(path, ec), end; it != end && !ec; it.increment(ec)) {
        auto entry = *it;
        if (entry.is_directory() || utils::string::toLower(utils::string::pathToString(entry.path().extension())) == ".gmd") {
            entries.push_back(entry);
        }
    }

    if (!m_search.empty()) {
        struct Score {
            std::filesystem::directory_entry entry;
            int score;
        };

        std::vector<Score> entryScores;

        for (auto& entry : entries) {
            int score = 0;

            auto str = utils::string::pathToString(entry.path());
            utils::string::toLowerIP(str);

            if (!lower.empty() && !fts::fuzzy_match(lower.c_str(), str.c_str(), score)) continue;
            if (geode::utils::string::contains(str, lower)) { 
                entryScores.push_back({entry, score});
            }
        }

        entries.clear();
        if (!entryScores.empty()) {
            std::sort(entryScores.begin(), entryScores.end(), [&](const auto& a, const auto& b) {
                return a.score > b.score;
            });

            for (const auto& score : entryScores) {
                entries.push_back(score.entry);
            }
        }
    }
    else {
        std::sort(entries.begin(), entries.end(),
            [this](const std::filesystem::directory_entry& entryA, const std::filesystem::directory_entry& entryB) {
                switch (SortState::get()->getSortMode()) {
                    case SortState::SortMode::Date: {
                        std::error_code ec1, ec2;
                        auto a = entryA.last_write_time(ec1);
                        auto b = entryB.last_write_time(ec2);

                        if (ec1 && ec2) return false;
                        if (ec1) return false;
                        if (ec2) return true;

                        return SortState::get()->getSortAscending() ? a < b : a > b;
                    }
                    case SortState::SortMode::Name: {
                        auto a = utils::string::pathToString(entryA.path().filename());
                        auto b = utils::string::pathToString(entryB.path().filename());

                        utils::string::toLowerIP(a);
                        utils::string::toLowerIP(b);

                        return SortState::get()->getSortAscending() ? a > b : a < b;
                    }
                }
                return false;
            }
        );
    }

    for (const auto& entry : entries) {
        m_scrollLayer->m_contentLayer->addChild(LocalLevelCell::create(entry.path(), this));
    }

    m_scrollLayer->m_contentLayer->updateLayout();
    m_scrollLayer->scrollToTop();

    bool even = false;
    for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
        child->setColor(even ? ccColor3B{80, 80, 80} : ccColor3B{0, 0, 0});
        child->setSelected(m_selectedPaths[child->getPath()]);
        even = !even;
    }

    m_itemCountLabel->setString(fmt::format("{} items", entries.size()).c_str());

    setPathLabel();
    checkFolders();
}

void LocalLevelsLayer::checkFolders() {
    for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
        if (child->isFolder()) {
            child->checkMoveAllowed();
        }
    }
}

std::vector<LocalLevelCell*> LocalLevelsLayer::getSelectedCells() {
    std::vector<LocalLevelCell*> selected;

    for (auto child : m_scrollLayer->m_contentLayer->getChildrenExt<LocalLevelCell>()) {
        if (child->isSelected()) {
            selected.push_back(child);
        }
    }

    return selected;
}