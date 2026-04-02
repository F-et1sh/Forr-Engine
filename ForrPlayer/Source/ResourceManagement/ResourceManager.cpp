/*===============================================

    Forr Engine

    File : ResourceManager.cpp
    Role : Resource Management System

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ResourceManagement/ResourceManager.hpp"

void fe::ResourceManager::SetupSceneResources(const std::vector<std::filesystem::path>& resource_full_paths) {
    for (const auto& path : resource_full_paths) {
        this->m_Importer.ImportResource(path); // uploads resource to the storage
    }
}
