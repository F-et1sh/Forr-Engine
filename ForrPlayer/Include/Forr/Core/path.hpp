/*===============================================

    Forr Engine

    File : path.hpp
    Role : structures and unifies work with paths
        Based on For Engine 2D. MIT License ( https://github.com/F-et1sh/For-Engine-2D )

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include <string>
#include <string_view>
#include <filesystem>

#include "attributes.hpp"

namespace fe {
    // TODO : this class does not create all this directories,
    // so if somebody will delete the original 'Files' aka assets path
    // all this structure will be gone
    class FORR_API path_manager {
    public:
        FORR_CLASS_NONCOPYABLE(path_manager)

        void init(std::string_view executable_path, bool is_editor);

        static path_manager& Instance() noexcept {
            static path_manager this_path_manager;
            return this_path_manager;
        }

        FORR_NODISCARD FORR_FORCE_INLINE const std::filesystem::path& getExecutablePath() const noexcept {
            return m_executable_path;
        }

        FORR_NODISCARD FORR_FORCE_INLINE const std::filesystem::path& getAssetsPath() const noexcept {
            return m_assets_path;
        }

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getEngineAssetsPath() const {
            return m_assets_path / u8"Engine";
        }

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getUserAssetsPath() const {
            return m_assets_path / u8"User";
        }

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getSharedAssetsPath() const {
            return m_assets_path / u8"Shared";
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

        //

        FORR_NODISCARD FORR_FORCE_INLINE std::filesystem::path getTempDirectory() {
            if (m_temp_directory.empty()) {
                m_temp_directory = this->create_temp_directory();
            }
            return m_temp_directory;
        }

    private:
        path_manager() = default;
        ~path_manager() { this->remove_temp_directory(); }

        static void    copy_if_new(const std::filesystem::path& from, const std::filesystem::path& to);
        FORR_NODISCARD std::filesystem::path create_temp_directory() const;
        void                                 remove_temp_directory() const;

        std::filesystem::path m_executable_path{};
        std::filesystem::path m_assets_path{};
        std::filesystem::path m_temp_directory{};
    };

    FORR_INLINE FORR_STATIC path_manager& PATH = path_manager::Instance();

} // namespace fe
