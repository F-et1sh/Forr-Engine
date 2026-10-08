/*===============================================

    Forr Engine

    File : ShaderReflection.hpp
    Role : structs to handle shaders reflection

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <cinttypes>
#include <vector>
#include <unordered_map>
#include <variant>

#include "Core/attributes.hpp"
#include "Core/string.hpp"

namespace fe::shader {
    enum class DescriptorType : std::uint8_t {
        UNIFORM_BUFFER,
        STORAGE_BUFFER,

        SAMPLED_IMAGE,
        SAMPLER,
        COMBINED_IMAGE_SAMPLER,
        STORAGE_IMAGE,

        ACCELERATION_STRUCTURE,

        GENERIC,

        UNKNOWN
    };

    // clang-format off
    enum class ValueType : std::uint8_t {
        VOID,
    
        BOOL,
    
        INT32, UINT32,
    
        INT64, UINT64,
    
        FLOAT16, FLOAT32, FLOAT64,
    
        INT8, UINT8, INT16, UINT16,
    
        INT_PTR, UINT_PTR,
    
        FLOAT2, FLOAT3, FLOAT4,
    
        INT2, INT3, INT4,
    
        UINT2, UINT3, UINT4,
    
        MAT3, MAT4,
    
        STRUCT,
    
        UNKNOWN
    };
    // clang-format on

    struct ReflectedMember; // forward declaration

    // base struct for reflection
    struct ReflectedDataNode {
        ValueType type{ ValueType::UNKNOWN };

        uint32_t array_size{ 1 };

        uint32_t size{};

        std::vector<ReflectedMember> members{};

        // TODO : provide attribute name
        fe::hashed_string name{};

        ReflectedDataNode() = default;
        ReflectedDataNode(ValueType type, uint32_t array_size, uint32_t size, std::vector<ReflectedMember> members, std::string name)
            : type(type), array_size(array_size), size(size), members(std::move(members)), name(std::move(name)) {}

        bool operator==(const ReflectedDataNode&) const noexcept = default;
    };

    // may be a field of a shader struct
    struct ReflectedMember : public ReflectedDataNode {
        uint32_t offset{};

        bool operator==(const ReflectedMember&) const noexcept = default;
    };

    struct ReflectedDescriptor : public ReflectedDataNode {
        DescriptorType descriptor_type{ DescriptorType::UNKNOWN };

        uint32_t set{};
        uint32_t binding{};

        uint8_t stage_flags{};

        bool is_bindless{};

        ReflectedDescriptor() = default;
        ReflectedDescriptor(ReflectedDataNode data_node, DescriptorType descriptor_type, uint32_t set, uint32_t binding, uint8_t stage_flags, bool is_bindless)
            : ReflectedDataNode(std::move(data_node)), descriptor_type(descriptor_type), set(set), binding(binding), stage_flags(stage_flags), is_bindless(is_bindless) {}

        bool operator==(const ReflectedDescriptor&) const noexcept = default;
    };

    struct ReflectedPushConstants : public ReflectedDataNode {
        uint8_t stage_flags{};

        ReflectedPushConstants() = default;
        ReflectedPushConstants(ReflectedDataNode data_node, uint8_t stage_flags)
            : ReflectedDataNode(std::move(data_node)), stage_flags(stage_flags) {}

        bool operator==(const ReflectedPushConstants&) const noexcept = default;
    };

    struct ReflectedStructureLayout {
        uint32_t                             size{};
        std::vector<shader::ReflectedMember> members{};
        fe::hashed_string                    name{};

        ReflectedStructureLayout() = default;
        ReflectedStructureLayout(uint32_t size, std::vector<ReflectedMember> members, fe::hashed_string name)
            : size(size), members(std::move(members)), name(std::move(name)) {}

        bool operator==(const ReflectedStructureLayout&) const noexcept = default;
    };

    enum class StageBits : std::uint8_t {
        NONE     = 0,
        VERTEX   = 1 << 0,
        GEOMETRY = 1 << 1,
        FRAGMENT = 1 << 2,
        COMPUTE  = 1 << 3,
    };

    struct ReflectedEntryPoint {
        StageBits stage_flag{};

        // sometimes a generic can be specialized by a lot of types, not only one
        // Example :
        // '<M : IMaterial, ISomeOtherType>' --> '{ "IMaterial", "ISomeOtherType" }'
        // there 'M' can be 'IMaterial' or 'ISomeOtherType'
        using Constraints = std::vector<fe::hashed_string>;
        
        // there are generic arguments : <M : IMaterial, O : IPostProcess, bool UseShadows>
        // every generic argument has it's constraints, which declare what types you can
        // use to specialize this argument. Look at 'fe::ReflectedEntryPoint::Constraints' comment for examples
        std::vector<Constraints> generic_arguments{};

        /* I'm not sure that I really need it */
        //std::vector<fe::hashed_string> arguments{};
        //fe::hashed_string return_value{};

        fe::hashed_string name{};

        ReflectedEntryPoint() = default;
        ReflectedEntryPoint(StageBits stage_flag /*, std::vector<fe::hashed_string> arguments*/, std::vector<Constraints> generic_arguments, fe::hashed_string name)
            : stage_flag(stage_flag), /*arguments(std::move(arguments)),*/ generic_arguments(std::move(generic_arguments)), name(std::move(name)) {}

        bool operator==(const ReflectedEntryPoint&) const noexcept = default;
    };

    using SpecializationValue = std::variant<fe::hashed_string,
                                             bool,
                                             int32_t,
                                             uint32_t,
                                             int64_t,
                                             uint64_t,
                                             float,
                                             double>;

    struct SpecializationArgument {
        fe::hashed_string   name{};
        SpecializationValue value;

        bool operator==(const SpecializationArgument&) const noexcept = default;
    };

    struct EntryPointSpecialization {
        fe::hashed_string                   name{};
        std::vector<SpecializationArgument> arguments{};

        bool operator==(const EntryPointSpecialization&) const noexcept = default;
    };

    struct ProgramSpecialization {
        std::vector<SpecializationArgument>   global_arguments{};
        std::vector<EntryPointSpecialization> entry_points{};

        bool operator==(const ProgramSpecialization&) const noexcept = default;
    };

    using SourceCode     = std::vector<uint8_t>;
    using ProgramSources = std::unordered_map<StageBits, SourceCode>;
} // namespace fe::shader
