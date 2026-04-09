/*===============================================

    Forr Engine

    File : ResourceCreator.hpp
    Role : creates engine-specific resources in the explorer
        
        I don't want to create any serialization till C++26

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/logging.hpp"
#include "Core/path.hpp"

#include "ResourceStorage.hpp"

#include <fstream>

namespace fe {
    class FORR_API ResourceCreator {
    public:
        ResourceCreator(ResourceStorage& storage) : m_Storage(storage) {}
        ~ResourceCreator() = default;

        void CreateDefaultResources();

        void CreateMaterial(fe::pointer<resource::Material> pointer, const std::filesystem::path& resource_relative_path);

        template <typename T>
        void CreateMeta(fe::pointer<T> pointer, const std::filesystem::path& resource_relative_path) {
            std::filesystem::path resource_full_path = PATH.getResourcesPath() / resource_relative_path;
            std::filesystem::path metadata_path      = resource_full_path += PATH.getMetadataExtension().wstring();

            std::ofstream file(metadata_path);
            if (!file.good()) {
                fe::logging::error("Unified -> %s. Failed create metadata\nResource relative path : %s\nResource full path : %s\nMetadata full path : %s",
                                   metadata_path.extension().string().c_str(),
                                   resource_relative_path.string().c_str(),
                                   resource_full_path.string().c_str(),
                                   metadata_path);
                return;
            }

            if constexpr (std::is_same_v<T, resource::Material>) { // WG21, do C++26 faster plsss
                resource::Material& resource = *m_Storage.GetResource(pointer);

                for (const auto& property : resource.properties) {
                    //property.first
                }
            }
        }

    private:
        void createDefaultMaterials();

    private:
        ResourceStorage& m_Storage;
    };
} // namespace fe
