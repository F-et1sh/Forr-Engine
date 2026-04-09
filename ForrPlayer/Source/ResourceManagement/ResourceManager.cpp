/*===============================================

    Forr Engine

    File : ResourceManager.cpp
    Role : resource management system

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ResourceManagement/ResourceManager.hpp"

void fe::ResourceManager::CreateDefaultResources() {
    m_Creator.CreateDefaultResources();
}

void fe::ResourceManager::SetupSceneResources(const std::vector<std::filesystem::path>& resource_full_paths) {
    for (const auto& path : resource_full_paths) {
        m_Importer.ImportResource(path); // uploads resource to the storage
    }
}
