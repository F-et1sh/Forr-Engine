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

fe::pointer<fe::resource::ShaderProgram> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    ShaderProgram shader_program{};

    // compile
    slang::IComponentType*               composed_program_raw = compile(shader_program, storage, resource_full_path);
    Slang::ComPtr<slang::IComponentType> composed_program{ composed_program_raw };
    if (!composed_program) {
        fe::logging::error("File -> Unified. Slang : Failed to compile the shader program\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    // reflect
    bool result = reflect(shader_program, composed_program.get());
    if (!result) {
        fe::logging::error("File -> Unified. Slang : Failed to reflect the shader program\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    auto ptr = storage.CreateResource(std::move(shader_program));
    return ptr;
}

slang::IComponentType* fe::ShaderImporter::compile(fe::resource::ShaderProgram& shader_program,
                                                   ResourceStorage&             storage,
                                                   const std::filesystem::path& resource_full_path) {
    static Slang::ComPtr<slang::IGlobalSession> global_session{};
    if (!global_session) {
        if (SLANG_FAILED(slang::createGlobalSession(global_session.writeRef()))) {
            fe::logging::error("File -> Unified. Slang : Failed to create global session");
            return {};
        }
    }

    slang::SessionDesc session_desc{};
    slang::TargetDesc  target_desc{};

    const auto& resource_management_context = storage.GetContext();

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
            fe::logging::warning("The selected renderer backend %i was not found. Using the default one", resource_management_context.graphics_backend);

            target_desc.format  = SLANG_GLSL;
            target_desc.profile = global_session->findProfile("glsl_450");
            break;
    }

    target_desc.flags = 0;

    session_desc.targets                  = &target_desc;
    session_desc.targetCount              = 1;
    session_desc.compilerOptionEntryCount = 0;

    Slang::ComPtr<slang::ISession> session{};
    if (SLANG_FAILED(global_session->createSession(session_desc, session.writeRef()))) {
        fe::logging::error("File -> Unified. Slang : Failed to create a session");
        return {};
    }

    slang::IModule* slang_module = nullptr;

    slang_module = session->loadModule(resource_full_path.string().c_str());
    if (!slang_module) {
        fe::logging::error("File -> Unified. Slang : Failed to load a slang module");
        return {};
    }

    std::vector<slang::IComponentType*> component_types{};
    component_types.emplace_back(slang_module);

    static constexpr std::array<std::string_view, 3> entry_point_names{
        "vertexMain",
        "fragmentMain",
        "computeMain"
    };

    struct EntryPoint {
        Slang::ComPtr<slang::IEntryPoint> entry_point{};
        ShaderProgram::ShaderType         shader_type{};

        EntryPoint()  = default;
        ~EntryPoint() = default;

        EntryPoint(slang::IEntryPoint* entry_point, ShaderProgram::ShaderType shader_type)
            : entry_point(entry_point), shader_type(shader_type) {}
    };

    std::vector<EntryPoint> entry_points{};

    for (std::uint32_t i = 0; i < std::size(entry_point_names); i++) {
        auto                entry_point_name = entry_point_names[i];
        slang::IEntryPoint* entry_point{};
        slang_module->findEntryPointByName(entry_point_name.data(), &entry_point);

        if (entry_point) {
            ShaderProgram::ShaderType shader_type{};

            if (entry_point_name == entry_point_names[0])
                shader_type = ShaderProgram::ShaderType::VERTEX;
            else if (entry_point_name == entry_point_names[1])
                shader_type = ShaderProgram::ShaderType::FRAGMENT;
            else if (entry_point_name == entry_point_names[2])
                shader_type = ShaderProgram::ShaderType::COMPUTE;

            entry_points.emplace_back(entry_point, shader_type);
            component_types.emplace_back(entry_point);
        }
    }

    slang::IComponentType* composed_program{};

    SlangResult result = session->createCompositeComponentType(component_types.data(),
                                                               component_types.size(),
                                                               &composed_program);
    if (SLANG_FAILED(result)) {
        fe::logging::error("File -> Unified. Slang : Failed to create a composed program");
        return {};
    }

    for (std::size_t i = 0; i < entry_points.size(); i++) {
        const auto&                 entry_point = entry_points[i];
        Slang::ComPtr<slang::IBlob> spirv_code{};

        SlangResult result = composed_program->getEntryPointCode(i, 0, spirv_code.writeRef());
        if (SLANG_FAILED(result)) {
            fe::logging::error("File -> Unified. Slang : Failed to get an entry point code\nEntry point index : %i", i);
            return {};
        }

        const size_t   byte_size = spirv_code->getBufferSize();
        const uint8_t* raw_data  = reinterpret_cast<const uint8_t*>(spirv_code->getBufferPointer());

        ShaderProgram::ShaderType shader_type     = entry_point.shader_type;
        auto&                     source_code_dst = shader_program.source_codes[shader_type];

        source_code_dst.resize(byte_size, 0);
        std::memcpy(source_code_dst.data(), raw_data, byte_size);
    }

    return composed_program;
}

bool fe::ShaderImporter::reflect(fe::resource::ShaderProgram& shader_program, slang::IComponentType* composed_program) {
    slang::ProgramLayout* program_layout = composed_program->getLayout(0); // 'target = 0'
    if (!program_layout) {
        fe::logging::error("File -> Unified. Slang : Failed to get the layout of the composed program while reflection");
        return false;
    }

    unsigned int parameter_count = program_layout->getParameterCount();

    shader_program.reflected_resources.reserve(parameter_count);

    for (unsigned int i = 0; i < parameter_count; i++) {
        ShaderProgram::ReflectedResource reflected_resource{};

        slang::VariableLayoutReflection* parameter = program_layout->getParameterByIndex(i);
        if (!parameter) {
            fe::logging::warning("File -> Unified. Slang : slang::VariableLayoutReflection* parameter = program_layout->getParameterByIndex(i) was nullptr. i = %i", i);
            continue;
        }

        reflected_resource.binding = parameter->getBindingIndex();
        reflected_resource.set     = parameter->getBindingSpace();

        slang::TypeLayoutReflection* type_layout = parameter->getTypeLayout();

        slang::TypeReflection::Kind       kind        = type_layout->getKind();
        slang::TypeReflection::ScalarType scalar_type = type_layout->getScalarType();

        reflected_resource.resource_class = to_resource_class(kind);
        reflected_resource.name           = parameter->getName();
        reflected_resource.size           = type_layout->getSize();

        if (reflected_resource.resource_class == ShaderProgram::ResourceClass::UNIFORM_BUFFER ||
            reflected_resource.resource_class == ShaderProgram::ResourceClass::STORAGE_BUFFER) {

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

            shader_program.reflected_resources.emplace_back(reflected_resource);
        }
    }

    return true;
}

ShaderProgram::ResourceClass fe::ShaderImporter::to_resource_class(slang::TypeReflection::Kind kind) {
    using _shader = ShaderProgram::ResourceClass;
    using _slang  = slang::TypeReflection::Kind;

    switch (kind) {
        case _slang::ConstantBuffer:
            return _shader::UNIFORM_BUFFER;

        case _slang::ParameterBlock:
            return _shader::UNIFORM_BUFFER;

        case _slang::SamplerState:
            return _shader::SAMPLER;

        case _slang::TextureBuffer:
        case _slang::Resource:
            return _shader::SAMPLED_TEXTURE;

        case _slang::ShaderStorageBuffer:
            return _shader::STORAGE_BUFFER;

        default:
            assert(false);
            return _shader::UNIFORM_BUFFER;
    }
}

ShaderProgram::ValueType fe::ShaderImporter::to_value_type(slang::TypeReflection* type) {
    using _shader = ShaderProgram::ValueType;
    using _slang  = slang::TypeReflection::ScalarType;

    if (type->getScalarType() == _slang::Float32) {
        if (type->getRowCount() == 4 && type->getColumnCount() == 4)
            return _shader::MAT4;

        if (type->getColumnCount() == 4)
            return _shader::FLOAT4;

        return _shader::FLOAT;
    }

    // clang-format off
        switch (type->getScalarType()) {
            case _slang::Float32: return _shader::FLOAT;
            case _slang::Int32  : return _shader::INT;
            case _slang::UInt32 : return _shader::UINT;
            default             : return _shader::STRUCT;
        }
    // clang-format on
}
