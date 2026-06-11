#include "GJGameLevel.hpp"

void MyGJGameLevel::setLocal(bool local) {
    auto fields = m_fields.self();
    fields->m_isLocal = local;
    fields->m_isUploaded = m_isUploaded;
}

bool MyGJGameLevel::isLocal() {
    return m_fields->m_isLocal;
}

void MyGJGameLevel::setFilePath(const std::filesystem::path& path) {
    m_fields->m_filePath = path;
}

const std::filesystem::path& MyGJGameLevel::getFilePath() {
    return m_fields->m_filePath;
}