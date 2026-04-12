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

#include "ResourceManagementContext.hpp"
#include "ResourceStorage.hpp"
#include "ResourceImporter.hpp"

#include <fstream>

namespace fe {
    class FORR_API ResourceCreator {
    public:
        ResourceCreator(ResourceManagementContext& context, ResourceStorage& storage, ResourceImporter& importer)
            : m_Context(context), m_Storage(storage), m_Importer(importer) {}
        ~ResourceCreator() = default;

        void CreateDefaultResources();

        template <typename T>
        void CreateMeta(fe::pointer<T> pointer, const std::filesystem::path& resource_relative_path) {
            std::filesystem::path resource_full_path = PATH.getEngineResourcesPath() / resource_relative_path;
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
        void createDefaultShaders();
        void createDefaultMaterials();

    private:
        ResourceManagementContext& m_Context;
        ResourceStorage&           m_Storage;
        ResourceImporter&          m_Importer;
    };
} // namespace fe
