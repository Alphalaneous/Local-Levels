#include "EditorPauseLayer.hpp"
#include "GJGameLevel.hpp"
#include <hjfod.gmd-api/include/GMD.hpp>

void MyEditorPauseLayer::saveLevel() {
    EditorPauseLayer::saveLevel();

    auto level = static_cast<MyGJGameLevel*>(m_editorLayer->m_level);
    if (level->isLocal()) {
        auto res = gmd::exportLevelAsGmd(m_editorLayer->m_level, level->getFilePath());
        if (!res) log::error("{}", res.err());
        
        return;
    }
}