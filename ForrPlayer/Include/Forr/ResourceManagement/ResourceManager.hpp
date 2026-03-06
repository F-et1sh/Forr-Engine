/*===============================================

    Forr Engine

    File : ResourceManager.hpp
    Role : Resource Management System

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <filesystem>
#include "Core/pointer.hpp"
#include "ResourceImporter.hpp"

namespace fe {
    class ResourceManager {
    public:
        ResourceManager()  = default;
        ~ResourceManager() = default;

        void SetupSceneResources(const std::vector<std::filesystem::path>& resource_paths);

    private:
        ResourceImporter m_Importer{};

        //typed_pointer_storage<Mesh> m_StorageMesh{};
    };
} // namespace fe
