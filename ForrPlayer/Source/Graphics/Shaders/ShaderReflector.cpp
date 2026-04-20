/*===============================================

    Forr Engine

    File : ShaderReflector.cpp
    Role : creates resource::Shader's layout part

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Graphics/Shaders/ShaderReflector.hpp"

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include "spirv_reflect.h"

using namespace fe::resource;

namespace fe {
    static Shader::Property::Type convertType(const SpvReflectTypeDescription* type);
    static void                   parseMember(resource::Shader& shader, const SpvReflectBlockVariable& block);
} // namespace fe

void fe::ShaderReflector::Reflect(resource::Shader& shader, bool& is_valid) {
    is_valid = false;

    SpvReflectShaderModule module{};
    spvReflectCreateShaderModule(shader.source_code.size() * sizeof(uint32_t), shader.source_code.data(), &module);

    uint32_t count{};
    spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);

    std::vector<SpvReflectDescriptorBinding*> bindings(count);
    spvReflectEnumerateDescriptorBindings(&module, &count, bindings.data());

    for (auto* binding : bindings) {
        if (binding->descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
            binding->descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            continue;
        }

        auto& block = binding->block;

        parseMember(shader, block);

        constexpr static std::string_view global_scene_data_name = "SceneData";

        if (std::string_view(block.type_description->type_name) == global_scene_data_name) { // TODO : improve this checking
            is_valid = true;
        }
    }

    spvReflectDestroyShaderModule(&module);
}

namespace fe {
    Shader::Property::Type convertType(const SpvReflectTypeDescription* type) {
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
            // clang-format off
        switch (type->traits.numeric.vector.component_count) {
            case 1: return Shader::Property::Type::FLOAT;
            case 2: return Shader::Property::Type::VEC2;
            case 3: return Shader::Property::Type::VEC3;
            case 4: return Shader::Property::Type::VEC4;
        }
            // clang-format on
        }

        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_INT) {
            return Shader::Property::Type::INT;
        }

        if (type->traits.numeric.matrix.column_count > 0) {
            return Shader::Property::Type::MAT4;
        }

        return Shader::Property::Type::FLOAT;
    }

    void parseMember(resource::Shader& shader, const SpvReflectBlockVariable& block) {
        for (uint32_t i = 0; i < block.member_count; i++) {
            const auto& member = block.members[i];

            if (member.member_count > 0) { // parse structs
                parseMember(shader, member);
            }
            else {
                Shader::Property property{};
                property.offset = member.offset;
                property.size   = member.size;
                property.type   = convertType(member.type_description);

                shader.properties.insert({ member.name, property });
            }
        }
    }
} // namespace fe
