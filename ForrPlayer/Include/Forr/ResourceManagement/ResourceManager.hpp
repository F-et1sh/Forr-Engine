/*===============================================

    Forr Engine

    File : ResourceManager.hpp
    Role : Resource Management System

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <filesystem>
#include "ResourceStorage.hpp"
#include "ResourceImporter.hpp"

namespace fe {
    class ResourceManager {
    public:
        ResourceManager()  = default;
        ~ResourceManager() = default;

        void SetupSceneResources(const std::vector<std::filesystem::path>& resource_paths);

    private:
        ResourceStorage  m_Storage{};
        ResourceImporter m_Importer{ m_Storage };
    };
} // namespace fe
