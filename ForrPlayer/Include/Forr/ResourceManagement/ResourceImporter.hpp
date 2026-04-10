/*===============================================

    Forr Engine

    File : ResourceImporter.hpp
    Role : imports resources and their metadata
            
    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagementContext.hpp"
#include "ResourceStorage.hpp"

namespace fe {
    class ResourceImporter {
    public:
        ResourceImporter(ResourceManagementContext& context, ResourceStorage& storage) 
            : m_Context(context), m_Storage(storage) {}
        ~ResourceImporter() = default;

        // upload resource to the storage
        void ImportResource(const std::filesystem::path& resource_full_path);

        // upload resource to the storage and get its pointer
        template<typename T>
        fe::pointer<T> ImportResource(const std::filesystem::path& resource_full_path);

    private:
        ResourceManagementContext& m_Context;
        ResourceStorage& m_Storage;
    };
} // namespace fe
