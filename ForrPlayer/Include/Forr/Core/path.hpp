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

        void init(std::string_view executable_path, bool is_editor);

        static PathManager& Instance() {
            static PathManager path_manager;
            return path_manager;
        }

        FORR_FORCE_INLINE FORR_NODISCARD const std::filesystem::path& getExecutablePath() const noexcept {
            return m_ExecutablePath;
        }

        FORR_FORCE_INLINE FORR_NODISCARD const std::filesystem::path& getAssetsPath() const noexcept {
            return m_AssetsPath;
        }

        FORR_FORCE_INLINE FORR_NODISCARD std::filesystem::path getEngineAssetsPath() const {
            return m_AssetsPath / L"Engine";
        }

        FORR_FORCE_INLINE FORR_NODISCARD std::filesystem::path getUserAssetsPath() const {
            return m_AssetsPath / L"User";
        }

        FORR_FORCE_INLINE FORR_NODISCARD std::filesystem::path getSharedAssetsPath() const {
            return m_AssetsPath / L"Shared";
        }

        FORR_FORCE_INLINE FORR_NODISCARD std::filesystem::path getEngineResourcesPath() const {
            return this->getEngineAssetsPath() / L"Resources";
        }

        FORR_FORCE_INLINE FORR_NODISCARD std::filesystem::path getModelsPath() const {
            return this->getEngineResourcesPath() / L"Models";
        }

        FORR_FORCE_INLINE FORR_NODISCARD std::filesystem::path getShadersPath() const {
            return this->getEngineResourcesPath() / L"Shaders";
        }

        FORR_FORCE_INLINE FORR_NODISCARD std::filesystem::path getDefaultShadersPath() const {
            return this->getShadersPath() / L"Default";
        }

        //

        FORR_STATIC FORR_NODISCARD FORR_CONSTEVAL std::wstring_view getMetadataExtension() { return L".forr_meta"; }
        FORR_STATIC FORR_NODISCARD FORR_CONSTEVAL std::wstring_view getMaterialExtension() { return L".forr_material"; }
        FORR_STATIC FORR_NODISCARD FORR_CONSTEVAL std::wstring_view getShaderExtension() { return L".slang"; }

    private:
        PathManager()  = default;
        ~PathManager() = default;

        static void copy_if_new(const std::filesystem::path& from, const std::filesystem::path& to);

        std::filesystem::path m_ExecutablePath;
        std::filesystem::path m_AssetsPath;
    };

    inline static PathManager& PATH = PathManager::Instance();

} // namespace fe
