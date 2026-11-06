/*===============================================

    Forr Engine

    File : path.hpp
    Role : gives you full paths to any folder

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <string>
#include <string_view>
#include <filesystem>

#include "attributes.hpp"

namespace fe {
    class FORR_API PathManager {
    public:
        FORR_CLASS_NONCOPYABLE(PathManager)

        void init(std::string_view executable_path, bool is_editor);

        static PathManager& Instance()noexcept {
            static PathManager path_manager;
            return path_manager;
        }

        FORR_NODISCARD FORR_FORCE_INLINE const std::filesystem::path& getExecutablePath() const noexcept {
            return m_ExecutablePath;
        }

        FORR_NODISCARD FORR_FORCE_INLINE const std::filesystem::path& getAssetsPath() const noexcept {
            return m_AssetsPath;
        }

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getEngineAssetsPath() const {
            return m_AssetsPath / u8"Engine";
        }

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getUserAssetsPath() const {
            return m_AssetsPath / u8"User";
        }

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getSharedAssetsPath() const {
            return m_AssetsPath / u8"Shared";
        }

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getEngineResourcesPath() const {
            return this->getEngineAssetsPath() / u8"Resources";
        }

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getModelsPath() const {
            return this->getEngineResourcesPath() / u8"Models";
        }

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getShadersPath() const {
            return this->getEngineResourcesPath() / u8"Shaders";
        }

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getDefaultShadersPath() const {
            return this->getShadersPath() / u8"Default";
        }

        //

        FORR_NODISCARD FORR_STATIC FORR_CONSTEXPR std::u8string_view getMetadataExtension() { return reinterpret_cast<const char8_t*>(u8".forr_meta"); }
        FORR_NODISCARD FORR_STATIC FORR_CONSTEXPR std::u8string_view getMaterialExtension() { return reinterpret_cast<const char8_t*>(u8".forr_material"); }
        FORR_NODISCARD FORR_STATIC FORR_CONSTEXPR std::u8string_view getShaderExtension() { return reinterpret_cast<const char8_t*>(u8".slang"); }

    private:
        PathManager()  = default;
        ~PathManager() = default;

        static void copy_if_new(const std::filesystem::path& from, const std::filesystem::path& to);

        std::filesystem::path m_ExecutablePath{};
        std::filesystem::path m_AssetsPath{};
    };

    FORR_INLINE FORR_STATIC PathManager& PATH = PathManager::Instance();

} // namespace fe
