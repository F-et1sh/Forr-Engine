/*===============================================

    Forr Engine

    File : path.hpp
    Role : gives you full paths to any folder

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <filesystem>

namespace fe {
    class PathManager {
    public:
        FORR_CLASS_NONCOPYABLE(PathManager)

        void init(const char* argv0, bool is_editor);

        static PathManager& Instance() {
            static PathManager path_manager;
            return path_manager;
        }

        [[nodiscard]] const std::filesystem::path& getExecutablePath() const noexcept {
            return m_ExecutablePath;
        }

        [[nodiscard]] const std::filesystem::path& getAssetsPath() const noexcept {
            return m_AssetsPath;
        }

        [[nodiscard]] std::filesystem::path getShadersPath() const {
            return this->getAssetsPath() / L"Shaders";
        }

        [[nodiscard]] std::filesystem::path getResourcesPath() const noexcept {
            return m_AssetsPath / L"Resources";
        }

    private:
        PathManager()  = default;
        ~PathManager() = default;

        static void copy_if_new(const std::filesystem::path& from, const std::filesystem::path& to);

        std::filesystem::path m_ExecutablePath;
        std::filesystem::path m_AssetsPath;
    };

    inline static PathManager& PATH = PathManager::Instance();

} // namespace fe
