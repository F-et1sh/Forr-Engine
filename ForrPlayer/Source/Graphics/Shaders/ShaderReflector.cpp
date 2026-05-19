/*===============================================

    Forr Engine

    File : ShaderReflector.cpp
    Role : creates resource::Shader's layout part

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Graphics/Shaders/ShaderReflector.hpp"

#include <array>

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include "spirv_reflect.h"

using namespace fe::resource;

namespace fe { // this functions are not a part of fe::ShaderReflector, because they're using spirv_reflect's structures that cannot be in headers
    static Shader::DescriptorType convertType(SpvReflectDescriptorType type);
} // namespace fe

void fe::ShaderReflector::Reflect(resource::Shader& shader, const std::filesystem::path& resource_full_path) {
    SpvReflectShaderModule module{};
    spvReflectCreateShaderModule(shader.source_code.size() * sizeof(uint32_t), shader.source_code.data(), &module);

    // read sets
    uint32_t sets_count{};
    spvReflectEnumerateDescriptorSets(&module, &sets_count, nullptr);
    std::vector<SpvReflectDescriptorSet*> sets(sets_count);
    spvReflectEnumerateDescriptorSets(&module, &sets_count, sets.data());

    for (auto* set : sets) {
        auto& descriptor_set_layout_data = shader.descriptor_sets.emplace_back();

        descriptor_set_layout_data.index = set->set;
        descriptor_set_layout_data.bindings.reserve(set->binding_count);

        for (uint32_t i = 0; i < set->binding_count; i++) {
            auto* binding      = set->bindings[i];
            auto& this_binding = descriptor_set_layout_data.bindings.emplace_back();

            this_binding.index           = binding->binding;
            this_binding.descriptor_type = convertType(binding->descriptor_type);
            this_binding.is_array        = (binding->array.dims_count > 0);

            if (this_binding.is_array) { // bindless
                uint32_t reflected_count = binding->array.dims[0];
                this_binding.count       = (reflected_count == 0) ? 100'000 : reflected_count;
            }
            else this_binding.count = 1;

            if (!binding->block.member_count) continue;

            this_binding.size = binding->block.size;
            this_binding.members.reserve(binding->block.member_count);

            for (uint32_t member_i = 0; member_i < binding->block.member_count; member_i++) {
                auto& member = binding->block.members[member_i];
                this_binding.members.push_back(Shader::BlockMember{ member.offset, member.size, member.padded_size });
            }
        }
    }

    spvReflectDestroyShaderModule(&module);
}

namespace fe {
    Shader::DescriptorType convertType(SpvReflectDescriptorType type) {
        // clang-format off
        switch (type) {
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER                : return Shader::DescriptorType::SAMPLER                ; break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : return Shader::DescriptorType::COMBINED_IMAGE_SAMPLER ; break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE          : return Shader::DescriptorType::SAMPLED_IMAGE          ; break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE          : return Shader::DescriptorType::STORAGE_IMAGE          ; break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   : return Shader::DescriptorType::UNIFORM_TEXEL_BUFFER   ; break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   : return Shader::DescriptorType::STORAGE_TEXEL_BUFFER   ; break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER         : return Shader::DescriptorType::UNIFORM_BUFFER         ; break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER         : return Shader::DescriptorType::STORAGE_BUFFER         ; break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : return Shader::DescriptorType::UNIFORM_BUFFER_DYNAMIC ; break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : return Shader::DescriptorType::STORAGE_BUFFER_DYNAMIC ; break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT       : return Shader::DescriptorType::INPUT_ATTACHMENT       ; break;
            default:
                assert(false);
        }
        // clang-format on

        return Shader::DescriptorType::SAMPLER;
    }
} // namespace fe
