/*===============================================

    Forr Engine

    File : ShaderImporter.cpp
    Role : imports resources and their metadata. for spv

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderImporter.hpp"

#include <fstream>

using namespace fe::resource;

fe::pointer<fe::resource::Shader> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    Shader              shader{};
    ShaderImportContext context{ shader, storage, resource_full_path };

    bool result = ShaderImporter::loadSourceCode(context);
    if (!result) return {};
    ShaderImporter::loadProperties(context);

    auto ptr = storage.CreateResource(std::move(shader));
    return ptr;
}

bool fe::ShaderImporter::loadSourceCode(ShaderImportContext& context) {
    std::ifstream file(context.resource_full_path, std::ios::binary | std::ios::ate);
    if (!file.good()) {
        fe::logging::error("SPR-V -> Unified. Failed to open shader file\nPath : %s", context.resource_full_path.string().c_str());
        return false;
    }

    std::streampos file_size;

    file.seekg(0, std::ios::end);
    file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    context.shader.source_code.resize(file_size);
    file.read((char*) &context.shader.source_code[0], file_size);

    return true;
}

void fe::ShaderImporter::loadProperties(ShaderImportContext& context) {
    spvReflectCreateShaderModule(context.shader.source_code.size(),
                                 context.shader.source_code.data(),
                                 &context.module);

    uint32_t count{};
    spvReflectEnumerateDescriptorBindings(&context.module, &count, nullptr);

    std::vector<SpvReflectDescriptorBinding*> bindings(count);
    spvReflectEnumerateDescriptorBindings(&context.module, &count, bindings.data());

    for (auto* binding : bindings) {
        if (binding->descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
            binding->descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            continue;
        }

        auto& block = binding->block;

        for (uint32_t i = 0; i < block.member_count; i++) {
            const auto& member = block.members[i];

            if (member.member_count > 0) { // parse structs
                ShaderImporter::parseMember(context, member);
            }
            else {
                Property property{};
                property.offset = member.offset;
                property.size   = member.size;
                property.type   = ShaderImporter::convertType(member.type_description);

                context.shader.properties.insert({ member.name, property });
            }
        }
    }

    spvReflectDestroyShaderModule(&context.module);
}

void fe::ShaderImporter::parseMember(ShaderImportContext& context, const SpvReflectBlockVariable& block) {
    for (uint32_t i = 0; i < block.member_count; i++) {
        const auto& member = block.members[i];

        if (member.member_count > 0) { // parse structs
            ShaderImporter::parseMember(context, member);
        }
        else {
            Property property{};
            property.offset = member.offset;
            property.size   = member.size;
            property.type   = ShaderImporter::convertType(member.type_description);

            context.shader.properties.insert({ member.name, property });
        }
    }
}

fe::ShaderImporter::PropertyType fe::ShaderImporter::convertType(const SpvReflectTypeDescription* type) {
    if (type->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
        // clang-format off
        switch (type->traits.numeric.vector.component_count) {
            case 1: return PropertyType::FLOAT;
            case 2: return PropertyType::VEC2;
            case 3: return PropertyType::VEC3;
            case 4: return PropertyType::VEC4;
        }
        // clang-format on
    }

    if (type->type_flags & SPV_REFLECT_TYPE_FLAG_INT) {
        return PropertyType::INT;
    }

    if (type->traits.numeric.matrix.column_count > 0) {
        return PropertyType::MAT4;
    }

    return PropertyType::FLOAT;
}
