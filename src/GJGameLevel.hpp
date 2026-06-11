#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/GJGameLevel.hpp>

using namespace geode::prelude;

class $modify(MyGJGameLevel, GJGameLevel) {

    struct Fields {
        bool m_isLocal = false;
        std::filesystem::path m_filePath;
        bool m_isUploaded;
    };

    void setLocal(bool local);
    bool isLocal();

    void setFilePath(const std::filesystem::path& path);
    const std::filesystem::path& getFilePath();
};
