/*===============================================

    Forr Engine

    File : MaterialImporter.cpp
    Role : imports resources and their metadata. for forr_material

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "MaterialImporter.hpp"

#include <fstream>

fe::pointer<fe::resource::Material> fe::MaterialImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    resource::Material this_material{};

    // TODO : provide material serializing and then this - importing

    // ( I don't wanna make any serialization without C++26 )

    this_material.color               = glm::vec3{ 0.76f, 0.67f, 0.52f };
    this_material.vertex_shader_ptr   = fe::pointer<resource::Shader>{ 0, 0 }; // this should work through guids but I don't use them yet
    this_material.fragment_shader_ptr = fe::pointer<resource::Shader>{ 1, 0 }; // this should work through guids but I don't use them yet

    auto ptr = storage.CreateResource<resource::Material>(std::move(this_material));
    return ptr;
}
