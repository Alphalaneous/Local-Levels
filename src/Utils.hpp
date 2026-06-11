#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace local_levels::utils {
    
    inline std::filesystem::path getNextAvailableName(const std::filesystem::path& path) {
        std::error_code err;

        if (!std::filesystem::exists(path, err)) {
            return path;
        }

        int i = 2;

        auto parent = path.parent_path();
        auto stem = geode::utils::string::pathToString(path.stem());
        auto ext = geode::utils::string::pathToString(path.extension());

        for (int i = 2;; ++i) {
            auto candidate = parent / fmt::format("{} ({}){}", stem, i, ext);

            if (!std::filesystem::exists(candidate, err)) {
                return candidate;
            }
        }
    }
}