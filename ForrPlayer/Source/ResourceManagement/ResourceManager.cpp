/*===============================================

    Forr Engine

    File : ResourceManager.cpp
    Role : Resource Management System

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ResourceManagement/ResourceManager.hpp"

void fe::ResourceManager::LoadResource(const std::filesystem::path& resource_relative_path) {
    this->m_Importer.StoreResource(resource_relative_path); // uploads resource to the storage


}

void fe::ResourceManager::SetupSceneResources(const std::vector<std::filesystem::path>& resource_relative_paths) {
    for (const auto& path : resource_relative_paths) {
        this->m_Importer.StoreResource(path); // uploads resource to the storage
    }
}
