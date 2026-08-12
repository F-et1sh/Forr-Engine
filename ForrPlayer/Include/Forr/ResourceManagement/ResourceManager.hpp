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
        // this needs to compile shaders
        GraphicsBackend graphics_backend{};

        ResourceManagerDesc()  = default;
        ~ResourceManagerDesc() = default;
    };

    class ResourceManager {
    public:
        ResourceManager(const ResourceManagerDesc& desc);
        ~ResourceManager() = default;

        void CreateDefaultResources();
        void SetupSceneResources(const std::vector<std::filesystem::path>& resource_full_paths);

        template <resource::resource_t T>
        FORR_NODISCARD fe::pointer<T> ImportResource(const std::filesystem::path& resource_full_path) {
            return m_Importer.ImportResource<T>(resource_full_path);
        }

        template <resource::resource_t T>
        FORR_NODISCARD fe::pointer<T> CreateResource(const T& value) {
            return m_Storage.CreateResource(value);
        }

        template <resource::resource_t T>
        FORR_NODISCARD fe::pointer<T> CreateResource(T&& value) {
            return m_Storage.CreateResource(std::move(value));
        }

        template <resource::resource_t T>
        FORR_NODISCARD fe::pointer<T> CreateResource()
            requires std::default_initializable<T>
        {
            return m_Storage.CreateResource();
        }

        template <resource::resource_t T>
        FORR_NODISCARD T* GetResource(fe::pointer<T> ptr) { return m_Storage.GetResource(ptr); }

        // TODO : add 'FindResource()'

        template <resource::resource_t T, typename Func>
        void RunForEach(Func&& func) { m_Storage.RunForEach<T>(func); }

        FORR_NODISCARD const ResourceManagementContext& GetContext() const noexcept { return m_Context; }

    private:
        ResourceManagementContext m_Context{};

        ResourceImporter m_Importer{ m_Context, m_Storage };
        ResourceCreator  m_Creator{ m_Context, m_Storage, m_Importer };
        ResourceStorage  m_Storage{ m_Context };
    };
} // namespace fe
