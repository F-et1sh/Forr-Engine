/*===============================================

    Forr Engine

    File : ResourceManager.hpp
    Role : resource management system

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <filesystem>
#include "ResourceManagementContext.hpp"
#include "ResourceStorage.hpp"
#include "ResourceImporter.hpp"
#include "ResourceCreator.hpp"

namespace fe {
    struct ResourceManagerDesc {
        GraphicsBackend graphics_backend{};

        ResourceManagerDesc()  = default;
        ~ResourceManagerDesc() = default;
    };

    class ResourceManager {
    public:
        ResourceManager(ResourceManagerDesc desc);
        ~ResourceManager() = default;

        void CreateDefaultResources();
        void SetupSceneResources(const std::vector<std::filesystem::path>& resource_full_paths);

        template <typename T>
        FORR_NODISCARD fe::pointer<T> ImportResource(const std::filesystem::path& resource_full_path) {
            return m_Importer.ImportResource<T>(resource_full_path);
        }

        template <typename T>
        FORR_NODISCARD fe::pointer<T> CreateResource(const T& value) {
            return m_Storage.CreateResource(value);
        }

        template <typename T>
        FORR_NODISCARD fe::pointer<T> CreateResource(T&& value) {
            return m_Storage.CreateResource(std::move(value));
        }

        template <typename T>
        FORR_NODISCARD fe::pointer<T> CreateResource()
            requires std::default_initializable<T>
        {
            return m_Storage.CreateResource();
        }

        template <typename T>
        FORR_NODISCARD T* GetResource(fe::pointer<T> ptr) { return m_Storage.GetResource(ptr); }

        template <typename T, typename Func>
        void RunForEach(Func&& func) { m_Storage.RunForEach<T>(func); }

    private:
        ResourceManagementContext m_Context{};

        ResourceImporter m_Importer{ m_Context, m_Storage };
        ResourceCreator  m_Creator{ m_Context, m_Storage };
        ResourceStorage  m_Storage{ m_Context };
    };
} // namespace fe
