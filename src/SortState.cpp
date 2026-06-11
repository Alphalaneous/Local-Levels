#include "SortState.hpp"

SortState* SortState::get() {
    static SortState state;
    return &state;
}

void SortState::setSortAscending(bool ascending) {
    m_sortAscending = ascending;
}

bool SortState::getSortAscending() {
    return m_sortAscending;
}

void SortState::setSortMode(SortMode mode) {
    m_sortMode = mode;
}

SortState::SortMode SortState::getSortMode() {
    return m_sortMode;
}