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
#include "ResourceCreator.hpp"

namespace fe {
    class ResourceManager {
    public:
        ResourceManager()  = default;
        ~ResourceManager() = default;

        void SetupSceneResources(const std::vector<std::filesystem::path>& resource_relative_paths);

        template <typename T>
        FORR_NODISCARD T* GetResource(fe::pointer<T> ptr) { return m_Storage.GetResource(ptr); }

        template <typename T, typename Func>
        void RunForEach(Func&& func) { m_Storage.RunForEach<T>(func); }

    private:
        ResourceStorage  m_Storage{};
        ResourceImporter m_Importer{ m_Storage };
        ResourceCreator m_Creator{ m_Storage };
    };
} // namespace fe
