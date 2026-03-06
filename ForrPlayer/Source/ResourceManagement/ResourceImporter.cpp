/*===============================================

    Forr Engine

    File : ResourceImporter.cpp
    Role : imports resources and their metadata

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ResourceManagement/ResourceImporter.hpp"

void fe::ResourceImporter::UploadResource(const std::filesystem::path& resource_relative_path) {
    std::filesystem::path resource_full_path = PATH.getResourcesPath() / resource_relative_path;
    std::filesystem::path extension          = resource_full_path.extension();

    if (extension == ".png") {
        this->UploadTexture(resource_full_path);
    }
}

void fe::ResourceImporter::UploadTexture(const std::filesystem::path& resource_full_path) {
    std::filesystem::path metadata_full_path = resource_full_path.wstring() + L".fs"; // TODO : write this to some desc

}
