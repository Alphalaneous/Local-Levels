#pragma once 

#include "GJGameLevel.hpp"
#include <Geode/Geode.hpp>
#include <Geode/ui/Button.hpp>

using namespace geode::prelude;

class LocalLevelsLayer;

class LocalLevelCell : public CCLayerColor {

public:
    static LocalLevelCell* create(const std::filesystem::path& path, LocalLevelsLayer* levelsLayer);
    bool isSelected();
    void setSelected(bool selected);
    bool isFolder();
    void deleteFile();

    void checkMoveAllowed();

    const std::filesystem::path& getPath();

protected:
    bool init(const std::filesystem::path& path, LocalLevelsLayer* levelsLayer);

    MyGJGameLevel* m_level;
    std::filesystem::path m_path;
    bool m_isFolder;
    CCMenuItemToggler* m_selectToggle;
    LocalLevelsLayer* m_levelsLayer;

    geode::Button* m_moveToFolderButton;
};