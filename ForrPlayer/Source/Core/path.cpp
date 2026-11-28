/*===============================================

    Forr Engine

    File : path.cpp
    Role : structures and unifies work with paths
        Based on For Engine 2D. MIT License ( https://github.com/F-et1sh/For-Engine-2D )

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/Core/path.hpp"

#include "Forr/Core/logging.hpp"

void fe::path_manager::init(std::string_view executable_path, bool is_editor) {
    if (executable_path.empty() || executable_path.data() == nullptr) {
        fe::logging::error("Failed to initialize 'fe::path_manager'. No arguments");
    }

    try {
        m_executable_path                               = executable_path;
        std::filesystem::path executable_directory_path = std::filesystem::canonical(m_executable_path).parent_path();

        std::filesystem::path solution_path =
            executable_directory_path.parent_path(). // exit from the platform folder
            parent_path();                           // exit from the configuration folder

        static std::filesystem::path assets_folder_name = L"Files";

        std::filesystem::path editor_assets_folder_path      = solution_path / assets_folder_name;
        std::filesystem::path application_assets_folder_path = executable_directory_path / assets_folder_name;

        if (is_editor) {
            m_assets_path = editor_assets_folder_path;

            if (!std::filesystem::exists(m_assets_path)) {
                m_assets_path = application_assets_folder_path;
            }
        }
        else {
            m_assets_path = application_assets_folder_path;
        }

        if (std::filesystem::exists(editor_assets_folder_path)) {

            std::filesystem::remove_all(application_assets_folder_path);
            path_manager::copy_if_new(editor_assets_folder_path, application_assets_folder_path);
        }

        m_temp_directory = this->create_temp_directory();
    }
    catch (const std::exception& e) {
        fe::logging::error("Failed to initialize 'fe::path_manager'. %s", e.what());
    }
}

void fe::path_manager::copy_if_new(const std::filesystem::path& from, const std::filesystem::path& to) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(from)) {
        auto relative_path = std::filesystem::relative(entry.path(), from);
        auto target_path   = to / relative_path;

        if (entry.is_directory()) {
            std::filesystem::create_directories(target_path);
        }
        else if (!std::filesystem::exists(target_path) ||
                 std::filesystem::last_write_time(entry) < std::filesystem::last_write_time(target_path)) {
            std::filesystem::copy_file(entry, target_path, std::filesystem::copy_options::overwrite_existing);
        }
    }
}

std::filesystem::path fe::path_manager::create_temp_directory() const {
    std::filesystem::path application_name = m_executable_path.stem();
    std::filesystem::path temp_directory   = std::filesystem::temp_directory_path() / application_name;

    if (std::filesystem::exists(temp_directory) &&
        std::filesystem::is_directory(temp_directory)) {
        return temp_directory;
    }

    bool temp_directory_result = std::filesystem::create_directory(temp_directory);
    if (temp_directory_result) {
        return temp_directory;
    }

    fe::logging::warning("Failed to create temp directory in 'fe::path_manager'. Using the local temp directory in assets. \nPath : %s",
                         temp_directory.string().c_str());

    std::filesystem::path local_temp_directory = m_assets_path / "temp";

    if (std::filesystem::exists(local_temp_directory) &&
        std::filesystem::is_directory(local_temp_directory)) {
        return local_temp_directory;
    }

    bool local_temp_directory_result = std::filesystem::create_directory(local_temp_directory);
    if (local_temp_directory_result) {
        return local_temp_directory;
    }

    fe::logging::error("Failed to create local temp directory in 'fe::path_manager'. \nPath : %s",
                       local_temp_directory.string().c_str());
    return {};
}

FORR_API void fe::path_manager::remove_temp_directory() const {
    bool temp_directory_result = std::filesystem::remove(m_temp_directory);
    if (!temp_directory_result) {
        fe::logging::warning("Failed to remove temp directory\nPath : %s", m_temp_directory.string().c_str());
    }
}
