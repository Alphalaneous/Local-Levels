#pragma once

#include "LocalLevelCell.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LocalLevelCell;

class LocalLevelsLayer : public CCLayer, public SetTextPopupDelegate {

public:
    static LocalLevelsLayer* create(const std::filesystem::path& path);

    void checkFolders();
    std::vector<LocalLevelCell*> getSelectedCells();

protected:

    bool init(const std::filesystem::path& path);
    void keyBackClicked() override;

    void loadLevelsForPath(const std::filesystem::path& path);
    void setTextPopupClosed(SetTextPopup* popup, gd::string text) override;

    void setPathLabel();

    geode::ScrollLayer* m_scrollLayer;
    std::filesystem::path m_path;
    CCLabelBMFont* m_itemCountLabel;

    CCLabelBMFont* m_pathLabel;
    CCLabelBMFont* m_nameLabel;

    CCNode* m_pathContainer;

    std::string m_search;
    
    std::unordered_map<std::filesystem::path, bool> m_selectedPaths;
    std::vector<CCMenuItemToggler*> m_sortModeButtons;
};