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

fe::pointer<fe::resource::ShaderProgram> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    ShaderProgram shader_program{};

    ShaderImportContext context{ storage, resource_full_path, shader_program };

    // compile
    bool compilation_result = compile(context);
    if (!compilation_result) {
        fe::logging::error("Slang -> Unified. Slang : Failed to compile a shader program\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    // reflect
    bool reflection_result = reflect(context);
    if (!reflection_result) {
        fe::logging::error("Slang -> Unified. Slang : Failed to reflect a shader program\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    // TODO : add reflection check

    auto ptr = storage.CreateResource(std::move(shader_program));
    return ptr;
}

bool fe::ShaderImporter::compile(ShaderImportContext& context) {
    static Slang::ComPtr<slang::IGlobalSession> global_session{};
    if (!global_session) {
        if (SLANG_FAILED(slang::createGlobalSession(global_session.writeRef()))) {
            fe::logging::error("Slang -> Unified. Slang : Failed to create global session");
            return false;
        }
    }

    slang::SessionDesc session_desc{};
    slang::TargetDesc  target_desc{};

    const auto& resource_management_context = context.storage.GetContext();

    switch (resource_management_context.graphics_backend) {
        case GraphicsBackend::OpenGL:
            target_desc.format  = SLANG_GLSL;
            target_desc.profile = global_session->findProfile("glsl_450");
            break;
        case GraphicsBackend::Vulkan:
            target_desc.format  = SLANG_SPIRV;
            target_desc.profile = global_session->findProfile("spirv_1_5");
            break;
        default:
            fe::logging::warning("The selected renderer backend %i was not found. Using the default one",
                                 resource_management_context.graphics_backend);

            target_desc.format  = SLANG_GLSL;
            target_desc.profile = global_session->findProfile("glsl_450");
            break;
    }

    target_desc.flags = 0;

    session_desc.targets                  = &target_desc;
    session_desc.targetCount              = 1;
    session_desc.compilerOptionEntryCount = 0;

    if (SLANG_FAILED(global_session->createSession(session_desc, context.session.writeRef()))) {
        fe::logging::error("Slang -> Unified. Slang : Failed to create a session");
        return false;
    }

    slang::IModule* slang_module = nullptr;

    slang_module = context.session->loadModule(context.resource_full_path.string().c_str());
    if (!slang_module) {
        fe::logging::error("Slang -> Unified. Slang : Failed to load a slang module");
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
        fe::logging::error("Slang -> Unified. Slang : Failed to create a composed program");
        return false;
    }

    for (std::size_t i = 0; i < entry_points.size(); i++) {
        const auto&                 entry_point = entry_points[i];
        Slang::ComPtr<slang::IBlob> spirv_code{};

        SlangResult result = context.composed_program->getEntryPointCode(i, 0, spirv_code.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("Slang -> Unified. Slang : Failed to get an entry point code\nEntry point index : %i", i);
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

bool fe::ShaderImporter::reflect(ShaderImportContext& context) {
    slang::ProgramLayout* program_layout = context.composed_program->getLayout(0); // 'target = 0'
    if (!program_layout) {
        fe::logging::error("Slang -> Unified. Slang : Failed to get the layout of the composed program while reflection");
        return false;
    }

    unsigned int parameter_count = program_layout->getParameterCount();

    context.shader_program.reflected_resources.reserve(parameter_count);

    for (unsigned int i = 0; i < parameter_count; i++) {
        ShaderProgram::ReflectedResource reflected_resource{};

        slang::VariableLayoutReflection* parameter = program_layout->getParameterByIndex(i);
        if (!parameter) {
            fe::logging::warning("Slang -> Unified. Slang : slang::VariableLayoutReflection* parameter = program_layout->getParameterByIndex(i) was nullptr. i = %i", i);
            continue;
        }

        slang::TypeLayoutReflection* type_layout = parameter->getTypeLayout();

        if (type_layout->getKind() == slang::TypeReflection::Kind::ParameterBlock ||
            type_layout->getKind() == slang::TypeReflection::Kind::ConstantBuffer ||
            type_layout->getKind() == slang::TypeReflection::Kind::Array) {

            type_layout = type_layout->getElementTypeLayout();
        }

        reflected_resource.set            = parameter->getBindingSpace();
        reflected_resource.size           = type_layout->getSize();
        reflected_resource.name           = parameter->getName();
        reflected_resource.binding        = parameter->getBindingIndex();
        reflected_resource.resource_class = to_resource_class(type_layout);

        unsigned int field_count = type_layout->getFieldCount();

        reflected_resource.members.reserve(field_count);

        for (unsigned int j = 0; j < field_count; j++) {
            ShaderProgram::ReflectedMember reflected_member{};

            slang::VariableLayoutReflection* variable_layout = type_layout->getFieldByIndex(j);

            slang::TypeReflection* type = variable_layout->getType();

            reflected_member.type   = to_value_type(type);
            reflected_member.offset = variable_layout->getOffset();
            reflected_member.size   = variable_layout->getTypeLayout()->getSize();
            reflected_member.name   = variable_layout->getName();

            reflected_resource.members.emplace_back(reflected_member);
        }

        context.shader_program.reflected_resources.emplace_back(reflected_resource);
    }

    return true;
}

ShaderProgram::ResourceClass fe::ShaderImporter::to_resource_class(slang::TypeLayoutReflection* type_layout) {
    using _shader    = ShaderProgram::ResourceClass;
    using _slang_kind = slang::TypeReflection::Kind;

    slang::TypeReflection* type = type_layout->getType();
    _slang_kind             kind = type->getKind();

    switch (kind) {
        case _slang_kind::ConstantBuffer:
            return _shader::UNIFORM_BUFFER;

        case _slang_kind::ShaderStorageBuffer:
            return _shader::STORAGE_BUFFER;

        case _slang_kind::SamplerState:
            return _shader::SAMPLER;

        case _slang_kind::Resource: {
            SlangResourceShape  shape  = type_layout->getResourceShape();
            SlangResourceAccess access = type_layout->getResourceAccess();

            if ((shape & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_STRUCTURED_BUFFER) {
                return _shader::STORAGE_BUFFER;
            }
            if ((shape & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_ACCELERATION_STRUCTURE) {
                return _shader::ACCELERATION_STRUCTURE;
            }

            if ((shape & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_TEXTURE_2D) {
                if (access == SLANG_RESOURCE_ACCESS_READ_WRITE) {
                    return _shader::STORAGE_TEXTURE;
                }
                return _shader::SAMPLED_TEXTURE;
            }

            return _shader::SAMPLED_TEXTURE;
        }

        case _slang_kind::ParameterBlock:
            assert(false && "ParameterBlock must be unwrapped using getElementTypeLayout()");
            return _shader::UNIFORM_BUFFER;

        default:
            assert(false && "Unknown resource class");
            return _shader::UNIFORM_BUFFER;
    }
}

ShaderProgram::ValueType fe::ShaderImporter::to_value_type(slang::TypeReflection* type) {
    using _shader      = ShaderProgram::ValueType;
    using _slang_kind   = slang::TypeReflection::Kind;
    using _slang_scalar = slang::TypeReflection::ScalarType;

    _slang_kind   kind   = type->getKind();
    _slang_scalar scalar = type->getScalarType();

    if (kind == _slang_kind::Scalar) {
        if (scalar == _slang_scalar::Float32) return _shader::FLOAT;
        if (scalar == _slang_scalar::Int32) return _shader::INT;
        if (scalar == _slang_scalar::UInt32) return _shader::UINT;
    }

    if (kind == _slang_kind::Vector) {
        uint32_t component_count = type->getElementCount();
        if (scalar == _slang_scalar::Float32) {
            if (component_count == 2) return _shader::FLOAT2;
            if (component_count == 3) return _shader::FLOAT3;
            if (component_count == 4) return _shader::FLOAT4;
        }
        if (scalar == _slang_scalar::Int32) {
            if (component_count == 2) return _shader::INT2;
            if (component_count == 3) return _shader::INT3;
            if (component_count == 4) return _shader::INT4;
        }
        if (scalar == _slang_scalar::UInt32) {
            if (component_count == 2) return _shader::UINT2;
            if (component_count == 3) return _shader::UINT3;
            if (component_count == 4) return _shader::UINT4;
        }
    }

    if (kind == _slang_kind::Matrix) {
        uint32_t rows = type->getRowCount();
        uint32_t cols = type->getColumnCount();
        if (rows == 4 && cols == 4) return _shader::MAT4;
        if (rows == 3 && cols == 3) return _shader::MAT3;
    }

    if (kind == _slang_kind::Struct) {
        return _shader::STRUCT;
    }

    return _shader::STRUCT;
}
