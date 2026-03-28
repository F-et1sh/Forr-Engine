/*===============================================

    Forr Engine

    File : ResourceCreator.cpp
    Role : creates engine-specific resources in the explorer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ResourceManagement/ResourceCreator.hpp"

#include <fstream>

void fe::ResourceCreator::CreateMaterial(fe::pointer<resource::Material> pointer, const std::filesystem::path& resource_full_path) {
    resource::Material& material = *m_Storage.GetResource(pointer);

    std::ofstream file{};
    file.open(resource_full_path);
}
