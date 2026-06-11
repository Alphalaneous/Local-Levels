#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class SortState {
public:
    enum class SortMode {
        Date,
        Name
    };

    static SortState* get();

    void setSortAscending(bool ascending);
    bool getSortAscending();

    void setSortMode(SortMode mode);
    SortMode getSortMode();

protected:
    bool m_sortAscending = false;
    SortMode m_sortMode = SortMode::Date;
};