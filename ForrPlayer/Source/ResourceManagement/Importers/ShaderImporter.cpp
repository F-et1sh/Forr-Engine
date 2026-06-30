/*===============================================

    Forr Engine

    File : ShaderImporter.cpp
    Role : imports resources and their metadata. for Slang

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderImporter.hpp"

#include <fstream>

using namespace fe::resource;

namespace fe {
    using _shader_descriptor = ShaderProgram::DescriptorType;
    using _shader_value      = ShaderProgram::ValueType;

    using _slang_kind     = slang::TypeReflection::Kind;
    using _slang_scalar   = slang::TypeReflection::ScalarType;
    using _slang_category = slang::ParameterCategory;
} // namespace fe

fe::pointer<fe::resource::ShaderProgram> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    ShaderProgram shader_program{};

    ShaderImportContext context{ storage, resource_full_path, shader_program };

    // compile
    bool compilation_result = ShaderImporter::compile(context);
    if (!compilation_result) {
        fe::logging::error("Slang -> Unified. Failed to compile a shader program\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    //// reflect
    //bool reflection_result = ShaderImporter::reflect(context);
    //if (!reflection_result) {
    //    fe::logging::error("Slang -> Unified. Failed to reflect a shader program\nPath : %s", resource_full_path.string().c_str());
    //    return {};
    //}

    // Validation is temporarily turned off till I completely fix the shaders

    // validate
    //bool validation_result = ShaderImporter::validate(context);
    //if (!validation_result) {
    //    fe::logging::error("Slang -> Unified. Validation failed\nPath : %s", resource_full_path.string().c_str());
    //    return {};
    //}

    auto ptr = storage.CreateResource(std::move(shader_program));
    return ptr;
}

bool fe::ShaderImporter::compile(ShaderImportContext& context) {
    static Slang::ComPtr<slang::IGlobalSession> global_session{};
    if (!global_session) {
        if (SLANG_FAILED(slang::createGlobalSession(global_session.writeRef()))) {
            fe::logging::error("Slang -> Unified. Failed to create global session");
            return false;
        }
    }

    slang::SessionDesc session_desc{};
    slang::TargetDesc  target_desc{};

    target_desc.format  = SLANG_SPIRV;
    target_desc.profile = global_session->findProfile("spirv_1_5");
    target_desc.flags   = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

    target_desc.flags |= SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM;

    session_desc.targets                  = &target_desc;
    session_desc.targetCount              = 1;
    session_desc.compilerOptionEntryCount = 0;

    if (SLANG_FAILED(global_session->createSession(session_desc, context.session.writeRef()))) {
        fe::logging::error("Slang -> Unified. Failed to create a session");
        return false;
    }

    slang::IModule* slang_module = nullptr;

    slang_module = context.session->loadModule(context.resource_full_path.string().c_str());
    if (!slang_module) {
        fe::logging::error("Slang -> Unified. Failed to load a slang module");
        return false;
    }

    std::vector<slang::IComponentType*> component_types{};
    component_types.emplace_back(slang_module);

    std::vector<EntryPoint> entry_points{};

    for (std::uint32_t i = 0; i < std::size(ShaderImportContext::entry_point_names); i++) {
        auto                entry_point_name = ShaderImportContext::entry_point_names[i];
        slang::IEntryPoint* entry_point{};
        slang_module->findEntryPointByName(entry_point_name.data(), &entry_point);

        if (entry_point) {
            ShaderProgram::ShaderType shader_type{};

            if (entry_point_name == ShaderImportContext::entry_point_names[0])
                shader_type = ShaderProgram::ShaderType::VERTEX;
            else if (entry_point_name == ShaderImportContext::entry_point_names[1])
                shader_type = ShaderProgram::ShaderType::FRAGMENT;
            else if (entry_point_name == ShaderImportContext::entry_point_names[2])
                shader_type = ShaderProgram::ShaderType::COMPUTE;

            entry_points.emplace_back(entry_point, shader_type);
            component_types.emplace_back(entry_point);
        }
    }

    SlangResult result = context.session->createCompositeComponentType(component_types.data(), component_types.size(), context.composed_program.writeRef());
    if (SLANG_FAILED(result)) {
        fe::logging::error("Slang -> Unified. Failed to create a composed program");
        return false;
    }

    for (std::size_t i = 0; i < entry_points.size(); i++) {
        const auto&                 entry_point = entry_points[i];
        Slang::ComPtr<slang::IBlob> spirv_code{};

        SlangResult result = context.composed_program->getEntryPointCode(i, 0, spirv_code.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Slang -> Unified. Failed to get an entry point code\nEntry point index : %i", i);
            return false;
        }

        const size_t   byte_size = spirv_code->getBufferSize();
        const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(spirv_code->getBufferPointer());

        ShaderProgram::ShaderType shader_type     = entry_point.shader_type;
        auto&                     source_code_dst = context.shader_program.source_codes[shader_type];

        source_code_dst.resize(byte_size, 0);
        std::memcpy(source_code_dst.data(), raw_data, byte_size);
    }

    return true;
}

//bool fe::ShaderImporter::reflect(ShaderImportContext& context) {
//    auto& shader_program = context.shader_program; // from context
//
//    slang::ProgramLayout* program_layout = context.composed_program->getLayout(0); // 'target = 0'
//    if (!program_layout) {
//        fe::logging::error("Slang -> Unified. Failed to get the layout of the composed program while reflection in fe::ShaderImporter::reflect()");
//        return false;
//    }
//
//    unsigned int parameter_count = program_layout->getParameterCount();
//
//    shader_program.reflected_resources.reserve(parameter_count);
//
//    for (unsigned int i = 0; i < parameter_count; i++) {
//        slang::VariableLayoutReflection* parameter = program_layout->getParameterByIndex(i);
//        if (!parameter) {
//            fe::logging::warning("Slang -> Unified. slang::VariableLayoutReflection* parameter = program_layout->getParameterByIndex(i) was nullptr. i = %i", i);
//            continue;
//        }
//
//        auto& resource = shader_program.reflected_resources.emplace_back();
//        ShaderImporter::parseVariable(context, parameter, resource);
//    }
//
//    return true;
//}
//
//bool fe::ShaderImporter::validate(ShaderImportContext& context) {
//    // TODO : add define for shader to mute the warning message
//    // TODO : make validation more strict
//
//    auto& graphics_backend    = context.storage.GetContext().graphics_backend;
//    auto& reflected_resources = context.shader_program.reflected_resources;
//
//    struct ExpectedResource {
//        ShaderProgram::DescriptorType descriptor_type{};
//
//        uint32_t set{};
//        uint32_t binding{};
//
//        uint32_t members_count{};
//
//        std::string_view name{};
//    };
//
//    constexpr static std::array expected_resources_vulkan{
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 0, 1, "global_data" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 1, 1, "model_matrices" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 2, 1, "lights" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 1, 0, 1, "materials" },
//        ExpectedResource{ ShaderProgram::DescriptorType::PUSH_CONSTANT, 0, 0, 1, "push_constants" }
//    };
//
//    constexpr static std::array expected_resources_opengl{
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 0, 1, "global_data" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 1, 1, "model_matrices" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 2, 1, "lights" },
//        ExpectedResource{ ShaderProgram::DescriptorType::STORAGE_BUFFER, 0, 3, 1, "materials" },
//        ExpectedResource{ ShaderProgram::DescriptorType::PUSH_CONSTANT, 0, 0, 1, "push_constants" }
//    };
//
//    const auto& expected_resources = (graphics_backend == GraphicsBackend::OpenGL) ? expected_resources_opengl : expected_resources_vulkan;
//
//    for (const auto& expected_resource : expected_resources) {
//        auto it = reflected_resources.end();
//
//        if (expected_resource.descriptor_type == ShaderProgram::DescriptorType::PUSH_CONSTANT) {
//            it = std::ranges::find(reflected_resources, ShaderProgram::DescriptorType::PUSH_CONSTANT, &ShaderProgram::ReflectedResource::descriptor_type);
//        }
//        else {
//            it = std::ranges::find_if(reflected_resources, [&](const auto& resource) -> bool {
//                return resource.set == expected_resource.set &&
//                       resource.binding == expected_resource.binding;
//            });
//        }
//
//        if (it == reflected_resources.end()) {
//            fe::logging::warning("Slang -> Unified. Shader has no descriptor set %i ( \"%s\" ).\nIf you don't need it in the shader you don't have to add it",
//                                 expected_resource.set,
//                                 expected_resource.name.data());
//            continue;
//        }
//
//        if (it->descriptor_type != expected_resource.descriptor_type) {
//            fe::logging::error("Slang -> Unified. Shader descriptor set %i has descriptor type %i, but must be %i",
//                               expected_resource.set,
//                               it->descriptor_type,
//                               expected_resource.descriptor_type);
//            return false;
//        }
//
//        if (it->binding != expected_resource.binding) {
//            fe::logging::error("Slang -> Unified. Shader descriptor set %i has binding %i, but must be %i",
//                               expected_resource.set,
//                               it->binding,
//                               expected_resource.binding);
//            return false;
//        }
//
//        if (it->members.size() != expected_resource.members_count) {
//            fe::logging::error("Slang -> Unified. Shader descriptor set %i has members count %i, but must be %i",
//                               expected_resource.set,
//                               it->members.size(),
//                               expected_resource.members_count);
//            return false;
//        }
//
//        if (it->name != expected_resource.name) {
//            fe::logging::error("Slang -> Unified. Shader descriptor set %i has name %s, but must be \"%s\"",
//                               expected_resource.set,
//                               it->name.c_str(),
//                               expected_resource.name.data());
//            return false;
//        }
//    }
//
//    return true;
//}
//
//void fe::ShaderImporter::parseVariable(ShaderImportContext&                        context,
//                                       slang::VariableLayoutReflection*            variable_layout,
//                                       resource::ShaderProgram::ReflectedResource& dst_resource) {
//    assert(variable_layout);
//
//    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
//    _slang_kind                  kind        = type_layout->getKind();
//
//    dst_resource.size    = static_cast<uint32_t>(type_layout->getSize());
//    dst_resource.name    = variable_layout->getName();
//    dst_resource.set     = variable_layout->getBindingSpace();
//    dst_resource.binding = variable_layout->getBindingIndex();
//
//    // 24.06.2026 Slang has got a bug, so, now this will work like this
//    //
//    // TODO : update Slang submodule and remove this part
//    {
//        static uint32_t vulkan_set_counter = 0;
//
//        if (dst_resource.name == "scene_set") {
//            vulkan_set_counter = 0;
//        }
//
//        if (kind == _slang_kind::ParameterBlock) {
//            dst_resource.set     = vulkan_set_counter++;
//            dst_resource.binding = 0;
//        }
//        else if (kind == _slang_kind::ConstantBuffer || kind == _slang_kind::ShaderStorageBuffer) {
//            dst_resource.set     = vulkan_set_counter;
//            dst_resource.binding = variable_layout->getBindingIndex();
//        }
//        else {
//            dst_resource.set     = variable_layout->getBindingSpace();
//            dst_resource.binding = variable_layout->getBindingIndex();
//        }
//
//        auto& graphics_backend = context.storage.GetContext().graphics_backend;
//
//        if (graphics_backend == GraphicsBackend::OpenGL) {
//            dst_resource.set     = 0;
//            dst_resource.binding = variable_layout->getBindingIndex();
//        }
//    }
//
//    ShaderImporter::setupDescriptorType(type_layout, dst_resource.descriptor_type);
//
//    // kinds like '_slang_kind::ConstantBuffer', '_slang_kind::ShaderStorageBuffer', '_slang_kind::ParameterBlock' are needs to be unwrapped like this
//    slang::TypeLayoutReflection* element_type_layout  = type_layout->getElementTypeLayout();
//    slang::TypeLayoutReflection* type_layout_to_parse = element_type_layout ? element_type_layout : type_layout;
//
//    uint32_t field_count = type_layout_to_parse->getFieldCount();
//    for (uint32_t i = 0; i < field_count; i++) {
//        parseMemberRecursive(type_layout_to_parse->getFieldByIndex(i), dst_resource.members);
//    }
//}
//
//void fe::ShaderImporter::parseMemberRecursive(slang::VariableLayoutReflection*                       variable_layout,
//                                              std::vector<resource::ShaderProgram::ReflectedMember>& dst_members) {
//    assert(variable_layout);
//
//    slang::TypeLayoutReflection* type_layout = variable_layout->getTypeLayout();
//
//    auto& member = dst_members.emplace_back();
//
//    member.name   = variable_layout->getName();
//    member.size   = static_cast<uint32_t>(type_layout->getSize());
//    member.offset = static_cast<uint32_t>(variable_layout->getOffset());
//
//    _slang_kind kind = type_layout->getKind();
//
//    auto parse_recursive = [](slang::TypeLayoutReflection*                           type_layout,
//                              std::vector<resource::ShaderProgram::ReflectedMember>& dst_members) {
//        uint32_t field_count = type_layout->getFieldCount();
//        for (uint32_t i = 0; i < field_count; i++) {
//            parseMemberRecursive(type_layout->getFieldByIndex(i), dst_members);
//        }
//    };
//
//    switch (kind) {
//        case _slang_kind::Struct:
//            member.type = _shader_value::STRUCT;
//            parse_recursive(type_layout, member.members);
//            break;
//
//        case _slang_kind::Array: {
//            //member.type = _shader_value::ARRAY;
//
//            slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
//            if (element_type_layout) parse_recursive(element_type_layout, member.members);
//        } break;
//
//            // clang-format off
//        case _slang_kind::Matrix: ShaderImporter::mapMatrix(type_layout, member.type); break;
//        case _slang_kind::Vector: ShaderImporter::mapVector(type_layout, member.type); break;
//        case _slang_kind::Scalar: ShaderImporter::mapScalar(type_layout, member.type); break;
//            // clang-format on
//
//        case _slang_kind::Resource: {
//            SlangResourceShape shape = type_layout->getResourceShape();
//
//            uint32_t base_shape = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;
//
//            if (base_shape == SLANG_STRUCTURED_BUFFER) {
//                //member.type = _shader_value::STRUCTURED_BUFFER;
//
//                slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
//                if (element_type_layout) parse_recursive(element_type_layout, member.members);
//            }
//            else if (base_shape == SLANG_TEXTURE_2D) {
//                member.type = _shader_value::TEXTURE_2D;
//            }
//        } break;
//
//        case _slang_kind::ConstantBuffer:
//        case _slang_kind::ShaderStorageBuffer:
//        case _slang_kind::ParameterBlock: {
//            member.type = _shader_value::STRUCT;
//
//            slang::TypeLayoutReflection* element_type_layout = type_layout->getElementTypeLayout();
//            if (element_type_layout) parse_recursive(element_type_layout, member.members);
//        } break;
//
//            // clang-format off
//        case _slang_kind::SamplerState  : member.type = _shader_value::UINT64    ; break;
//        case _slang_kind::TextureBuffer : member.type = _shader_value::TEXTURE_2D; break;
//        case _slang_kind::Pointer       : member.type = _shader_value::INT_PTR   ; break;
//            // clang-format on
//
//        case _slang_kind::Enum:
//            ShaderImporter::mapScalar(type_layout, member.type);
//            if (member.type == _shader_value::UNKNOWN) member.type = _shader_value::INT32;
//            break;
//
//        case _slang_kind::GenericTypeParameter:
//        case _slang_kind::Interface:
//        case _slang_kind::OutputStream:
//        case _slang_kind::Specialized:
//        case _slang_kind::Feedback:
//        case _slang_kind::DynamicResource:
//        case _slang_kind::MeshOutput:
//            fe::logging::error("Slang -> Unified. Failed to reflect resource : got invalid slang::TypeReflection::Kind in fe::ShaderImporter::parseMemberRecursive()");
//            member.type = ShaderProgram::ValueType::UNKNOWN;
//            break;
//
//        default:
//            fe::logging::error("Slang -> Unified. Failed to reflect resource : got unknown slang::TypeReflection::Kind");
//            break;
//    }
//}
//
//void fe::ShaderImporter::setupDescriptorType(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::DescriptorType& dst_descriptor_type) {
//    _slang_kind     kind     = type_layout->getKind();
//    _slang_category category = type_layout->getParameterCategory();
//
//    // clang-format off
//    switch (category) {
//        case _slang_category::Uniform:
//        case _slang_category::ConstantBuffer:
//            dst_descriptor_type = _shader_descriptor::UNIFORM_BUFFER;
//            break;
//    
//        case _slang_category::UnorderedAccess:
//            if (kind == _slang_kind::Resource) dst_descriptor_type = _shader_descriptor::STORAGE_TEXTURE;
//            else                               dst_descriptor_type = _shader_descriptor::STORAGE_BUFFER;
//            break;
//    
//        case _slang_category::ShaderResource:
//            if (kind == _slang_kind::Resource) dst_descriptor_type = _shader_descriptor::SAMPLED_TEXTURE;
//            else                               dst_descriptor_type = _shader_descriptor::STORAGE_BUFFER;
//            break;
//    
//        case _slang_category::PushConstantBuffer:
//            dst_descriptor_type = _shader_descriptor::PUSH_CONSTANT;
//            break;
//
//        case _slang_category::SubElementRegisterSpace:
//            if (kind == _slang_kind::Resource) {
//                SlangResourceShape shape = type_layout->getResourceShape();
//                uint32_t base_shape = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;
//                
//                if (base_shape == SLANG_STRUCTURED_BUFFER) dst_descriptor_type = _shader_descriptor::STORAGE_BUFFER;
//                else                                       dst_descriptor_type = _shader_descriptor::SAMPLED_TEXTURE;
//            }
//            else {
//                dst_descriptor_type = _shader_descriptor::STORAGE_BUFFER;
//            }
//            break;
//
//        case _slang_category::DescriptorTableSlot:
//            if (kind == _slang_kind::Resource) {
//                SlangResourceShape shape = type_layout->getResourceShape();
//                uint32_t base_shape = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;
//                
//                if (base_shape == SLANG_STRUCTURED_BUFFER) dst_descriptor_type = _shader_descriptor::STORAGE_BUFFER;
//                else
//                    ShaderImporter::setupDescriptorType(type_layout, dst_descriptor_type); // maybe fallback on '_shader_descriptor::UNIFORM_BUFFER'
//            }
//            else if (kind == _slang_kind::ConstantBuffer) {
//                dst_descriptor_type = _shader_descriptor::UNIFORM_BUFFER;
//            }
//            else {
//                dst_descriptor_type = _shader_descriptor::STORAGE_BUFFER;
//            }
//            break;
//        
//        case _slang_category::None:
//            fe::logging::error("Slang -> Unified. Failed to reflect resource : got slang::ParameterCategory::None.\nSetting descriptor_type as fe::resource::ShaderProgram::DescriptorType::UNKNOWN");
//            dst_descriptor_type = _shader_descriptor::UNKNOWN;
//            break;
//    
//        default:
//            fe::logging::error("Slang -> Unified. Failed to reflect resource : got unknown slang::ParameterCategory %i.\nSetting descriptor_type as fe::resource::ShaderProgram::DescriptorType::UNKNOWN", category);
//            dst_descriptor_type = _shader_descriptor::UNKNOWN;
//            break;
//    }
//    // clang-format on
//}
//
//void fe::ShaderImporter::mapMatrix(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ValueType& type) {
//    assert(type_layout);
//
//    uint32_t rows = type_layout->getRowCount();
//    uint32_t cols = type_layout->getColumnCount();
//
//    if (rows == 4 && cols == 4)
//        type = _shader_value::MAT4;
//    else if (rows == 3 && cols == 3)
//        type = _shader_value::MAT3;
//    else
//        assert(false);
//}
//
//void fe::ShaderImporter::mapVector(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ValueType& type) {
//    assert(type_layout);
//
//    _slang_scalar scalar          = type_layout->getScalarType();
//    uint32_t      component_count = type_layout->getElementCount();
//
//    switch (scalar) {
//        case _slang_scalar::Float32:
//            if (component_count == 2) type = _shader_value::FLOAT2;
//            if (component_count == 3) type = _shader_value::FLOAT3;
//            if (component_count == 4) type = _shader_value::FLOAT4;
//            break;
//        case _slang_scalar::Int32:
//            if (component_count == 2) type = _shader_value::INT2;
//            if (component_count == 3) type = _shader_value::INT3;
//            if (component_count == 4) type = _shader_value::INT4;
//            break;
//        case _slang_scalar::UInt32:
//            if (component_count == 2) type = _shader_value::UINT2;
//            if (component_count == 3) type = _shader_value::UINT3;
//            if (component_count == 4) type = _shader_value::UINT4;
//            break;
//        default:
//            assert(false);
//            break;
//    }
//}
//
//void fe::ShaderImporter::mapScalar(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ValueType& type) {
//    assert(type_layout);
//
//    _slang_scalar scalar = type_layout->getScalarType();
//
//    // clang-format off
//    switch (scalar) {
//        case _slang_scalar::Void     : type = _shader_value::VOID     ; break;
//        case _slang_scalar::Bool     : type = _shader_value::BOOL     ; break;
//        case _slang_scalar::Int32    : type = _shader_value::INT32    ; break;
//        case _slang_scalar::UInt32   : type = _shader_value::UINT32   ; break;
//        case _slang_scalar::Int64    : type = _shader_value::INT64    ; break;
//        case _slang_scalar::UInt64   : type = _shader_value::UINT64   ; break;
//        case _slang_scalar::Float16  : type = _shader_value::FLOAT16  ; break;
//        case _slang_scalar::Float32  : type = _shader_value::FLOAT32  ; break;
//        case _slang_scalar::Float64  : type = _shader_value::FLOAT64  ; break;
//        case _slang_scalar::Int8     : type = _shader_value::INT8     ; break;
//        case _slang_scalar::UInt8    : type = _shader_value::UINT8    ; break;
//        case _slang_scalar::Int16    : type = _shader_value::INT16    ; break;
//        case _slang_scalar::UInt16   : type = _shader_value::UINT16   ; break;
//        case _slang_scalar::IntPtr   : type = _shader_value::INT_PTR  ; break;
//        case _slang_scalar::UIntPtr  : type = _shader_value::UINT_PTR ; break;
//        case _slang_scalar::BFloat16 : type = _shader_value::BFLOAT16 ; break;
//        case _slang_scalar::FloatE4M3: type = _shader_value::FLOATE4M3; break;
//        case _slang_scalar::FloatE5M2: type = _shader_value::FLOATE5M2; break;
//        case _slang_scalar::None:
//            fe::logging::error("Slang -> Unified. Failed to reflect resource : got slang::TypeReflection::ScalarType::None.\nSetting type as fe::resource::ShaderProgram::ValueType::UNKNOWN");
//            type = _shader_value::UNKNOWN;
//            break;
//        default:
//            fe::logging::error("Slang -> Unified. Failed to reflect resource : got unknown slang::TypeReflection::ScalarType.\nSetting type as fe::resource::ShaderProgram::ValueType::UNKNOWN");
//            type = _shader_value::UNKNOWN;
//            break;
//    }
//    // clang-format on
//}
