/*===============================================

    Forr Engine

    File : ResourceCreator.hpp
    Role : creates engine-specific resources in the explorer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceStorage.hpp"

namespace fe {
    class FORR_API ResourceCreator {
    public:
        ResourceCreator(ResourceStorage& storage) : m_Storage(storage) {}
        ~ResourceCreator() = default;

        void CreateMaterial(fe::pointer<resource::Material> pointer, const std::filesystem::path& resource_full_path);

    private:
        void createMetadata(const std::filesystem::path& resource_full_path);

    private:
        ResourceStorage& m_Storage;
    };
} // namespace fe
